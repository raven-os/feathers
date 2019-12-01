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

private:
    wlr_xwayland *xwayland;
};