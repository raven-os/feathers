#include <unistd.h>
#include <iostream>
#include <algorithm>

#include "Keyboard.hpp"
#include "Server.hpp"


std::map<std::string, uint32_t> modifiersLst = {
  {"Alt", WLR_MODIFIER_ALT},
  {"Ctrl", WLR_MODIFIER_CTRL}
};

Keyboard::Keyboard(Server *server, struct wlr_input_device *device) : server(server), device(device), keymap(nullptr)
{

  // a + Something are some temporary shortcut for the sub compositor, otherwise our own compositor will override the Shortcuts
  // i.e: Alt+F4

  shortcuts["a+b"] = {"Nothing", [](){std::cout << "NOTHING" << std::endl;}};
  shortcuts["Ctrl+Alt+t"] = {"Terminal", [](){
    if (fork() == 0)
    {
      execl("/bin/sh", "/bin/sh", "-c", "weston-terminal", nullptr);
    }
  }};

  shortcuts["a+F2"] = {"Toggle fullscreen", [server](){
    if (server->views.size() >= 1)
      {
	std::unique_ptr<View> &view = server->views.front();
	auto const &output =
	  std::find_if(server->output.getOutputs().begin(), server->output.getOutputs().end(),
		       [&view](auto &out) {
			 return out->getWlrOutput() == view->getOutput();
		       })
	  ->get();

	if (!output->getFullscreen())
	  {
	    wlr_xdg_surface_v6_get_geometry(view->xdg_surface, &output->saved);
	    output->saved.x = view->x;
	    output->saved.y = view->y;
	    struct wlr_box *outputBox = wlr_output_layout_get_box(view->server->output.getLayout(), view->getOutput());
	    wlr_xdg_toplevel_v6_set_size(view->xdg_surface, outputBox->width, outputBox->height);
	    view->x = 0;
	    view->y = 0;
	    wlr_xdg_toplevel_v6_set_fullscreen(view->xdg_surface, true);
	  }
	else
	  {
	    wlr_xdg_toplevel_v6_set_fullscreen(view->xdg_surface, false);
	    wlr_xdg_toplevel_v6_set_size(view->xdg_surface, output->saved.width, output->saved.height);
	    view->x = output->saved.x;
	    view->y = output->saved.y;
	  }
	output->setFullscreen(!output->getFullscreen());
      }
  }};
  shortcuts["a+Tab"] = {"Switch window", [server](){
    if (server->views.size() >= 2)
      {
	std::unique_ptr<View> &view = server->views[1];
  	ServerView::focus_view(view.get(), view->xdg_surface->surface);
	// focus view put the newly focused view in front
	// so we put it back to its position and then rotate
	std::iter_swap(server->views.begin(), server->views.begin() + 1);
	std::rotate(server->views.begin(), server->views.begin() + 1, server->views.end());
      }
  }};

  shortcuts["Alt+Escape"] = {"Leave", [server](){ wl_display_terminate(server->getWlDisplay());}};
  shortcuts["Ctrl+D"] = {"Leave", [server](){ wl_display_terminate(server->getWlDisplay());}};
  shortcuts["Alt+D"] = {"Debug", [this](){debug = !debug;}};
}

Keyboard::~Keyboard() {
  if (keymap) {
    xkb_keymap_unref(keymap);
  }
  wl_list_remove(&key.link);
  wl_list_remove(&modifiers.link);
}

bool Keyboard::handle_keybinding()
{
  for (auto const & shortcut : shortcuts) {
    std::string key = shortcut.first;
    std::vector<std::string> splitStr;
    size_t i = 0;
    size_t j = key.find("+");
    uint32_t sum = 0;
    uint32_t mod = 0;

    while (j != std::string::npos) {
      key[j] = 0;
      splitStr.push_back(key.data() + i);
      i = j + 1;
      j = key.find("+", j + 1);
    }
    splitStr.push_back(key.data() + i);

    for (std::string tmp : splitStr) {
      if (modifiersLst.find(tmp) != modifiersLst.end())
        mod |= modifiersLst[tmp];
      else
        sum += xkb_keysym_from_name(tmp.c_str(), XKB_KEYSYM_CASE_INSENSITIVE);
    }

    if (debug)
      std::cout << "Shortcuts Code: " << sum << " + " << mod << " -> " << shortcut.second.name << std::endl;
    if ((keycodes_states.last_raw_modifiers == mod) && sum == keycodes_states.sum) {
      if (debug)
        std::cout << keycodes_states.sum << " + " << keycodes_states.last_raw_modifiers << std::endl;
      shortcut.second.action();
      return true;
    }
  }
  if (debug)
    std::cout << keycodes_states.sum << " + " << keycodes_states.last_raw_modifiers << std::endl << std::endl;
  return false;
}

