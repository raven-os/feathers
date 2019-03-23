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
