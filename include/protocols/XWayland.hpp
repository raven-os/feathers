#pragma once

# include "Wlroots.hpp"

extern "C" {

# include <wlr/xwayland.h>
#include <xcb/xproto.h>

}

enum atom_name {
	NET_WM_WINDOW_TYPE_NORMAL,
	NET_WM_WINDOW_TYPE_DIALOG,
	NET_WM_WINDOW_TYPE_UTILITY,
	NET_WM_WINDOW_TYPE_TOOLBAR,
	NET_WM_WINDOW_TYPE_SPLASH,
	NET_WM_WINDOW_TYPE_MENU,
	NET_WM_WINDOW_TYPE_DROPDOWN_MENU,
	NET_WM_WINDOW_TYPE_POPUP_MENU,
	NET_WM_WINDOW_TYPE_TOOLTIP,
	NET_WM_WINDOW_TYPE_NOTIFICATION,
	NET_WM_STATE_MODAL,
	ATOM_LAST,
};

struct XWaylandListeners {
    wl_listener new_xwayland_surface;
    wl_listener xwayland_ready;
};

class XWayland : XWaylandListeners
{
public:
    XWayland(bool lazy);
    ~XWayland() = default;

    void server_new_xwayland_surface(wl_listener *listener, void *data);
    void xwayland_surface_destroy(wl_listener *listener, void *data);
    void handle_xwayland_ready(wl_listener *listener, void *data);

    static void xwayland_surface_get_geometry(wlr_xwayland_surface *surface, wlr_box *box);

private:
    wlr_xwayland *xwayland;
    xcb_atom_t atoms[ATOM_LAST];
};