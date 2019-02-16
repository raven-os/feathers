#include "ServerInput.hpp"
#include "Server.hpp"

namespace ServerInput
{
  static void keyboard_handle_modifiers(struct wl_listener *listener, void *data)
  {
    Keyboard *keyboard = wl_container_of(listener, keyboard, modifiers);
    wlr_seat_set_keyboard(keyboard->server->seat, keyboard->device);
    wlr_seat_keyboard_notify_modifiers(keyboard->server->seat,
				       &keyboard->device->keyboard->modifiers);
  }

  static bool handle_keybinding(Server *server, xkb_keysym_t sym)
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

  static void keyboard_handle_key(struct wl_listener *listener, void *data)
  {
    Keyboard *keyboard = wl_container_of(listener, keyboard, key);
    Server *server = keyboard->server;
    struct wlr_event_keyboard_key *event = static_cast<struct wlr_event_keyboard_key *>(data);
    struct wlr_seat *seat = server->seat;

    uint32_t keycode = event->keycode + 8;
    const xkb_keysym_t *syms;
    int nsyms = xkb_state_key_get_syms(keyboard->device->keyboard->xkb_state, keycode, &syms);

    bool handled = false;
    uint32_t modifiers = wlr_keyboard_get_modifiers(keyboard->device->keyboard);
    if ((modifiers & WLR_MODIFIER_ALT) && event->state == WLR_KEY_PRESSED)
      {
	for (int i = 0; i < nsyms; i++)
	  {
	    handled = handle_keybinding(server, syms[i]);
	  }
      }

    if (!handled)
      {
	wlr_seat_set_keyboard(seat, keyboard->device);
	wlr_seat_keyboard_notify_key(seat, event->time_msec,
				     event->keycode, event->state);
      }
  }

  static void server_new_keyboard(Server *server, struct wlr_input_device *device)
  {
    Keyboard *keyboard = new Keyboard();
    keyboard->server = server;
    keyboard->device = device;

    struct xkb_rule_names rules = { 0 };
    struct xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    struct xkb_keymap *keymap = xkb_map_new_from_names(context, &rules, XKB_KEYMAP_COMPILE_NO_FLAGS);

    wlr_keyboard_set_keymap(device->keyboard, keymap);
    xkb_keymap_unref(keymap);
    xkb_context_unref(context);
    wlr_keyboard_set_repeat_info(device->keyboard, 25, 600);

    keyboard->modifiers.notify = keyboard_handle_modifiers;
    wl_signal_add(&device->keyboard->events.modifiers, &keyboard->modifiers);
    keyboard->key.notify = keyboard_handle_key;
    wl_signal_add(&device->keyboard->events.key, &keyboard->key);

    wlr_seat_set_keyboard(server->seat, device);

    wl_list_insert(&server->keyboards, &keyboard->link);
  }

  static void server_new_pointer(Server *server, struct wlr_input_device *device)
  {
    wlr_cursor_attach_input_device(server->cursor->cursor, device);
  }

  void server_new_input(struct wl_listener *listener, void *data)
  {
    Server *server = wl_container_of(listener, server, new_input);
    struct wlr_input_device *device = static_cast<struct wlr_input_device *>(data);
    switch (device->type)
      {
      case WLR_INPUT_DEVICE_KEYBOARD:
	server_new_keyboard(server, device);
	break;
      case WLR_INPUT_DEVICE_POINTER:
	server_new_pointer(server, device);
	break;
      default:
	break;
      }
    uint32_t caps = WL_SEAT_CAPABILITY_POINTER;
    if (!wl_list_empty(&server->keyboards))
      {
	caps |= WL_SEAT_CAPABILITY_KEYBOARD;
      }

    wlr_seat_set_capabilities(server->seat, caps);
  }
}
