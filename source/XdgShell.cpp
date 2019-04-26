#include <cassert>

#include "XdgShell.hpp"
#include "Server.hpp"
#include "wm/Container.hpp"

XdgShell::XdgShell(Server *server) : server(server) {
  xdg_shell = wlr_xdg_shell_v6_create(server->display);
  SET_LISTENER(XdgShell, XdgShellListeners, new_xdg_surface, server_new_xdg_surface);
  wl_signal_add(&xdg_shell->events.new_surface, &new_xdg_surface);
}

void XdgShell::xdg_surface_destroy([[maybe_unused]]struct wl_listener *listener, [[maybe_unused]]void *data)
{
  View *view = wl_container_of(listener, view, destroy);

  server->views.erase(std::find_if(server->views.begin(), server->views.end(),
				   [view](auto const &ptr)
				   {
				     return ptr.get() == view;
				   }));
};

void XdgShell::server_new_xdg_surface([[maybe_unused]]struct wl_listener *listener, [[maybe_unused]]void *data)
{
  struct wlr_xdg_surface_v6 *xdg_surface = static_cast<struct wlr_xdg_surface_v6 *>(data);

  if (xdg_surface->role != WLR_XDG_SURFACE_V6_ROLE_TOPLEVEL)
    {
      assert(!"not handled yet");
      return;
    }
  server->views.emplace_back(new View(server, xdg_surface));
};
