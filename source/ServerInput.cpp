#include "ServerInput.hpp"
#include "Server.hpp"

ServerInput::ServerInput(Server *server) : server(server) {
  wl_list_init(&keyboards);
  SET_LISTENER(ServerInput, ServerInputListeners, new_input, server_new_input);
  wl_signal_add(&server->backend->events.new_input, &new_input);
}

void ServerInput::server_new_input(struct wl_listener *listener, void *data)
{
  struct wlr_input_device *device = static_cast<struct wlr_input_device *>(data);
  switch (device->type)
  {
    case WLR_INPUT_DEVICE_KEYBOARD:
      server_new_keyboard(device);
      break;
    case WLR_INPUT_DEVICE_POINTER:
      server_new_pointer(device);
      break;
    default:
      break;
  }
  uint32_t caps = WL_SEAT_CAPABILITY_POINTER;
  if (!wl_list_empty(&keyboards))
  {
    caps |= WL_SEAT_CAPABILITY_KEYBOARD;
  }

  wlr_seat_set_capabilities(server->seat->getSeat(), caps);
}

void ServerInput::server_new_keyboard(struct wlr_input_device *device)
{
  keyboard = new Keyboard(server, device);

  struct xkb_rule_names rules = { 0 };
  struct xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
  struct xkb_keymap *keymap = xkb_map_new_from_names(context, &rules, XKB_KEYMAP_COMPILE_NO_FLAGS);

  xkb_keymap_unref(keyboard->keymap);
  keyboard->setKeyMap(keymap);
  wlr_keyboard_set_keymap(device->keyboard, keymap);
  xkb_context_unref(context);
  wlr_keyboard_set_repeat_info(device->keyboard, 25, 600);

  keyboard->setModifiersListener();
  keyboard->setKeyListener();

  wlr_seat_set_keyboard(server->seat->getSeat(), device);
  wl_list_insert(&keyboards, &keyboard->link);
}

void ServerInput::server_new_pointer(struct wlr_input_device *device)
{
  wlr_cursor_attach_input_device(server->cursor->cursor, device);
}
