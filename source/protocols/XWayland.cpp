#include "protocols/XWayland.hpp"
#include "XdgView.hpp"
#include "Server.hpp"

XWayland::XWayland() {
    Server &server = Server::getInstance();
    xwayland = wlr_xwayland_create(server.getWlDisplay(), server.compositor, true);
    wlr_xwayland_set_seat(xwayland, server.seat.getSeat());
    
    //server->xwayland_new_surface.notify = xwayland_new_surface_event;
    SET_LISTENER(XWayland, XWaylandListeners, new_xwayland_surface, server_new_xwayland_surface);
	wl_signal_add(&xwayland->events.new_surface, &new_xwayland_surface);
}

void XWayland::server_new_xwayland_surface(wl_listener *listener, void *data)
{
    wlr_xwayland_surface *xwayland_surface = static_cast<wlr_xwayland_surface *>(data);

    Workspace *workspace = Server::getInstance().outputManager.getActiveWorkspace();
    workspace->getViews().emplace_back(new XdgView(xwayland_surface->surface, workspace));
}