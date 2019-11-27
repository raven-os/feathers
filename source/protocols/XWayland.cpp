#include "protocols/XWayland.hpp"
#include "Server.hpp"

XWayland::XWayland() {
    Server &server = Server::getInstance();
    xwayland = wlr_xwayland_create(server.getWlDisplay(), server.compositor, true);
}

void XWayland::server_new_xwayland_surface(wl_listener *listener, void *data)
{
    wlr_xwayland_surface *xwayland_surface = static_cast<wlr_xwayland_surface *>(data);
}