void Keyboard::keyboard_handle_modifiers([[maybe_unused]]struct wl_listener *listener, [[maybe_unused]]void *data)
{
  struct wlr_seat *seat = server->seat.getSeat();
  wlr_seat_set_keyboard(seat, device);
  wlr_seat_keyboard_notify_modifiers(seat,
				     &device->keyboard->modifiers);
}

void Keyboard::keyboard_handle_key([[maybe_unused]]struct wl_listener *listener, void *data)
{
  struct wlr_event_keyboard_key *event = static_cast<struct wlr_event_keyboard_key *>(data);
  struct wlr_seat *seat = server->seat.getSeat();

  uint32_t keycode = event->keycode + 8;
  const xkb_keysym_t *syms;
  int nsyms = xkb_state_key_get_syms(device->keyboard->xkb_state, keycode, &syms);

  bool handled = false;
  uint32_t modifiers = wlr_keyboard_get_modifiers(device->keyboard);
  keycodes_states.update_state(event, keycode, modifiers, device->keyboard->xkb_state);
  char name[30] = {0};
  //std::cout << keycode << " " <<  "'A' sym:" <<xkb_keysym_from_name("a", XKB_KEYSYM_CASE_INSENSITIVE) << " " << xkb_state_key_get_utf32(device->keyboard->xkb_state, keycode) << std::endl;
  if (event->state == WLR_KEY_PRESSED)
  {
    for (int i = 0; i < nsyms; i++)
    {
      if (debug) {
        xkb_keysym_get_name(syms[i], name, 30);
        std::cout << name << ": " << syms[i] << std::endl;
      }
      handled = handle_keybinding();
    }
  }

  if (!handled)
  {
    wlr_seat_set_keyboard(seat, device);
    wlr_seat_keyboard_notify_key(seat, event->time_msec,
      event->keycode, event->state);
  }
}

void Keyboard::configure() {
    struct xkb_rule_names rules = { NULL, NULL, "fr", NULL, NULL };
    struct xkb_context *context;
    struct xkb_keymap *key_map;

    //TODO KEYBOARD config

    if (!(context = xkb_context_new(XKB_CONTEXT_NO_FLAGS))) {
      std::cerr << "Cannot create the xkb context" << std::endl;
      return ;
    }
    if (!(key_map = xkb_map_new_from_names(context, &rules, XKB_KEYMAP_COMPILE_NO_FLAGS))) {
      std::cerr << "Cannot configure keyboard: keymap not found" << std::endl;
      xkb_context_unref(context);
      return ;
    }

    //xkb_keymap_unref(keymap);
    xkb_keymap_unref(this->keymap);
    this->keymap = key_map;
    if (debug)
      std::cout << xkb_keymap_get_as_string(this->keymap, XKB_KEYMAP_USE_ORIGINAL_FORMAT) << std::endl;
    wlr_keyboard_set_keymap(device->keyboard, this->keymap);

    //TODO implem repeat info


    xkb_context_unref(context);
    wlr_keyboard_set_repeat_info(device->keyboard, 25, 600);
}

void Keyboard::setModifiersListener()
{
  SET_LISTENER(Keyboard, KeyboardListeners, modifiers, keyboard_handle_modifiers);
  wl_signal_add(&device->keyboard->events.modifiers, &modifiers);
}

void Keyboard::setKeyListener()
{
  SET_LISTENER(Keyboard, KeyboardListeners, key, keyboard_handle_key);
  wl_signal_add(&device->keyboard->events.key, &key);
}
