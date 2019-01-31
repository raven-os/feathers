#ifndef KEYBOARD_HPP_
# define KEYBOARD_HPP_

# include "Wlroots.hpp"

class Server;

struct Keyboard
{
  struct wl_list link;
  Server *server;
  struct wlr_input_device *device;

  struct wl_listener modifiers;
  struct wl_listener key;
};

#endif /* !KEYBOARD_HPP_ */
