#include "ServerCursor.hpp"
#include "Server.hpp"
#include "XdgView.hpp"
#include "Output.hpp"

#include <cassert>
#include <stdio.h>

ServerCursor::ServerCursor()
  : cursor_mode(CursorMode::CURSOR_PASSTHROUGH)
{
  cursor = wlr_cursor_create();
  wlr_cursor_attach_output_layout(cursor, Server::getInstance().outputManager.getLayout());

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

void ServerCursor::process_cursor_move(uint32_t time)
{
  Server &server = Server::getInstance();
  server.grabbed_view->x = FixedPoint<-4, int>(int(double(1 << 4) * (cursor->x - server.grab_x)));
  server.grabbed_view->y = FixedPoint<-4, int>(int(double(1 << 4) * (cursor->y - server.grab_y)));
}

void ServerCursor::process_cursor_resize(uint32_t time)
{
  Server &server = Server::getInstance();
  XdgView *view = server.grabbed_view;

  if (view->windowNode == wm::nullNode)
    {
      wlr_box box[1];
      if (wlr_surface_is_xdg_surface_v6(view->surface))
	wlr_xdg_surface_v6_get_geometry(wlr_xdg_surface_v6_from_wlr_surface(view->surface), box);
      else if (wlr_surface_is_xdg_surface(view->surface))
	wlr_xdg_surface_get_geometry(wlr_xdg_surface_from_wlr_surface(view->surface), box);

      double dx = cursor->x - server.grab_x + box->x;
      double dy = cursor->y - server.grab_y + box->y;
      int width = server.grab_width;
      int height = server.grab_height;

      if (server.resize_edges & WLR_EDGE_TOP)
	{
	  double y = cursor->y;

	  height -= int(dy);
	  if (height < 1) // TODO: prevent moving the window downwards properly
	    {
	      y += double(height);
	    }
	  view->y = FixedPoint<-4, int>(int(double(1 << 4) * (y - box->y)));
	}
      else if (server.resize_edges & WLR_EDGE_BOTTOM)
	{
	  height += int(dy);
	}
      if (server.resize_edges & WLR_EDGE_LEFT)
	{
	  double x = cursor->x;

	  width -= int(dx);
	  if (width < 1) // TODO: prevent moving the window to the right properly
	    {
	      x += double(width);
	    }
	  view->x = FixedPoint<-4, int>(int(double(1 << 4) * (x - box->x)));
	}
      else if (server.resize_edges & WLR_EDGE_RIGHT)
	{
	  width += int(dx);
	}
      if (wlr_surface_is_xdg_surface(view->surface))
	wlr_xdg_toplevel_set_size(wlr_xdg_surface_from_wlr_surface(view->surface), width, height);
      else if (wlr_surface_is_xdg_surface_v6(view->surface))
	wlr_xdg_toplevel_v6_set_size(wlr_xdg_surface_v6_from_wlr_surface(view->surface), width, height);
    }
  else
    {
      wm::WindowTree &windowTree(server.getActiveWindowTree());

      for (bool direction : std::array<bool, 2u>{wm::Container::horizontalTiling, wm::Container::verticalTiling})
	{
	  // TODO: clamp cursor position to not cause negative sizes and moving windows
	  FixedPoint<-4, int32_t> cursor_pos((1 << 4) * int32_t((direction == wm::Container::horizontalTiling ? cursor->x : cursor->y)));

	  auto validataPrevAndNextNewPos([&windowTree, direction, cursor_pos](auto node)
					 {
					   auto &data(windowTree.getData(node));
					   
					   if (data.getPosition()[direction] + FixedPoint<-4, int>(FixedPoint<0, int32_t>(data.getSize()[direction]))
					       <= cursor_pos + data.getMinSize(node, windowTree)[direction])
					     return false;
					   if (node == windowTree.getFirstChild(windowTree.getParent(node)))
					     return true;
					   auto prevNode(windowTree.getPrevSibling(node));
					   auto &prevData(windowTree.getData(prevNode));

					   return (prevData.getPosition()[direction] + prevData.getMinSize(prevNode, windowTree)[direction] < cursor_pos);
					 });

	  for (auto node = view->windowNode; node != windowTree.getRootIndex(); node = windowTree.getParent(node))
	    {
	      auto parentNode(windowTree.getParent(node));
	      auto &parentData(windowTree.getData(parentNode).getContainer());

	      if (parentData.direction == direction)
		{
		  if ((server.resize_edges & (direction == wm::Container::horizontalTiling ? WLR_EDGE_LEFT : WLR_EDGE_TOP))
		      && windowTree.getFirstChild(parentNode) != node)
		    {
		      if (validataPrevAndNextNewPos(node))
			{
			  auto &data(windowTree.getData(node));
			  auto newPos(data.getPosition());

			  newPos[direction] = cursor_pos;
			  data.move(node, windowTree, newPos);
			  parentData.updateChildWidths(parentNode, windowTree);
			}
		    }
		  else if ((server.resize_edges & (direction == wm::Container::horizontalTiling ? WLR_EDGE_RIGHT : WLR_EDGE_BOTTOM))
		  	   && windowTree.getSibling(node) != wm::nullNode)
		    {
		      auto nextNode(windowTree.getSibling(node));

		      if (validataPrevAndNextNewPos(nextNode))
			{
			  auto &nextData(windowTree.getData(nextNode));
			  auto newPos(nextData.getPosition());

			  newPos[direction] = cursor_pos;
			  nextData.move(nextNode, windowTree, newPos);
			  parentData.updateChildWidths(parentNode, windowTree);
			}
		    }
		  else
		    {
		      continue;
		    }
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
	wlr_seat *seat = Server::getInstance().seat.getSeat();
	wlr_surface *surface = nullptr;
	View *view = View::view_at(cursor->x, cursor->y, &surface, &sx, &sy);
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
	    else if (Server::getInstance().configuration.getBool("enable enter on hover"))
	      {
		if (View *view = View::view_at(cursor->x, cursor->y, &surface, &sx, &sy))
		  view->focus_view();
	      }
	  }
	else
	  {
	    wlr_seat_pointer_clear_focus(seat);
	  }
      }
    }
}

void ServerCursor::server_cursor_motion(wl_listener *listener, void *data)
{
  wlr_event_pointer_motion *event = static_cast<wlr_event_pointer_motion *>(data);
  wlr_cursor_move(cursor, event->device, event->delta_x, event->delta_y);
  process_cursor_motion(event->time_msec);
}

void ServerCursor::server_cursor_motion_absolute(wl_listener *listener, void *data)
{
  wlr_event_pointer_motion_absolute *event = static_cast<wlr_event_pointer_motion_absolute *>(data);
  wlr_cursor_warp_absolute(cursor, event->device, event->x, event->y);
  process_cursor_motion(event->time_msec);
}

void ServerCursor::server_cursor_button(wl_listener *listener, void *data)
{
  wlr_event_pointer_button *event = static_cast<wlr_event_pointer_button *>(data);
  wlr_seat *seat = Server::getInstance().seat.getSeat();

  wlr_seat_pointer_notify_button(seat, event->time_msec, event->button, event->state);

  switch (event->state)
    {
    case WLR_BUTTON_RELEASED:
      cursor_mode = CursorMode::CURSOR_PASSTHROUGH;
      break;
    case WLR_BUTTON_PRESSED:
      {
	double sx, sy;
	wlr_surface *surface;

	if (View *view = View::view_at(cursor->x, cursor->y, &surface, &sx, &sy))
	  view->focus_view();
      }
      break;
    default:
      assert(!"Unknown WLR_BUTTON value");
    }
}

void ServerCursor::server_cursor_frame(wl_listener *, void *)
{
  wlr_seat_pointer_notify_frame(Server::getInstance().seat.getSeat());
}

void ServerCursor::server_cursor_axis(wl_listener *listener, void *data)
{
  wlr_event_pointer_axis *event = static_cast<wlr_event_pointer_axis *>(data);
  wlr_seat_pointer_notify_axis(Server::getInstance().seat.getSeat(),
			       event->time_msec, event->orientation, event->delta,
			       event->delta_discrete, event->source);
}
