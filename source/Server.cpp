#include "Server.hpp"
#include "ServerCursor.hpp"
#include "OutputManager.hpp"
#include "InputManager.hpp"
#include "Seat.hpp"
#include "LayerSurface.hpp"
#include "Output.hpp"
#include "View.hpp"
#include "XdgView.hpp"
#include <unistd.h>

Server Server::_instance = Server();

Server::Server()
  : display(wl_display_create())
  , backend(wlr_backend_autocreate(getWlDisplay(), nullptr)) // nullptr can be replaced with a custom rendererx
  , renderer([this]()
	     {
	       auto *renderer = wlr_backend_get_renderer(backend);

	       wlr_renderer_init_wl_display(renderer, getWlDisplay());
	       compositor = wlr_compositor_create(getWlDisplay(), renderer);

	       return renderer;
	     }())
  , wl_event_loop(wl_display_get_event_loop(getWlDisplay()))
  , outputManager()
  , xdgShell(new XdgShell())
  , xdgShellV6(new XdgShellV6())
  , cursor()
  , inputManager()
  , seat()
  // , ipcServer([this]()
  //             {
  //               std::string feathersSocket = configuration.getOnce("feathers_socket");
  //               if (feathersSocket.empty())
  //                 feathersSocket = "/tmp/featherssocket";
  //               setenv("FEATHERS_SOCKET", feathersSocket.c_str(), true);
  //               return feathersSocket;
  //             }(), this)
  , xWayland(new XWayland(true))
  , openType(OpenType::dontCare)
{
  wlr_data_device_manager_create(getWlDisplay());

  compositor_new_surface.notify = [](wl_listener *listener, void *data){ Server::getInstance().handle_compositor_new_surface(listener, data); };
  wl_signal_add(&compositor->events.new_surface, &compositor_new_surface);
}

Server::~Server() noexcept = default;

wlr_surface *Server::getFocusedSurface() const noexcept
{
  if (LayerSurface *layerSurface = getFocusedLayerSurface())
    return layerSurface->surface;
  if (XdgView *view = getFocusedView())
    return view->surface;
  return nullptr;
}


std::vector<std::unique_ptr<XdgView>> &Server::getViews()
{
  return outputManager.getActiveWorkspace()->getViews();
}

wm::WindowTree &Server::getActiveWindowTree()
{
  return outputManager.getActiveWorkspace()->getWindowTree();
}

// TODO REFACTO IN ANOTHER CLASS
void Server::startupCommands(char *command) const
{
  // Launch waybar
  if (fork() == 0)
    {
      execl("/bin/sh", "/bin/sh", "-c", "waybar", nullptr);
    }
  if (fork() == 0)
  {
    execl("/bin/sh", "/bin/sh", "-c", command, nullptr);
  }
}

void Server::run(char *command)
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

  startupCommands(command);

  wlr_xwayland_create(getWlDisplay(), compositor, true);
  wl_display_run(getWlDisplay());
}

void Server::handle_compositor_new_surface(wl_listener *listener, void *data)
{
  wlr_surface *surface = static_cast<wlr_surface *>(data);

  View *view = new View(surface);
  surface->data = view;

  std::cout << "COMPOTIOR NEW SURFACE:" << surface->role << std::endl;
}