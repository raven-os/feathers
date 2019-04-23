#pragma once

# include "Wlroots.hpp"
# include "Listeners.hpp"
# include "FthShortcutState.hpp"

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
  void configure();

  struct wl_list link;
  struct xkb_keymap *keymap;

  //TODO repeat info (wlr_keyboard_set_repeat_info)
  /*
    struct wl_event_source *key_repeat_src;
    //TODO struct for binding
  */


private:
  Server *server;
  struct wlr_input_device *device;
  FthShortcutState keycodes_states;

private:
  bool handle_keybinding(xkb_keysym_t sym);
  void keyboard_handle_modifiers(struct wl_listener *listener, void *data);
  void keyboard_handle_key(struct wl_listener *listener, void *data);
};
