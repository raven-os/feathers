#include "protocols/XWayland.hpp"
#include "XdgView.hpp"
//#include "XWaylandView.hpp"
#include "Server.hpp"

#include <iostream>

static const char *atom_map[ATOM_LAST] = {
	"_NET_WM_WINDOW_TYPE_NORMAL",
	"_NET_WM_WINDOW_TYPE_DIALOG",
	"_NET_WM_WINDOW_TYPE_UTILITY",
	"_NET_WM_WINDOW_TYPE_TOOLBAR",
	"_NET_WM_WINDOW_TYPE_SPLASH",
	"_NET_WM_WINDOW_TYPE_MENU",
	"_NET_WM_WINDOW_TYPE_DROPDOWN_MENU",
	"_NET_WM_WINDOW_TYPE_POPUP_MENU",
	"_NET_WM_WINDOW_TYPE_TOOLTIP",
	"_NET_WM_WINDOW_TYPE_NOTIFICATION",
	"_NET_WM_STATE_MODAL",
};

XWayland::XWayland(bool lazy) {
    Server &server = Server::getInstance();
    xwayland = wlr_xwayland_create(server.getWlDisplay(), server.compositor, lazy);
    wlr_xwayland_set_seat(xwayland, server.seat.getSeat());
    
    SET_LISTENER(XWayland, XWaylandListeners, new_xwayland_surface, server_new_xwayland_surface);
	  wl_signal_add(&xwayland->events.new_surface, &new_xwayland_surface);
    SET_LISTENER(XWayland, XWaylandListeners, xwayland_ready, handle_xwayland_ready);
    wl_signal_add(&xwayland->events.ready, &xwayland_ready);
    setenv("DISPLAY", xwayland->display_name, true);
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

void XWayland::handle_xwayland_ready(struct wl_listener *listener, void *data) {
	Server &server = Server::getInstance();
	
	xcb_connection_t *xcb_conn = xcb_connect(NULL, NULL);
	int err = xcb_connection_has_error(xcb_conn);
	if (err) {
		std::cerr << "XCB connect failed: " <<  err << std::endl;
		return;
	}

	xcb_intern_atom_cookie_t cookies[ATOM_LAST];
	for (size_t i = 0; i < ATOM_LAST; i++) {
		cookies[i] =
			xcb_intern_atom(xcb_conn, 0, strlen(atom_map[i]), atom_map[i]);
	}
	for (size_t i = 0; i < ATOM_LAST; i++) {
		xcb_generic_error_t *error = NULL;
		xcb_intern_atom_reply_t *reply =
			xcb_intern_atom_reply(xcb_conn, cookies[i], &error);
		if (reply != NULL && error == NULL) {
			atoms[i] = reply->atom;
		}
		free(reply);

		if (error != NULL) {
			std::cerr << "could not resolve atom " << atom_map[i] << " X11 error code " << error->error_code << std::endl;
			free(error);
			break;
		}
	}

	xcb_disconnect(xcb_conn);
}

void XWayland::server_new_xwayland_surface(wl_listener *listener, void *data)
{
    wlr_xwayland_surface *xwayland_surface = static_cast<wlr_xwayland_surface *>(data);

    Workspace *workspace = Server::getInstance().outputManager.getActiveWorkspace();
    if (xwayland_surface->surface == nullptr) {
      std::cout << "BITE" << std::endl;
	  return ;
	}
    auto view = new XdgView(xwayland_surface->surface, workspace);
    
    std::cout << "New xwayland cient: " << xwayland_surface->title << std::endl;
    //view->surface->data = 
    // wlr_xwayland_surface_ping(xwayland_surface);
    // auto view = new XWaylandView(xwayland_surface, workspace);
    // SET_LISTENER(XdgView, ViewListeners, map, xdg_surface_map<SurfaceType::xwayland>);
    //view->map.notify = [](wl_listener *listener, void *data) { Server::getInstance().Wayland->xwayland_surface_map(listener, data); };
   // wl_signal_add(&xwayland_surface->events.map, &map);
    workspace->getViews().emplace_back(view);
}

void XWayland::xwayland_surface_get_geometry(wlr_xwayland_surface *surface, wlr_box *box)
{
  box->x = surface->x;
  box->y = surface->y;
  box->width = surface->width;
  box->height = surface->height;
}
