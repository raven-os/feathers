#include "Keyboard.hpp"
#include "Server.hpp"

#include <iostream>

Keyboard::Keyboard(Server *server, struct wlr_input_device *device) : server(server), device(device)
{

}

Keyboard::~Keyboard() {
  if (keymap) {
		xkb_keymap_unref(keymap);
	}
  wl_list_remove(&key.link);
	wl_list_remove(&modifiers.link);
}

bool Keyboard::handle_keybinding(xkb_keysym_t sym)
{
  switch (sym)
    {
    case XKB_KEY_Escape:
      wl_display_terminate(server->display);
      break;
    case XKB_KEY_F1:
      {
  	if (wl_list_length(&server->views) < 2)
  	  {
  	    break;
  	  }
  	View *current_view = wl_container_of(server->views.next, current_view, link);
  	View *next_view = wl_container_of(current_view->link.next, next_view, link);
  	ServerView::focus_view(next_view, next_view->xdg_surface->surface);
  	wl_list_remove(&current_view->link);
  	wl_list_insert(server->views.prev, &current_view->link);
  	break;
      }
    default:
      return false;
    }
  return true;
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
  keycodes_states.update_state(event, (uint32_t)keycode, modifiers);
  if ((modifiers & WLR_MODIFIER_ALT) && event->state == WLR_KEY_PRESSED)
  {
    for (int i = 0; i < nsyms; i++)
    {
      std::cout << "SYM: " << syms[i] << std::endl;
      handled = handle_keybinding(syms[i]);
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
