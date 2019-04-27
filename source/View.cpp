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

  SET_LISTENER(View, ViewListeners, map, xdg_surface_map);
  wl_signal_add(&xdg_surface->events.map, &map);
  SET_LISTENER(View, ViewListeners, unmap, xdg_surface_unmap);
  wl_signal_add(&xdg_surface->events.unmap, &unmap);
  SET_LISTENER(View, ViewListeners, destroy, server->xdgShell->xdg_surface_destroy);
  wl_signal_add(&xdg_surface->events.destroy, &destroy);
  SET_LISTENER(View, ViewListeners, request_move, xdg_toplevel_request_move);
  wl_signal_add(&toplevel->events.request_move, &request_move);
  SET_LISTENER(View, ViewListeners, request_resize, xdg_toplevel_request_resize);
  wl_signal_add(&toplevel->events.request_resize, &request_resize);
}

View::~View()
{
  wl_list_remove(&map.link);
  wl_list_remove(&unmap.link);
  wl_list_remove(&destroy.link);
  wl_list_remove(&request_move.link);
  wl_list_remove(&request_resize.link);
}

void View::xdg_surface_map([[maybe_unused]]struct wl_listener *listener, [[maybe_unused]]void *data)
{
  mapped = true;

  ServerView::focus_view(this, xdg_surface->surface);

  auto rootNode(server->windowTree.getRootIndex());
  auto &rootNodeData(server->windowTree.getData(rootNode));

  windowNode = std::get<wm::Container>(rootNodeData.data).addChild(rootNode, server->windowTree, wm::ClientData{this});
};

void View::xdg_surface_unmap([[maybe_unused]]struct wl_listener *listener, [[maybe_unused]]void *data)
{
  mapped = false;

  auto rootNode(server->windowTree.getRootIndex());
  auto &rootNodeData(server->windowTree.getData(rootNode));

  std::get<wm::Container>(rootNodeData.data).removeChild(rootNode, server->windowTree, windowNode);
};

void View::xdg_toplevel_request_move([[maybe_unused]]struct wl_listener *listener, [[maybe_unused]]void *data)
{
  ServerView::begin_interactive(this, CursorMode::CURSOR_MOVE, 0);
};

void View::xdg_toplevel_request_resize([[maybe_unused]]struct wl_listener *listener, [[maybe_unused]]void *data)
{
  struct wlr_xdg_toplevel_v6_resize_event *event = static_cast<struct wlr_xdg_toplevel_v6_resize_event *>(data);
  ServerView::begin_interactive(this, CursorMode::CURSOR_RESIZE, event->edges);
};

void View::close() {
  wlr_xdg_surface_v6_send_close(xdg_surface);
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
    struct wlr_seat *seat = server->seat.getSeat();
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

    {
      auto it(std::find_if(server->views.begin(), server->views.end(),
			   [view](auto const &ptr)
			   {
			     return ptr.get() == view;
			   }));
      std::unique_ptr<View> ptr(std::move(*it));

      std::move_backward(server->views.begin(), it, it + 1);
      server->views.front() = std::move(ptr);
    }
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
    for (auto &view : server->views)
      {
	if (view_at(view.get(), lx, ly, surface, sx, sy))
	  {
	    return view.get();
	  }
      }
    return NULL;
  }

  void begin_interactive(View *view, CursorMode mode, uint32_t edges)
  {
    Server *server = view->server;
    struct wlr_surface *focused_surface = server->seat.getSeat()->pointer_state.focused_surface;
    if (view->xdg_surface->surface != focused_surface)
      {
	return;
      }
    server->grabbed_view = view;
    server->cursor.cursor_mode = mode;
    struct wlr_box geo_box;
    wlr_xdg_surface_v6_get_geometry(view->xdg_surface, &geo_box);
    if (mode == CursorMode::CURSOR_MOVE)
      {
	server->grab_x = server->cursor.cursor->x - view->x;
	server->grab_y = server->cursor.cursor->y - view->y;
      }
    else
      {
	server->grab_x = server->cursor.cursor->x + geo_box.x;
	server->grab_y = server->cursor.cursor->y + geo_box.y;
      }
    server->grab_width = geo_box.width;
    server->grab_height = geo_box.height;
    server->resize_edges = edges;
  }
}
