#include "protocols/XWayland.hpp"
#include "XdgView.hpp"
#include "Server.hpp"

XWayland::XWayland() {
    Server &server = Server::getInstance();
    xwayland = wlr_xwayland_create(server.getWlDisplay(), server.compositor, true);
    wlr_xwayland_set_seat(xwayland, server.seat.getSeat());
    
    SET_LISTENER(XWayland, XWaylandListeners, new_xwayland_surface, server_new_xwayland_surface);
	wl_signal_add(&xwayland->events.new_surface, &new_xwayland_surface);
}

void XWayland::xwayland_surface_destroy(wl_listener *listener, void *data)
{
  Server &server = Server::getInstance();
  XdgView *view = wl_container_of(listener, view, destroy);

  if (server.getFocusedView() == view)
    {
      server.seat.getSeat()->keyboard_state.focused_surface = nullptr;
    }
  auto &views(view->workspace->getViews());

  views.erase(std::find_if(views.begin(), views.end(),
			   [view](auto const &ptr) noexcept
			   {
			     return ptr.get() == view;
			   }));
  if (XdgView *view = server.getFocusedView())
    view->focus_view();
}

void XWayland::server_new_xwayland_surface(wl_listener *listener, void *data)
{
    wlr_xwayland_surface *xwayland_surface = static_cast<wlr_xwayland_surface *>(data);

    wlr_xwayland_surface_ping(xwayland_surface);
    Workspace *workspace = Server::getInstance().outputManager.getActiveWorkspace();
    workspace->getViews().emplace_back(new XdgView(xwayland_surface->surface, workspace));
}