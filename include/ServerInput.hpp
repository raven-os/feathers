#pragma once

# include "Wlroots.hpp"
# include "Keyboard.hpp"
# include "Listeners.hpp"

class Server;

class ServerInput : public Listeners::ServerInputListeners
{
public:
  ServerInput(Server *server);
  ~ServerInput() = default;

  // void keyboard_handle_modifiers(struct wl_listener *listener, void *data);
  // void keyboard_handle_key(struct wl_listener *listener, void *data);
  void server_new_input(struct wl_listener *listener, void *data);

private:
  Server *server;
  Keyboard *keyboard;

  struct wl_list keyboards;

  bool handle_keybinding(xkb_keysym_t sym);
  void server_new_keyboard(struct wlr_input_device *device);
  void server_new_pointer(struct wlr_input_device *device);

};
