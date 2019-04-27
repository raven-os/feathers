#pragma once

# include "Wlroots.hpp"
# include "View.hpp"
# include "XdgShell.hpp"
# include "Output.hpp"
# include "ServerCursor.hpp"
# include "ServerInput.hpp"
# include "ServerOutput.hpp"
# include "Seat.hpp"
# include "wm/WindowTree.hpp"

class Server
{
public:
  Server();
  ~Server();

  void run();

  wm::WindowTree windowTree;

  struct DisplayDeleter
  {
    void operator()(struct wl_display *display) const noexcept
    {
      wl_display_destroy_clients(display);
      wl_display_destroy(display);
    }
  };

  std::unique_ptr<struct wl_display, DisplayDeleter> display;
  struct wlr_backend *backend;
  struct wlr_renderer *renderer;

  std::vector<std::unique_ptr<View>> views;

  ServerOutput output;
  XdgShell *xdgShell;
  ServerCursor cursor;
  ServerInput input;
  Seat seat;

  View *grabbed_view;
  double grab_x, grab_y;
  int grab_width, grab_height;
  uint32_t resize_edges;

  wl_display *getWlDisplay() const noexcept
  {
    return display.get();
  }
};
