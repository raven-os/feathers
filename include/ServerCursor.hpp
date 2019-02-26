#pragma once

# include "Wlroots.hpp"

enum CursorMode
  {
   CURSOR_PASSTHROUGH,
   CURSOR_MOVE,
   CURSOR_RESIZE,
  };

namespace ServerCursor
{
  void server_cursor_motion(struct wl_listener *listener, void *data);
  void server_cursor_motion_absolute(struct wl_listener *listener, void *data);
  void server_cursor_button(struct wl_listener *listener, void *data);
  void server_cursor_axis(struct wl_listener *listener, void *data);
};
