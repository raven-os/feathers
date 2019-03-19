#include "Keyboard.hpp"
#include "Server.hpp"

Keyboard::Keyboard(Server *server, struct wlr_input_device *device) : server(server), device(device) {

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

void Keyboard::keyboard_handle_modifiers(struct wl_listener *listener, void *data)
{
  // Keyboard *keyboard = wl_container_of(listener, keyboard, modifiers);
  wlr_seat_set_keyboard(server->seat, device);
  wlr_seat_keyboard_notify_modifiers(server->seat,
				     &device->keyboard->modifiers);
}

void Keyboard::keyboard_handle_key(struct wl_listener *listener, void *data)
{
  // Keyboard *keyboard = wl_container_of(listener, keyboard, key);
  // Server *server = keyboard->server;
  struct wlr_event_keyboard_key *event = static_cast<struct wlr_event_keyboard_key *>(data);
  struct wlr_seat *seat = server->seat;

  uint32_t keycode = event->keycode + 8;
  const xkb_keysym_t *syms;
  int nsyms = xkb_state_key_get_syms(device->keyboard->xkb_state, keycode, &syms);

  bool handled = false;
  uint32_t modifiers = wlr_keyboard_get_modifiers(device->keyboard);
  if ((modifiers & WLR_MODIFIER_ALT) && event->state == WLR_KEY_PRESSED)
  {
    for (int i = 0; i < nsyms; i++)
    {
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

void Keyboard::setModifiersListener() {
  SET_LISTENER(Keyboard, KeyboardListeners, modifiers, keyboard_handle_modifiers);
}

void Keyboard::setKeyListener() {
  SET_LISTENER(Keyboard, KeyboardListeners, key, keyboard_handle_key);
}
