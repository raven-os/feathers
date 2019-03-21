#pragma once

# include "Wlroots.hpp"
# include "View.hpp"
# include "XdgShell.hpp"
# include "Output.hpp"
# include "ServerCursor.hpp"
# include "ServerInput.hpp"
# include "ServerOutput.hpp"
# include "Seat.hpp"

class Server
{
public:
  Server();
  ~Server();

  void run();

  struct wl_display *display;
  struct wlr_backend *backend;
  struct wlr_renderer *renderer;

  // struct wlr_xdg_shell *xdg_shell;
  // struct wl_listener new_xdg_surface;
  XdgShell *xdgShell;
  struct wl_list views;

  ServerCursor *cursor;
  ServerInput *input;
  ServerOutput *output;
  Seat *seat;

  View *grabbed_view;
  double grab_x, grab_y;
  int grab_width, grab_height;
  uint32_t resize_edges;

};
