#ifndef SERVER_HPP_
# define SERVER_HPP_

# include "Wlroots.hpp"
# include "XdgShell.hpp"

class Server
{
public:
  Server();
  ~Server();

  struct wl_display *display;
  struct wlr_backend *backend;
  struct wlr_renderer *renderer;

  XdgShell *xdgShell;
  // struct wlr_xdg_shell *xdg_shell;
  // struct wl_listener new_xdg_surface;
  struct wl_list views;

  struct wlr_cursor *cursor;
  struct wlr_xcursor_manager *cursor_mgr;
  struct wl_listener cursor_motion;
  struct wl_listener cursor_motion_absolute;
  struct wl_listener cursor_button;
  struct wl_listener cursor_axis;

  struct wlr_seat *seat;
  struct wl_listener new_input;
  struct wl_listener request_cursor;
  struct wl_list keyboards;
  // enum tinywl_cursor_mode cursor_mode;
  // struct tinywl_view *grabbed_view;
  double grab_x, grab_y;
  int grab_width, grab_height;
  uint32_t resize_edges;

  struct wlr_output_layout *output_layout;
  struct wl_list outputs;
  struct wl_listener new_output;
};

struct Output
{
  struct wl_list link;
  Server *server;
  struct wlr_output *wlr_output;
  struct wl_listener frame;
};

struct View
{
  struct wl_list link;
  Server *server;
  struct wlr_xdg_surface *xdg_surface;
  struct wl_listener map;
  struct wl_listener unmap;
  struct wl_listener destroy;
  struct wl_listener request_move;
  struct wl_listener request_resize;
  bool mapped;
  int x, y;
};

/* Used to move all of the data necessary to render a surface from the top-level
 * frame handler to the per-surface render function. */
struct render_data
{
  struct wlr_output *output;
  struct wlr_renderer *renderer;
  struct View *view;
  struct timespec *when;
};

#endif /* !SERVER_HPP_ */
