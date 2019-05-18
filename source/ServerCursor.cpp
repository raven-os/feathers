#include "ServerCursor.hpp"
#include "Server.hpp"

#include <cassert>
#include <stdio.h>

ServerCursor::ServerCursor(Server *server)
  : server(server),
    cursor_mode(CursorMode::CURSOR_PASSTHROUGH)
{
  cursor = wlr_cursor_create();
  wlr_cursor_attach_output_layout(cursor, server->output.getLayout());

  cursor_mgr = wlr_xcursor_manager_create(nullptr, 24);
  wlr_xcursor_manager_load(cursor_mgr, 1);
  SET_LISTENER(ServerCursor, ServerCursorListeners, cursor_motion, server_cursor_motion);
  wl_signal_add(&cursor->events.motion, &cursor_motion);
  SET_LISTENER(ServerCursor, ServerCursorListeners, cursor_motion_absolute, server_cursor_motion_absolute);
  wl_signal_add(&cursor->events.motion_absolute, &cursor_motion_absolute);
  SET_LISTENER(ServerCursor, ServerCursorListeners, cursor_button, server_cursor_button);
  wl_signal_add(&cursor->events.button, &cursor_button);
  SET_LISTENER(ServerCursor, ServerCursorListeners, cursor_axis, server_cursor_axis);
  wl_signal_add(&cursor->events.axis, &cursor_axis);
  SET_LISTENER(ServerCursor, ServerCursorListeners, cursor_frame, server_cursor_frame);
  wl_signal_add(&cursor->events.frame, &cursor_frame);
}

void ServerCursor::process_cursor_move([[maybe_unused]]uint32_t time)
{
  server->grabbed_view->x = cursor->x - server->grab_x;
  server->grabbed_view->y = cursor->y - server->grab_y;
}

void ServerCursor::process_cursor_resize([[maybe_unused]]uint32_t time)
{
  View *view = server->grabbed_view;

  if (view->windowNode == wm::nullNode)
    {
      struct wlr_box box[1];
      wlr_xdg_surface_v6_get_geometry(view->xdg_surface, box);

      double dx = cursor->x - server->grab_x + box->x;
      double dy = cursor->y - server->grab_y + box->y;
      int width = server->grab_width;
      int height = server->grab_height;

      if (server->resize_edges & WLR_EDGE_TOP)
	{
	  double y = cursor->y;

	  height -= dy;
	  if (height < 1) // TODO: prevent moving the window downwards properly
	    {
	      y += height;
	    }
	  view->y = y - box->y;
	}
      else if (server->resize_edges & WLR_EDGE_BOTTOM)
	{
	  height += dy;
	}
      if (server->resize_edges & WLR_EDGE_LEFT)
	{
	  double x = cursor->x;

	  width -= dx;
	  if (width < 1) // TODO: prevent moving the window to the right properly
	    {
	      x += width;
	    }
	  view->x = x - box->x;
	}
      else if (server->resize_edges & WLR_EDGE_RIGHT)
	{
	  width += dx;
	}
      wlr_xdg_toplevel_v6_set_size(view->xdg_surface, width, height);
    }
  else
    {
      Output &output(server->output.getOutput(view->getOutput()));
      wm::WindowTree &windowTree(output.getWindowTree());

      for (bool direction : std::array<bool, 2u>{wm::Container::horizontalTilling, wm::Container::verticalTilling})
	{
	  // TODO: clamp cursor position to not cause negative sizes and moving windows
	  int16_t cursor_pos(direction == wm::Container::horizontalTilling ? cursor->x : cursor->y);

	  for (auto node = view->windowNode; node != windowTree.getRootIndex(); node = windowTree.getParent(node))
	    {
	      auto parentNode(windowTree.getParent(node));
	      auto &parentData(std::get<wm::Container>(windowTree.getData(parentNode).data));

	      if (parentData.direction == direction)
		{
		  if ((server->resize_edges & (direction == wm::Container::horizontalTilling ? WLR_EDGE_LEFT : WLR_EDGE_TOP))
		      && windowTree.getFirstChild(parentNode) != node)
		    {
		      auto &data(windowTree.getData(node));
		      auto newPos(data.getPosition());

		      newPos[direction] = cursor_pos;
		      data.move(node, windowTree, newPos);
		    }
		  else if ((server->resize_edges & (direction == wm::Container::horizontalTilling ? WLR_EDGE_RIGHT : WLR_EDGE_BOTTOM))
		  	   && windowTree.getSibling(node) != wm::nullNode)
		    {
		      auto nextNode(windowTree.getSibling(node));
		      auto &nextData(windowTree.getData(nextNode));
		      auto newPos(nextData.getPosition());

		      newPos[direction] = cursor_pos;
		      nextData.move(nextNode, windowTree, newPos);
		    }
		  parentData.updateChildWidths(parentNode, windowTree);
		  break;
		}
	    }
	}
    }
}

