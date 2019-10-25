#include <cassert>

#include "XdgShellV6.hpp"
#include "Server.hpp"
#include "wm/Container.hpp"

XdgShellV6::XdgShellV6() {
  xdg_shell = wlr_xdg_shell_v6_create(Server::getInstance().getWlDisplay());
  SET_LISTENER(XdgShellV6, XdgShellV6Listeners, new_xdg_surface, server_new_xdg_surface);
  wl_signal_add(&xdg_shell->events.new_surface, &new_xdg_surface);
}

void XdgShellV6::xdg_surface_destroy([[maybe_unused]]wl_listener *listener, [[maybe_unused]]void *data)
{
  Server &server = Server::getInstance();
  View *view = wl_container_of(listener, view, destroy);

  if (server.getViews().front().get() == view)
    {
      server.seat.getSeat()->keyboard_state.focused_surface = nullptr;
    }
  server.getViews().erase(std::find_if(server.getViews().begin(), server.getViews().end(),
				   [view](auto const &ptr)
				   {
				     return ptr.get() == view;
				   }));
  if (!server.getViews().empty())
    {
      std::unique_ptr<View> &currentView = server.getViews().front();
      currentView->focus_view();
    }
};

void XdgShellV6::server_new_xdg_surface([[maybe_unused]]wl_listener *listener, [[maybe_unused]]void *data)
{
  struct wlr_xdg_surface_v6 *xdg_surface = static_cast<struct wlr_xdg_surface_v6 *>(data);

  if (xdg_surface->role == WLR_XDG_SURFACE_V6_ROLE_TOPLEVEL)
    Server::getInstance().getViews().emplace_back(new View(xdg_surface->surface));
};
