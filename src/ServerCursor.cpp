#include "ServerCursor.hpp"
#include "Server.hpp"

namespace
{
  void process_cursor_move(Server *server, uint32_t time)
  {
    server->grabbed_view->x = server->cursor->x - server->grab_x;
    server->grabbed_view->y = server->cursor->y - server->grab_y;
  }

  void process_cursor_resize(Server *server, uint32_t time)
  {
    View *view = server->grabbed_view;
    double dx = server->cursor->x - server->grab_x;
    double dy = server->cursor->y - server->grab_y;
    double x = view->x;
    double y = view->y;
    int width = server->grab_width;
    int height = server->grab_height;
    if (server->resize_edges & WLR_EDGE_TOP)
      {
	y = server->grab_y + dy;
	height -= dy;
	if (height < 1)
	  {
	    y += height;
	  }
      }
    else if (server->resize_edges & WLR_EDGE_BOTTOM)
      {
	height += dy;
      }
    if (server->resize_edges & WLR_EDGE_LEFT)
      {
	x = server->grab_x + dx;
	width -= dx;
	if (width < 1)
	  {
	    x += width;
	  }
      }
    else if (server->resize_edges & WLR_EDGE_RIGHT)
      {
	width += dx;
      }
    view->x = x;
    view->y = y;
    wlr_xdg_toplevel_set_size(view->xdg_surface, width, height);
  }

  void process_cursor_motion(Server *server, uint32_t time)
  {
    if (server->cursor_mode == CursorMode::CURSOR_MOVE)
      {
	process_cursor_move(server, time);
	return;
      }
    else if (server->cursor_mode == CursorMode::CURSOR_RESIZE)
      {
	process_cursor_resize(server, time);
	return;
      }

    double sx, sy;
    struct wlr_seat *seat = server->seat;
    struct wlr_surface *surface = NULL;
    View *view = ServerView::desktop_view_at(server,server->cursor->x,
					     server->cursor->y, &surface, &sx, &sy);
    if (!view)
      {
	wlr_xcursor_manager_set_cursor_image(server->cursor_mgr, "left_ptr", server->cursor);
      }
    if (surface)
      {
	bool focus_changed = seat->pointer_state.focused_surface != surface;
	wlr_seat_pointer_notify_enter(seat, surface, sx, sy);
	if (!focus_changed)
	  {
	    wlr_seat_pointer_notify_motion(seat, time, sx, sy);
	  }
      }
    else
      {
	wlr_seat_pointer_clear_focus(seat);
      }
  }
}

namespace ServerCursor
{
  void server_cursor_motion(struct wl_listener *listener, void *data)
  {
    Server *server = wl_container_of(listener, server, cursor_motion);
    struct wlr_event_pointer_motion *event = static_cast<struct wlr_event_pointer_motion *>(data);
    wlr_cursor_move(server->cursor, event->device, event->delta_x, event->delta_y);
    process_cursor_motion(server, event->time_msec);
  }

  void server_cursor_motion_absolute(struct wl_listener *listener, void *data)
  {
    Server *server = wl_container_of(listener, server, cursor_motion_absolute);
    struct wlr_event_pointer_motion_absolute *event = static_cast<struct wlr_event_pointer_motion_absolute *>(data);
    wlr_cursor_warp_absolute(server->cursor, event->device, event->x, event->y);
    process_cursor_motion(server, event->time_msec);
  }

  void server_cursor_button(struct wl_listener *listener, void *data)
  {
    Server *server = wl_container_of(listener, server, cursor_button);
    struct wlr_event_pointer_button *event = static_cast<struct wlr_event_pointer_button *>(data);
    wlr_seat_pointer_notify_button(server->seat, event->time_msec, event->button, event->state);
    double sx, sy;
    struct wlr_seat *seat = server->seat;
    struct wlr_surface *surface;
    View *view = ServerView::desktop_view_at(server, server->cursor->x, server->cursor->y, &surface, &sx, &sy);
    if (event->state == WLR_BUTTON_RELEASED)
      {
	server->cursor_mode = CursorMode::CURSOR_PASSTHROUGH;
      }
    else
      {
	ServerView::focus_view(view, surface);
      }
  }

  void server_cursor_axis(struct wl_listener *listener, void *data)
  {
    Server *server = wl_container_of(listener, server, cursor_axis);
    struct wlr_event_pointer_axis *event = static_cast<struct wlr_event_pointer_axis *>(data);
    wlr_seat_pointer_notify_axis(server->seat,
				 event->time_msec, event->orientation, event->delta,
				 event->delta_discrete, event->source);
  }
}
