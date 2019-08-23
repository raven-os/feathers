#include "Server.hpp"
#include "XdgShell.hpp"
#include "ServerCursor.hpp"
#include "OutputManager.hpp"
#include "InputManager.hpp"
#include "Seat.hpp"

Server Server::_instance = Server();

Server::Server()
  : display(wl_display_create())
  , backend(wlr_backend_autocreate(getWlDisplay(), nullptr)) // nullptr can be replaced with a custom rendererx
  , renderer([this]()
	     {
	       auto *renderer = wlr_backend_get_renderer(backend);

	       wlr_renderer_init_wl_display(renderer, getWlDisplay());
	       wlr_compositor_create(getWlDisplay(), renderer);

	       return renderer;
	     }())
  , wl_event_loop(wl_display_get_event_loop(getWlDisplay()))
  , outputManager()
  , xdgShell(new XdgShell())
  , xdgShellV6(new XdgShellV6())
  , cursor()
  , inputManager()
  , seat()
  , openType(OpenType::dontCare)
{
  wlr_data_device_manager_create(getWlDisplay());
}

Server::~Server()
{
  // wl_display_destroy_clients(display);
  // wl_display_destroy(display);
}

std::vector<std::unique_ptr<View>> &Server::getViews()
{
  return outputManager.getActiveWorkspace()->getViews();
}

wm::WindowTree &Server::getActiveWindowTree()
{
  return outputManager.getActiveWorkspace()->getWindowTree();
}

void Server::run()
{
  const char *socket = wl_display_add_socket_auto(getWlDisplay());
  if (!socket)
    {
      wlr_backend_destroy(backend);
      // TODO THROW
    }

  if (!wlr_backend_start(backend))
    {
      wlr_backend_destroy(backend);
      wl_display_destroy(getWlDisplay());
      // TODO THROW
    }

  setenv("WAYLAND_DISPLAY", socket, true);
  wlr_log(WLR_INFO, "Running Wayland compositor on WAYLAND_DISPLAY=%s",
	  socket);
  wl_display_run(getWlDisplay());
}
