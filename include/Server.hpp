#pragma once

# include "Wlroots.hpp"
# include "View.hpp"
# include "XdgShell.hpp"
# include "Output.hpp"
# include "ServerCursor.hpp"
# include "ServerInput.hpp"
# include "ServerOutput.hpp"
# include "Seat.hpp"
# include "conf/Configuration.hpp"

enum class OpenType : uint8_t
  {
   dontCare = 0,
   below,
   right,
   floating
  };

class Server
{
public:
  ~Server();


  void run();

  conf::Configuration configuration;

  struct DisplayDeleter
  {
    void operator()(struct wl_display *display) const noexcept
    {
      wl_display_destroy_clients(display);
      wl_display_destroy(display);
    }
  };

  static Server &getInstance();

  std::unique_ptr<struct wl_display, DisplayDeleter> display;
  struct wlr_backend *backend;
  struct wlr_renderer *renderer;
  struct wl_event_loop *wl_event_loop;

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
  OpenType openType;

  wl_display *getWlDisplay() const noexcept
  {
    return display.get();
  }

private:
  Server();

  static std::unique_ptr<Server> _instance;
};
