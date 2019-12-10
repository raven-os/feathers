#pragma once

# include "Wlroots.hpp"
# include "protocols/XdgShell.hpp"
# include "protocols/XdgShellV6.hpp"
# include "protocols/XWayland.hpp"
# include "LayerShell.hpp"
# include "ServerCursor.hpp"
# include "InputManager.hpp"
# include "OutputManager.hpp"
# include "Seat.hpp"
# include "IpcServer.hpp"
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


  void run(char *command);

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
  wlr_compositor *compositor;
  wlr_backend *backend;
  wlr_renderer *renderer;
  struct wl_event_loop *wl_event_loop;

  std::vector<std::unique_ptr<XdgView>> &getViews();
  wm::WindowTree &getActiveWindowTree();

  XdgView *getFocusedView() const noexcept
  {
    if (outputManager.getActiveWorkspace()->getViews().empty())
      return nullptr;
    return outputManager.getActiveWorkspace()->getViews().front().get();
  }

  wlr_surface *getFocusedSurface() const noexcept;

  OutputManager outputManager;

  XdgShell *xdgShell;
  XdgShellV6 *xdgShellV6;

  LayerShell layerShell;
  ServerCursor cursor;
  InputManager inputManager;
  Seat seat;
  //IpcServer ipcServer;

  XWayland *xWayland;

  XdgView *grabbed_view;
  double grab_x, grab_y;
  int grab_width, grab_height;
  uint32_t resize_edges;
  OpenType openType;

  wl_display *getWlDisplay() const noexcept
  {
    return display.get();
  }

  LayerSurface *getFocusedLayerSurface() const noexcept
  {
    return layerSurface;
  }

  void setFocusedLayerSurface(LayerSurface *layerSurface) noexcept
  {
    this->layerSurface = layerSurface;
  }


private:
  LayerSurface *layerSurface = nullptr;
  wl_listener compositor_new_surface;

  void handle_compositor_new_surface(struct wl_listener *listener, void *data);

  Server();

  void startupCommands(char *command) const;

  static Server _instance;
};
