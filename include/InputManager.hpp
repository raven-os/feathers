#pragma once

# include "Wlroots.hpp"
# include "Keyboard.hpp"
# include "Listeners.hpp"

# include <memory>
# include <vector>

class Server;

struct InputManagerListeners
{
  wl_listener new_input;
};

class InputManager : public InputManagerListeners
{
public:
  InputManager();
  ~InputManager() = default;

  void server_new_input(wl_listener *listener, void *data);

private:
  std::vector<std::unique_ptr<Keyboard>> keyboards;

  bool handle_keybinding(xkb_keysym_t sym);
  void server_new_keyboard(wlr_input_device *device);
  void server_new_pointer(wlr_input_device *device);

};
