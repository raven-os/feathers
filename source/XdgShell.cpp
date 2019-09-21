#include <cassert>

#include "XdgShell.hpp"
#include "Server.hpp"
#include "wm/Container.hpp"

XdgShell::XdgShell() {
  xdg_shell = wlr_xdg_shell_create(Server::getInstance().getWlDisplay());
  SET_LISTENER(XdgShell, XdgShellListeners, new_xdg_surface, server_new_xdg_surface);
  wl_signal_add(&xdg_shell->events.new_surface, &new_xdg_surface);
}

void XdgShell::xdg_surface_destroy([[maybe_unused]]struct wl_listener *listener, [[maybe_unused]]void *data)
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

void XdgShell::server_new_xdg_surface([[maybe_unused]]struct wl_listener *listener, [[maybe_unused]]void *data)
{
  struct wlr_xdg_surface *xdg_surface = static_cast<struct wlr_xdg_surface *>(data);

  if (xdg_surface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL)
    Server::getInstance().getViews().emplace_back(new View(xdg_surface->surface));
};
