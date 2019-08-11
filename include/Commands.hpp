#pragma once

# include <cstring>

# include "Wlroots.hpp"
# include "Server.hpp"

namespace Commands
{
  void open_terminal();
  void open_config_editor();
  void toggle_fullscreen();
  void switch_window();
  void toggle_float_window();
  void switch_container_direction();
  void close_compositor();
  void switch_focus_up();
  void switch_focus_left();
  void switch_focus_down();
  void switch_focus_right();
  void switch_workspace(bool next);
  void new_workspace();
}
