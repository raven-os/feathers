#pragma once

# include <cstring>
# include <map>

# include "Wlroots.hpp"
# include "Server.hpp"

namespace Commands
{
  static std::map<std::string, int> concordances = {
    {"&", 0},
    {"1", 0},
    {"é", 1},
    {"2", 1},
    {"\"", 2},
    {"3", 2},
    {"\\", 3},
    {"4", 3},
    {"(", 4},
    {"5", 4},
    {"-", 5},
    {"6", 5},
    {"è", 6},
    {"7", 6},
    {"_", 7},
    {"8", 7},
    {"ç", 8},
    {"9", 8}
  };

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
  void switch_workspace(int direction, void *data);
  void switch_window_from_workspace(int direction);
  void new_workspace();
  void close_workspace();
}
