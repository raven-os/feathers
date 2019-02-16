#ifndef SERVER_HPP_
# define SERVER_HPP_

# include "Wlroots.hpp"
# include "View.hpp"
# include "Output.hpp"
# include "Keyboard.hpp"
# include "ServerCursor.hpp"

class Server
{
public:
  Server();
  ~Server();

  void run();

  struct wl_display *display;
  struct wlr_backend *backend;
  struct wlr_renderer *renderer;

  struct wlr_xdg_shell *xdg_shell;
  struct wl_listener new_xdg_surface;
  struct wl_list views;

  ServerCursor *cursor;

  // struct wlr_cursor *cursor;
  // struct wlr_xcursor_manager *cursor_mgr;
  // struct wl_listener cursor_motion;
  // struct wl_listener cursor_motion_absolute;
  // struct wl_listener cursor_button;
  // struct wl_listener cursor_axis;
  // CursorMode cursor_mode;

  struct wlr_seat *seat;
  struct wl_listener new_input;
  struct wl_listener request_cursor;
  struct wl_list keyboards;
  View *grabbed_view;
  double grab_x, grab_y;
  int grab_width, grab_height;
  uint32_t resize_edges;

  struct wlr_output_layout *output_layout;
  struct wl_list outputs;
  struct wl_listener new_output;
};

#endif /* !SERVER_HPP_ */
