#pragma once

# include "Wlroots.hpp"
# include "Listeners.hpp"

class Server;

class Keyboard : public Listeners::KeyboardListeners
{
public:
  Keyboard(Server *server, struct wlr_input_device *device);
  ~Keyboard() = default;

  void setModifiersListener();
  void setKeyListener();

  struct wl_list link;

private:
  Server *server;
  struct wlr_input_device *device;

private:
  bool handle_keybinding(xkb_keysym_t sym);
  void keyboard_handle_modifiers(struct wl_listener *listener, void *data);
  void keyboard_handle_key(struct wl_listener *listener, void *data);
};
