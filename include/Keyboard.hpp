#pragma once

# include "Wlroots.hpp"
# include "Listeners.hpp"
# include "ShortcutState.hpp"

#include <map>

class Server;

struct KeyboardListeners
{
  struct wl_listener modifiers;
  struct wl_listener key;
};

class Keyboard : public KeyboardListeners
{
public:
  Keyboard(struct wlr_input_device *device);
  ~Keyboard();

  void setModifiersListener();
  void setKeyListener();
  void configure();

  struct xkb_keymap *keymap;

  bool debug = false;

private:
  struct wlr_input_device *device;
  struct wl_event_source *key_repeat_source;
  ShortcutState keycodes_states;
  std::string repeatBinding = "";
  std::map<std::string, binding> shortcuts;

private:
  bool handle_keybinding();
  void keyboard_handle_modifiers(struct wl_listener *listener, void *data);
  void keyboard_handle_key(struct wl_listener *listener, void *data);
  static int keyboard_handle_repeat(void *data);
};
