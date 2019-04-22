#pragma once

# include "Wlroots.hpp"
# include "Listeners.hpp"

class Server;

struct KeyboardListeners
{
  struct wl_listener modifiers;
  struct wl_listener key;
};

class Keyboard : public KeyboardListeners
{
public:
  Keyboard(Server *server, struct wlr_input_device *device);
  ~Keyboard();

  void setModifiersListener();
  void setKeyListener();
  void setKeyMap(struct xkb_keymap *keymap);

  struct wl_list link;
  struct xkb_keymap *keymap;

private:
  Server *server;
  struct wlr_input_device *device;

private:
  bool handle_keybinding(xkb_keysym_t sym);
  void keyboard_handle_modifiers(struct wl_listener *listener, void *data);
  void keyboard_handle_key(struct wl_listener *listener, void *data);
};
