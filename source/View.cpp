#include "View.hpp"
#include "Server.hpp"

View::View(Server *server, struct wlr_xdg_surface_v6 *xdg_surface) :
  server(server),
  xdg_surface(xdg_surface),
  mapped(false),
  x(0),
  y(0)
{
  struct wlr_xdg_toplevel_v6 *toplevel = xdg_surface->toplevel;

  SET_LISTENER(View, ViewListeners, map, server->xdgShell->xdg_surface_map);
  wl_signal_add(&xdg_surface->events.map, &map);
  SET_LISTENER(View, ViewListeners, unmap, server->xdgShell->xdg_surface_unmap);
  wl_signal_add(&xdg_surface->events.unmap, &unmap);
  SET_LISTENER(View, ViewListeners, destroy, server->xdgShell->xdg_surface_destroy);
  wl_signal_add(&xdg_surface->events.destroy, &destroy);
  SET_LISTENER(View, ViewListeners, request_move, server->xdgShell->xdg_toplevel_request_move);
  wl_signal_add(&toplevel->events.request_move, &request_move);
  SET_LISTENER(View, ViewListeners, request_resize, server->xdgShell->xdg_toplevel_request_resize);
  wl_signal_add(&toplevel->events.request_resize, &request_resize);
}

namespace ServerView
{
  void focus_view(View *view, struct wlr_surface *surface)
  {
    if (!view)
      {
	return;
      }
    Server *server = view->server;
    struct wlr_seat *seat = server->seat->getSeat();
    struct wlr_surface *prev_surface = seat->keyboard_state.focused_surface;
    if (prev_surface == surface)
      {
	return;
      }
    if (prev_surface)
      {
	struct wlr_xdg_surface_v6 *previous =
	  wlr_xdg_surface_v6_from_wlr_surface(seat->keyboard_state.focused_surface);
	wlr_xdg_toplevel_v6_set_activated(previous, false);
      }
    struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(seat);
    wl_list_remove(&view->link);
    wl_list_insert(&server->views, &view->link);
    wlr_xdg_toplevel_v6_set_activated(view->xdg_surface, true);
    wlr_seat_keyboard_notify_enter(seat, view->xdg_surface->surface, keyboard->keycodes,
				   keyboard->num_keycodes, &keyboard->modifiers);
  }

  bool view_at(View *view, double lx, double ly, struct wlr_surface **surface,
	       double *sx, double *sy)
  {
    double view_sx = lx - view->x;
    double view_sy = ly - view->y;

    struct wlr_surface_state *state = &view->xdg_surface->surface->current;

    double _sx, _sy;
    struct wlr_surface *_surface = NULL;
    _surface = wlr_xdg_surface_v6_surface_at(view->xdg_surface, view_sx, view_sy, &_sx, &_sy);

    if (_surface )
      {
	*sx = _sx;
	*sy = _sy;
	*surface = _surface;
	return true;
      }

    return false;
  }

  View *desktop_view_at(Server *server, double lx, double ly,
			struct wlr_surface **surface, double *sx, double *sy)
  {
    View *view;
    wl_list_for_each(view, &server->views, link)
      {
	if (view_at(view, lx, ly, surface, sx, sy))
	  {
	    return view;
	  }
      }
    return NULL;
  }

  void begin_interactive(View *view, CursorMode mode, uint32_t edges)
  {
    Server *server = view->server;
    struct wlr_surface *focused_surface = server->seat->getSeat()->pointer_state.focused_surface;
    if (view->xdg_surface->surface != focused_surface)
      {
	return;
      }
    server->grabbed_view = view;
    server->cursor->cursor_mode = mode;
    struct wlr_box geo_box;
    wlr_xdg_surface_v6_get_geometry(view->xdg_surface, &geo_box);
    if (mode == CursorMode::CURSOR_MOVE)
      {
	server->grab_x = server->cursor->cursor->x - view->x;
	server->grab_y = server->cursor->cursor->y - view->y;
      }
    else
      {
	server->grab_x = server->cursor->cursor->x + geo_box.x;
	server->grab_y = server->cursor->cursor->y + geo_box.y;
      }
    server->grab_width = geo_box.width;
    server->grab_height = geo_box.height;
    server->resize_edges = edges;
  }
}