void ServerCursor::process_cursor_motion(uint32_t time)
{
  switch (cursor_mode)
    {
    case CursorMode::CURSOR_MOVE:
      process_cursor_move(time);
      break;
    case CursorMode::CURSOR_RESIZE:
      process_cursor_resize(time);
      break;
    default:
      {
	double sx, sy;
	struct wlr_seat *seat = server->seat.getSeat();
	struct wlr_surface *surface = NULL;
	View *view = View::desktop_view_at(server, cursor->x,
						 cursor->y, &surface, &sx, &sy);
	if (!view)
	  {
	    wlr_xcursor_manager_set_cursor_image(cursor_mgr, "left_ptr", cursor);
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
}

void ServerCursor::server_cursor_motion([[maybe_unused]]struct wl_listener *listener, void *data)
{
  struct wlr_event_pointer_motion *event = static_cast<struct wlr_event_pointer_motion *>(data);
  wlr_cursor_move(cursor, event->device, event->delta_x, event->delta_y);
  process_cursor_motion(event->time_msec);
}

void ServerCursor::server_cursor_motion_absolute([[maybe_unused]]struct wl_listener *listener, void *data)
{
  struct wlr_event_pointer_motion_absolute *event = static_cast<struct wlr_event_pointer_motion_absolute *>(data);
  wlr_cursor_warp_absolute(cursor, event->device, event->x, event->y);
  process_cursor_motion(event->time_msec);
}

void ServerCursor::server_cursor_button([[maybe_unused]]struct wl_listener *listener, void *data)
{
  struct wlr_event_pointer_button *event = static_cast<struct wlr_event_pointer_button *>(data);
  struct wlr_seat *seat = server->seat.getSeat();

  wlr_seat_pointer_notify_button(seat, event->time_msec, event->button, event->state);

  switch (event->state)
    {
    case WLR_BUTTON_RELEASED:
      cursor_mode = CursorMode::CURSOR_PASSTHROUGH;
      break;
    case WLR_BUTTON_PRESSED:
      {
	double sx, sy;
	struct wlr_surface *surface;
	View *view = View::desktop_view_at(server, cursor->x, cursor->y, &surface, &sx, &sy);

	view->focus_view();
      }
      break;
    default:
      assert(!"Unknown WLR_BUTTON value");
    }
}

void ServerCursor::server_cursor_frame(struct wl_listener *, void *)
{
  wlr_seat_pointer_notify_frame(server->seat.getSeat());
}

void ServerCursor::server_cursor_axis([[maybe_unused]]struct wl_listener *listener, void *data)
{
  struct wlr_event_pointer_axis *event = static_cast<struct wlr_event_pointer_axis *>(data);
  wlr_seat_pointer_notify_axis(server->seat.getSeat(),
			       event->time_msec, event->orientation, event->delta,
			       event->delta_discrete, event->source);
}
