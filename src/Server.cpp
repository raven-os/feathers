#include <unistd.h>
#include "Server.hpp"
#include "XdgShell.hpp"
#include "ServerCursor.hpp"
#include "ServerOutput.hpp"
#include "ServerInput.hpp"
#include "Seat.hpp"

Server::Server()
{
  wlr_log_init(WLR_DEBUG, NULL);

  display = wl_display_create();
  // nullptr can be replaced with a custom renderer
  backend = wlr_backend_autocreate(display, nullptr);
  renderer = wlr_backend_get_renderer(backend);

  wlr_renderer_init_wl_display(renderer, display);
  wlr_compositor_create(display, renderer);
  wlr_data_device_manager_create(display);

  output = new ServerOutput(this);

  wl_list_init(&views);
  xdgShell = new XdgShell(this);

  cursor = new ServerCursor(this);
  input = new ServerInput(this);
  seat = new Seat(this);
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
  if (fork() == 0)
    {
      execl("/bin/sh", "/bin/sh", "-c", "weston-terminal", nullptr);
    }
  wlr_log(WLR_INFO, "Running Wayland compositor on WAYLAND_DISPLAY=%s",
	  socket);
  wl_display_run(display);
}
