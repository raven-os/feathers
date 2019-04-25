#include <unistd.h>
#include <iostream>

#include "Keyboard.hpp"
#include "Server.hpp"


std::map<std::string, uint32_t> modifiersLst = {
  {"Alt", WLR_MODIFIER_ALT},
  {"Ctrl", WLR_MODIFIER_CTRL}
};

Keyboard::Keyboard(Server *server, struct wlr_input_device *device) : server(server), device(device)
{
  shortcuts["a+b"] = {"Nothing", [](){std::cout << "NOTHING" << std::endl;}};
  shortcuts["Ctrl+Alt+t"] = {"Terminal", [](){
    if (fork() == 0)
    {
      execl("/bin/sh", "/bin/sh", "-c", "weston-terminal", nullptr);
    }
  }};

  shortcuts["a+F4"] = {"destroy", [server](){
    View *view;
    wl_list_for_each(view, &server->views, link) {
      if (view->xdg_surface->role == WLR_XDG_SURFACE_V6_ROLE_TOPLEVEL &&
          view->xdg_surface->toplevel->server_pending.activated)
        view->close();
    };
  }};

  shortcuts["Alt+Escape"] = {"Leave", [server](){ wl_display_terminate(server->display);}};
  shortcuts["Ctrl+Escape"] = {"Leave", [server](){ wl_display_terminate(server->display);}};
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
      std::cout << "TMP: " << tmp << std::endl;
      if (modifiersLst.find(tmp) != modifiersLst.end())
        mod |= modifiersLst[tmp];
      else
        sum += xkb_keysym_from_name(tmp.c_str(), XKB_KEYSYM_CASE_INSENSITIVE);
    }

    std::cout << "Shortcuts Code: " << sum << " + " << mod << " -> " << shortcut.second.name << std::endl;
    if ((keycodes_states.last_raw_modifiers == mod) && sum == keycodes_states.sum) {
      std::cout << keycodes_states.sum << " + " << keycodes_states.last_raw_modifiers << std::endl;
      shortcut.second.action();
      return true;
    }
  }
  std::cout << keycodes_states.sum << " + " << keycodes_states.last_raw_modifiers << std::endl << std::endl;
  return false;
}

void Keyboard::keyboard_handle_modifiers([[maybe_unused]]struct wl_listener *listener, [[maybe_unused]]void *data)
{
  struct wlr_seat *seat = server->seat->getSeat();
  wlr_seat_set_keyboard(seat, device);
  wlr_seat_keyboard_notify_modifiers(seat,
				     &device->keyboard->modifiers);
}

void Keyboard::keyboard_handle_key([[maybe_unused]]struct wl_listener *listener, void *data)
{
  struct wlr_event_keyboard_key *event = static_cast<struct wlr_event_keyboard_key *>(data);
  struct wlr_seat *seat = server->seat->getSeat();

  uint32_t keycode = event->keycode + 8;
  const xkb_keysym_t *syms;
  int nsyms = xkb_state_key_get_syms(device->keyboard->xkb_state, keycode, &syms);

  bool handled = false;
  uint32_t modifiers = wlr_keyboard_get_modifiers(device->keyboard);
  keycodes_states.update_state(event, static_cast<uint32_t>(keycode), modifiers, device->keyboard->xkb_state);
  char name[30] = {0};
  //std::cout << keycode << " " <<  "'A' sym:" <<xkb_keysym_from_name("a", XKB_KEYSYM_CASE_INSENSITIVE) << " " << xkb_state_key_get_utf32(device->keyboard->xkb_state, keycode) << std::endl;
  if (event->state == WLR_KEY_PRESSED)
  {
    for (int i = 0; i < nsyms; i++)
    {
      xkb_keysym_get_name(syms[i], name, 30);
      std::cout << name << ": " << syms[i] << std::endl;
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
    struct xkb_rule_names rules = { 0 };
    struct xkb_context *context;
    struct xkb_keymap *key_map;

    //TODO KEYBOARD config

    rules.layout = "fr";

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
    this->keymap = key_map;
    std::cout << xkb_keymap_get_as_string(this->keymap, XKB_KEYMAP_USE_ORIGINAL_FORMAT) << std::endl;
    wlr_keyboard_set_keymap(device->keyboard, this->keymap);

    //TODO implem repeat info

    xkb_keymap_unref(key_map);
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
