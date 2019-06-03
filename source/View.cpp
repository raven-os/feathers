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
  SET_LISTENER(View, ViewListeners, request_fullscreen, xdg_toplevel_request_fullscreen);
  SET_LISTENER(View, ViewListeners, new_popup, xdg_handle_new_popup);
  wl_signal_add(&xdg_surface->events.new_popup, &new_popup);
}

View::~View()
{
  wl_list_remove(&map.link);
  wl_list_remove(&unmap.link);
  wl_list_remove(&destroy.link);
  if (wl_list_empty(&request_move.link) > 0)
    wl_list_remove(&request_move.link);
  if (wl_list_empty(&request_resize.link) > 0)
    wl_list_remove(&request_resize.link);
  wl_list_remove(&new_popup.link);
}

void View::xdg_surface_map([[maybe_unused]]struct wl_listener *listener, [[maybe_unused]]void *data)
{
  struct wlr_box box[1];

  mapped = true;
  focus_view();
  wlr_xdg_surface_v6_get_geometry(xdg_surface, box);

  previous_size = {box->width, box->height};

  auto &output(server->output.getOutput(getOutput()));
  auto &windowTree(output.getWindowTree());

  if (server->openType == OpenType::dontCare)
    {
      char const *tiling = server->configuration.get("tiling");
      // tiling is either 'on' or 'off'
      server->openType = (strcmp(tiling, "off") == 0) ? OpenType::floating : OpenType::dontCare;
    }
  if (server->openType != OpenType::floating &&
      (server->views.size() == 1 || server->views[1]->windowNode == wm::nullNode)) // node: we are at least ourselves in the tree
    {
      auto rootNode(windowTree.getRootIndex());
      auto &rootNodeData(windowTree.getData(rootNode));

      windowNode = rootNodeData.getContainer().addChild(rootNode, windowTree, wm::ClientData{this});
    }
  else
    {
      switch (server->openType)
	{
	case OpenType::dontCare:
	  {
	    auto prevNode(server->views[1]->windowNode);
	    auto parentNode(windowTree.getParent(prevNode));
	    auto &parentNodeData(windowTree.getData(parentNode));

	    windowNode = parentNodeData.getContainer().addChild(parentNode, windowTree, prevNode, wm::ClientData{this});
	  }
	  break;
	case OpenType::below:
	case OpenType::right:
	  {
	    auto prevNode(server->views[1]->windowNode);
	    auto &prevData(windowTree.getData(prevNode));
	    auto position(prevData.getPosition());
	    auto size(prevData.getSize());

	    prevData.data.emplace<wm::Container>(wm::Rect{position, {FixedPoint<0, uint32_t>(size[0]), FixedPoint<0, uint32_t>(size[1])}});

	    auto &container(prevData.getContainer());

	    container.direction = (server->openType == OpenType::below) ? wm::Container::verticalTiling : wm::Container::horizontalTiling;
	    server->views[1]->windowNode = container.addChild(prevNode, windowTree, wm::ClientData{server->views[1].get()});
	    windowNode = container.addChild(prevNode, windowTree, server->views[1]->windowNode, wm::ClientData{this});
	    server->openType = OpenType::dontCare;
	  }
	  break;
	case OpenType::floating:
	  {
	    windowNode = wm::nullNode;
	    server->openType = OpenType::dontCare;
	  }
	  break;
	}
    }
}

void View::xdg_surface_unmap([[maybe_unused]]struct wl_listener *listener, [[maybe_unused]]void *data)
{
  auto &output(server->output.getOutput(getOutput()));

  mapped = false;

  if (output.getFullscreenView() == this)
    xdg_toplevel_request_fullscreen(nullptr, nullptr);
  if (windowNode == wm::nullNode)
    return;

  auto &windowTree(output.getWindowTree());
  auto parentNode(windowTree.getParent(windowNode));
  auto &parentNodeData(windowTree.getData(parentNode));

  parentNodeData.getContainer().removeChild(parentNode, windowTree, windowNode);
};

void View::xdg_toplevel_request_move([[maybe_unused]]struct wl_listener *listener, [[maybe_unused]]void *data)
{
  if (windowNode == wm::nullNode)
    begin_interactive(CursorMode::CURSOR_MOVE, 0);
};

