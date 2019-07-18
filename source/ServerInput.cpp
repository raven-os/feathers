#include "ServerInput.hpp"
#include "Server.hpp"

ServerInput::ServerInput() : server(Server::getInstance()) {
  SET_LISTENER(ServerInput, ServerInputListeners, new_input, server_new_input);
  wl_signal_add(&server.backend->events.new_input, &new_input);
}

void ServerInput::server_new_input([[maybe_unused]]struct wl_listener *listener, void *data)
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
  if (!keyboards.empty())
  {
    caps |= WL_SEAT_CAPABILITY_KEYBOARD;
  }

  wlr_seat_set_capabilities(server.seat.getSeat(), caps);
}

void ServerInput::server_new_keyboard(struct wlr_input_device *device)
{
  std::unique_ptr<Keyboard> keyboard(new Keyboard(device));

  keyboard->configure();
  keyboard->setModifiersListener();
  keyboard->setKeyListener();

  wlr_seat_set_keyboard(server.seat.getSeat(), device);
  keyboards.emplace_back(std::move(keyboard));
}

void ServerInput::server_new_pointer(struct wlr_input_device *device)
{
  wlr_cursor_attach_input_device(server.cursor.cursor, device);
}
