#pragma once

# include "Wlroots.hpp"
# include "Keyboard.hpp"
# include "Listeners.hpp"

# include <memory>
# include <vector>

class Server;

struct ServerInputListeners
{
  struct wl_listener new_input;
};

class ServerInput : public ServerInputListeners
{
public:
  ServerInput();
  ~ServerInput() = default;

  void server_new_input(struct wl_listener *listener, void *data);

private:
  std::vector<std::unique_ptr<Keyboard>> keyboards;

  bool handle_keybinding(xkb_keysym_t sym);
  void server_new_keyboard(struct wlr_input_device *device);
  void server_new_pointer(struct wlr_input_device *device);

};