void View::xdg_toplevel_request_fullscreen([[maybe_unused]]struct wl_listener *listener, [[maybe_unused]]void *data)
{
  if (server->views.size() >= 1)
    {
      auto &output = server->output.getOutput(getOutput());

      if (!output.getFullscreenView())
	{
	  wlr_xdg_surface_v6_get_geometry(xdg_surface, &output.saved);
	  struct wlr_box *outputBox = wlr_output_layout_get_box(server->output.getLayout(), getOutput());
	  wlr_xdg_toplevel_v6_set_size(xdg_surface, outputBox->width, outputBox->height);
	  wlr_xdg_toplevel_v6_set_fullscreen(xdg_surface, true);
	  output.setFullscreenView(this);
	  fullscreen = true;
	}
      else
	{
	  wlr_xdg_toplevel_v6_set_fullscreen(xdg_surface, false);
	  wlr_xdg_toplevel_v6_set_size(xdg_surface, output.saved.width, output.saved.height);
	  output.setFullscreenView(nullptr);
	  fullscreen = false;
	}
    }
}

void View::xdg_handle_new_popup([[maybe_unused]]struct wl_listener *listener, [[maybe_unused]]void *data)
{
  struct wlr_xdg_popup_v6 *xdg_popup = static_cast<struct wlr_xdg_popup_v6 *>(data);
  popup = std::make_unique<Popup>(Popup(server, this, xdg_popup->base));
}

void View::xdg_toplevel_request_resize([[maybe_unused]]struct wl_listener *listener, [[maybe_unused]]void *data)
{
  struct wlr_xdg_toplevel_v6_resize_event *event = static_cast<struct wlr_xdg_toplevel_v6_resize_event *>(data);
  begin_interactive(CursorMode::CURSOR_RESIZE, event->edges);
};

void View::close()
{
  wlr_xdg_surface_v6_send_close(xdg_surface);
}

struct wlr_output *View::getOutput()
{
  struct wlr_box viewBox;
  wlr_xdg_surface_v6_get_geometry(xdg_surface, &viewBox);

  double outputX;
  double outputY;
  wlr_output_layout_closest_point(server->output.getLayout(), nullptr,
				  x.getDoubleValue() + (double)viewBox.width/2,
				  y.getDoubleValue() + (double)viewBox.height/2,
				  &outputX, &outputY);
  return wlr_output_layout_output_at(server->output.getLayout(), outputX, outputY);
}

void View::focus_view()
{
  struct wlr_surface *surface = xdg_surface->surface;
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
			 [this](auto const &ptr)
			 {
			   return ptr.get() == this;
			 }));
    std::unique_ptr<View> ptr(std::move(*it));

    std::move_backward(server->views.begin(), it, it + 1);
    server->views.front() = std::move(ptr);
  }
  wlr_xdg_toplevel_v6_set_activated(xdg_surface, true);
  wlr_seat_keyboard_notify_enter(seat, xdg_surface->surface, keyboard->keycodes,
				 keyboard->num_keycodes, &keyboard->modifiers);
}

void View::begin_interactive(CursorMode mode, uint32_t edges)
{
  struct wlr_surface *focused_surface = server->seat.getSeat()->pointer_state.focused_surface;
  if (xdg_surface->surface != focused_surface)
    {
      return;
    }
  server->grabbed_view = this;
  server->cursor.cursor_mode = mode;
  struct wlr_box geo_box;
  wlr_xdg_surface_v6_get_geometry(xdg_surface, &geo_box);
  if (mode == CursorMode::CURSOR_MOVE)
    {
      server->grab_x = server->cursor.cursor->x - x.getDoubleValue();
      server->grab_y = server->cursor.cursor->y - y.getDoubleValue();
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

bool View::at(double lx, double ly, struct wlr_surface **surface, double *sx, double *sy)
{
  double view_sx = lx - x.getDoubleValue();
  double view_sy = ly - y.getDoubleValue();

  struct wlr_surface_state *state = &xdg_surface->surface->current;

  double _sx, _sy;
  struct wlr_surface *_surface = nullptr;
  _surface = wlr_xdg_surface_v6_surface_at(xdg_surface, view_sx, view_sy, &_sx, &_sy);

  if (_surface )
    {
      *sx = _sx;
      *sy = _sy;
      *surface = _surface;
      return true;
    }

  return false;
}

View *View::desktop_view_at(Server *server, double lx, double ly,
			    struct wlr_surface **surface, double *sx, double *sy)
{
  for (auto &view : server->views)
    {
      if (view->at(lx, ly, surface, sx, sy))
	{
	  return view.get();
	}
    }
  return nullptr;
}
