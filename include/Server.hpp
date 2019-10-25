#pragma once

# include "Wlroots.hpp"
# include "XdgShell.hpp"
# include "XdgShellV6.hpp"
# include "LayerShell.hpp"
# include "ServerCursor.hpp"
# include "InputManager.hpp"
# include "OutputManager.hpp"
# include "Seat.hpp"
# include "conf/Configuration.hpp"

enum class OpenType : uint8_t
  {
   dontCare = 0,
   below,
   right,
   floating
  };

class View;

class Server
{
public:
  Server(Server const &) = delete;
  Server(Server &&) = delete;
  ~Server() noexcept;


  void run();

  conf::Configuration configuration;

  struct DisplayDeleter
  {
    void operator()(wl_display *display) const noexcept
    {
      wl_display_destroy_clients(display);
      wl_display_destroy(display);
    }
  };

  static Server &getInstance() noexcept
  {
      return _instance;
  };

  std::unique_ptr<wl_display, DisplayDeleter> display;
  wlr_backend *backend;
  wlr_renderer *renderer;
  struct wl_event_loop *wl_event_loop;

  std::vector<std::unique_ptr<View>> &getViews();
  wm::WindowTree &getActiveWindowTree();

  OutputManager outputManager;
  XdgShell *xdgShell;
  XdgShellV6 *xdgShellV6;
  LayerShell layerShell;
  ServerCursor cursor;
  InputManager inputManager;
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

  void startupCommands() const;

  static Server _instance;
};
