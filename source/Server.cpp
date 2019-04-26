#include "Server.hpp"
#include "XdgShell.hpp"
#include "ServerCursor.hpp"
#include "ServerOutput.hpp"
#include "ServerInput.hpp"
#include "Seat.hpp"

Server::Server()
  : windowTree(wm::WindowData{wm::Container{{{{0, 0}}, {{1920, 1080}}}}})
  , display(wl_display_create())
  , backend(wlr_backend_autocreate(display, nullptr)) // nullptr can be replaced with a custom rendererx
  , renderer(wlr_backend_get_renderer(backend))
  , xdgShell(new XdgShell(this))
  , output(this)
  , cursor(this)
  , input(this)
  , seat(this)
{

  wlr_renderer_init_wl_display(renderer, display);
  wlr_compositor_create(display, renderer);
  wlr_data_device_manager_create(display);


  wl_list_init(&views);
  

}

Server::~Server()
{
  wl_display_destroy_clients(display);
  wl_display_destroy(display);
}

void Server::run()
{
  const char *socket = wl_display_add_socket_auto(display);
  if (!socket)
    {
      wlr_backend_destroy(backend);
      // TODO THROW
    }

  if (!wlr_backend_start(backend))
    {
      wlr_backend_destroy(backend);
      wl_display_destroy(display);
      // TODO THROW
    }

  setenv("WAYLAND_DISPLAY", socket, true);
  wlr_log(WLR_INFO, "Running Wayland compositor on WAYLAND_DISPLAY=%s",
	  socket);
  wl_display_run(display);
}
