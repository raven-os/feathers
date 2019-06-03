#pragma once

# include <cstring>

# include "Wlroots.hpp"
# include "Server.hpp"

namespace Commands
{
  void open_terminal(Server *server);
  void open_config_editor(Server *server);
  void toggle_fullscreen(Server *server);
  void switch_window(Server *server);
  void toggle_float_window(Server *server);
  void switch_container_direction(Server *server);
  void close_compositor(Server *server);
  void switch_focus_up(Server *server);
  void switch_focus_left(Server *server);
  void switch_focus_down(Server *server);
  void switch_focus_right(Server *server);
}
