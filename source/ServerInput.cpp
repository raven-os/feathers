#include "InputManager.hpp"
#include "Server.hpp"

InputManager::InputManager()
{
  SET_LISTENER(InputManager, InputManagerListeners, new_input, server_new_input);
  wl_signal_add(&Server::getInstance().backend->events.new_input, &new_input);
}

void InputManager::server_new_input([[maybe_unused]]struct wl_listener *listener, void *data)
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

  wlr_seat_set_capabilities(Server::getInstance().seat.getSeat(), caps);
}

void InputManager::server_new_keyboard(struct wlr_input_device *device)
{
  std::unique_ptr<Keyboard> keyboard(new Keyboard(device));

  keyboard->configure();
  keyboard->setModifiersListener();
  keyboard->setKeyListener();

  wlr_seat_set_keyboard(Server::getInstance().seat.getSeat(), device);
  keyboards.emplace_back(std::move(keyboard));
}

void InputManager::server_new_pointer(struct wlr_input_device *device)
{
  wlr_cursor_attach_input_device(Server::getInstance().cursor.cursor, device);
}
