#pragma once

# include "Wlroots.hpp"
# include "Listeners.hpp"
# include "ShortcutState.hpp"

#include <map>

class Server;

struct KeyboardListeners
{
  wl_listener modifiers;
  wl_listener key;
};

class Keyboard : public KeyboardListeners
{
public:
  Keyboard(wlr_input_device *device);
  ~Keyboard();

  void setModifiersListener();
  void setKeyListener();
  void configure();

  xkb_keymap *keymap;

  bool debug = false;

private:
  wlr_input_device *device;
  wl_event_source *key_repeat_source;
  ShortcutState keycodes_states;
  std::string repeatBinding = "";
  std::map<std::string, binding> default_shortcuts;
  std::map<std::string, binding> shortcuts;

private:
  void disarm_key_repeat();
  void update_shortcuts();
  std::vector<std::string> split_shortcut(std::string key);
  std::string get_active_binding();
  void keyboard_handle_modifiers(wl_listener *listener, void *data);
  void keyboard_handle_key(wl_listener *listener, void *data);
  void parse_shortcuts();
  static int keyboard_handle_repeat(void *data);
  void configureKeyRepeat();
};
