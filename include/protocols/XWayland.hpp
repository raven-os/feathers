#pragma once

# include "Wlroots.hpp"

extern "C" {

# include <wlr/xwayland.h>

}

struct XWaylandListeners {
    wl_listener new_xwayland_surface;
};

class XWayland : XWaylandListeners
{
public:
    XWayland();
    ~XWayland() = default;

    void server_new_xwayland_surface(wl_listener *listener, void *data);
    void xwayland_surface_destroy(wl_listener *listener, void *data);

    static void xwayland_surface_get_geometry(wlr_xwayland_surface *surface, wlr_box *box);

private:
    wlr_xwayland *xwayland;
};