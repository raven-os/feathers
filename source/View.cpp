#include <cassert>
#include "View.hpp"
#include "Server.hpp"

View::View(struct wlr_surface *surface) :
  surface(surface),
  mapped(false),
  x(0),
  y(0)
{
  if (wlr_surface_is_xdg_surface_v6(surface))
    {
      struct wlr_xdg_surface_v6 *xdg_surface = wlr_xdg_surface_v6_from_wlr_surface(surface);
      struct wlr_xdg_toplevel_v6 *toplevel = xdg_surface->toplevel;

      SET_LISTENER(View, ViewListeners, map, xdg_surface_map<SurfaceType::xdg_v6>);
      wl_signal_add(&xdg_surface->events.map, &map);
      SET_LISTENER(View, ViewListeners, unmap, xdg_surface_unmap<SurfaceType::xdg_v6>);
      wl_signal_add(&xdg_surface->events.unmap, &unmap);
      // external function (not class member) so manual assignement necessary
      destroy.notify = [](wl_listener *listener, void *data) { Server::getInstance().xdgShellV6->xdg_surface_destroy(listener, data); };
      wl_signal_add(&xdg_surface->events.destroy, &destroy);
      SET_LISTENER(View, ViewListeners, request_move, xdg_toplevel_request_move<SurfaceType::xdg_v6>);
      wl_signal_add(&toplevel->events.request_move, &request_move);
      SET_LISTENER(View, ViewListeners, request_resize, xdg_toplevel_request_resize<SurfaceType::xdg_v6>);
      wl_signal_add(&toplevel->events.request_resize, &request_resize);
      SET_LISTENER(View, ViewListeners, request_fullscreen, xdg_toplevel_request_fullscreen<SurfaceType::xdg_v6>);
      SET_LISTENER(View, ViewListeners, new_popup, xdg_handle_new_popup<SurfaceType::xdg_v6>);
      wl_signal_add(&xdg_surface->events.new_popup, &new_popup);
    }
  else if (wlr_surface_is_xdg_surface(surface))
    {
      struct wlr_xdg_surface *xdg_surface = wlr_xdg_surface_from_wlr_surface(surface);
      struct wlr_xdg_toplevel *toplevel = xdg_surface->toplevel;

      SET_LISTENER(View, ViewListeners, map, xdg_surface_map<SurfaceType::xdg>);
      wl_signal_add(&xdg_surface->events.map, &map);
      SET_LISTENER(View, ViewListeners, unmap, xdg_surface_unmap<SurfaceType::xdg>);
      wl_signal_add(&xdg_surface->events.unmap, &unmap);
      // external function (not class member) so manual assignement necessary
      destroy.notify = [](wl_listener *listener, void *data) { Server::getInstance().xdgShell->xdg_surface_destroy(listener, data); };
      wl_signal_add(&xdg_surface->events.destroy, &destroy);
      SET_LISTENER(View, ViewListeners, request_move, xdg_toplevel_request_move<SurfaceType::xdg>);
      wl_signal_add(&toplevel->events.request_move, &request_move);
      SET_LISTENER(View, ViewListeners, request_resize, xdg_toplevel_request_resize<SurfaceType::xdg>);
      wl_signal_add(&toplevel->events.request_resize, &request_resize);
      SET_LISTENER(View, ViewListeners, request_fullscreen, xdg_toplevel_request_fullscreen<SurfaceType::xdg>);
      SET_LISTENER(View, ViewListeners, new_popup, xdg_handle_new_popup<SurfaceType::xdg>);
      wl_signal_add(&xdg_surface->events.new_popup, &new_popup);
    }
  else
    {
      assert(!"Unknown surface type");
    }
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

template<SurfaceType surfaceType>
void View::xdg_surface_map([[maybe_unused]]struct wl_listener *listener, [[maybe_unused]]void *data)
{
  Server &server = Server::getInstance();
  struct wlr_box box[1];

  mapped = true;
  focus_view();
  if constexpr (surfaceType == SurfaceType::xdg_v6)
    wlr_xdg_surface_v6_get_geometry(wlr_xdg_surface_v6_from_wlr_surface(surface), box);
  else if constexpr (surfaceType == SurfaceType::xdg)
    wlr_xdg_surface_get_geometry(wlr_xdg_surface_from_wlr_surface(surface), box);
  previous_size = {box->width, box->height};

  auto &windowTree(server.getActiveWindowTree());

  if (server.openType == OpenType::dontCare)
    {
      char const *tiling = server.configuration.get("tiling");
      // tiling is either 'on' or 'off'
      server.openType = (strcmp(tiling, "off") == 0) ? OpenType::floating : OpenType::dontCare;
    }
  if (server.openType != OpenType::floating &&
      (server.getViews().size() == 1 || server.getViews()[1]->windowNode == wm::nullNode)) // node: we are at least ourselves in the tree
    {
      auto rootNode(windowTree.getRootIndex());
      auto &rootNodeData(windowTree.getData(rootNode));

      windowNode = rootNodeData.getContainer().addChild(rootNode, windowTree, wm::ClientData{this});
    }
  else
    {
      switch (server.openType)
	{
	case OpenType::dontCare:
	  {
	    auto prevNode(server.getViews()[1]->windowNode);
	    auto parentNode(windowTree.getParent(prevNode));
	    auto &parentNodeData(windowTree.getData(parentNode));

	    windowNode = parentNodeData.getContainer().addChild(parentNode, windowTree, prevNode, wm::ClientData{this});
	  }
	  break;
	case OpenType::below:
	case OpenType::right:
	  {
	    auto prevNode(server.getViews()[1]->windowNode);
	    auto &prevData(windowTree.getData(prevNode));
	    auto position(prevData.getPosition());
	    auto size(prevData.getSize());

	    prevData.data.emplace<wm::Container>(wm::Rect{position, {FixedPoint<0, uint32_t>(size[0]), FixedPoint<0, uint32_t>(size[1])}});

	    auto &container(prevData.getContainer());

	    container.direction = (server.openType == OpenType::below) ? wm::Container::verticalTiling : wm::Container::horizontalTiling;
	    server.getViews()[1]->windowNode = container.addChild(prevNode, windowTree, wm::ClientData{server.getViews()[1].get()});
	    windowNode = container.addChild(prevNode, windowTree, server.getViews()[1]->windowNode, wm::ClientData{this});
	    server.openType = OpenType::dontCare;
	  }
	  break;
	case OpenType::floating:
	  {
	    windowNode = wm::nullNode;
	    server.openType = OpenType::dontCare;
	  }
	  break;
  default:
    assert("Unexpected openType value.");
    break;
	}
    }
}

template<SurfaceType surfaceType>
void View::xdg_surface_unmap([[maybe_unused]]struct wl_listener *listener, [[maybe_unused]]void *data)
{
  auto &output(Server::getInstance().outputManager.getOutput(getWlrOutput()));

  mapped = false;

  if (output.getFullscreenView() == this)
    {
      xdg_toplevel_request_fullscreen<surfaceType>(nullptr, nullptr);
    }
  if (windowNode == wm::nullNode)
    return;

  auto &windowTree(Server::getInstance().outputManager.getActiveWorkspace()->getWindowTree());
  auto parentNode(windowTree.getParent(windowNode));
  auto &parentNodeData(windowTree.getData(parentNode));

  parentNodeData.getContainer().removeChild(parentNode, windowTree, windowNode);
};

template<SurfaceType surfaceType>
void View::xdg_toplevel_request_move([[maybe_unused]]struct wl_listener *listener, [[maybe_unused]]void *data)
{
  if (windowNode == wm::nullNode)
    begin_interactive(CursorMode::CURSOR_MOVE, 0);
};

template<SurfaceType surfaceType>
void View::xdg_toplevel_request_fullscreen([[maybe_unused]]struct wl_listener *listener, [[maybe_unused]]void *data)
{
  Server &server = Server::getInstance();
  if (server.getViews().size() >= 1)
    {
      auto &output = server.outputManager.getOutput(getWlrOutput());

      if (!output.getFullscreenView())
	{
	  struct wlr_box *outputBox = wlr_output_layout_get_box(server.outputManager.getLayout(), getWlrOutput());

	  if constexpr (surfaceType == SurfaceType::xdg_v6)
	    {
	      struct wlr_xdg_surface_v6 *xdg_surface = wlr_xdg_surface_v6_from_wlr_surface(surface);

	      wlr_xdg_surface_v6_get_geometry(xdg_surface, &output.saved);
	      wlr_xdg_toplevel_v6_set_size(xdg_surface, outputBox->width, outputBox->height);
	      wlr_xdg_toplevel_v6_set_fullscreen(xdg_surface, true);
	    }
	  else if constexpr (surfaceType == SurfaceType::xdg)
	    {
	      struct wlr_xdg_surface *xdg_surface = wlr_xdg_surface_from_wlr_surface(surface);

	      wlr_xdg_surface_get_geometry(xdg_surface, &output.saved);
	      wlr_xdg_toplevel_set_size(xdg_surface, outputBox->width, outputBox->height);
	      wlr_xdg_toplevel_set_fullscreen(xdg_surface, true);
	    }

	  output.setFullscreenView(this);
	  fullscreen = true;
	}
      else
	{
	  if constexpr (surfaceType == SurfaceType::xdg_v6)
	    {
	      struct wlr_xdg_surface_v6 *xdg_surface = wlr_xdg_surface_v6_from_wlr_surface(surface);

	      wlr_xdg_toplevel_v6_set_fullscreen(xdg_surface, false);
	      wlr_xdg_toplevel_v6_set_size(xdg_surface, output.saved.width, output.saved.height);
	    }
	  else if constexpr (surfaceType == SurfaceType::xdg)
	    {
	      struct wlr_xdg_surface *xdg_surface = wlr_xdg_surface_from_wlr_surface(surface);

	      wlr_xdg_toplevel_set_fullscreen(xdg_surface, false);
	      wlr_xdg_toplevel_set_size(xdg_surface, output.saved.width, output.saved.height);
	    }
	  output.setFullscreenView(nullptr);
	  fullscreen = false;
	}
    }
}

template<SurfaceType surfaceType>
void View::xdg_handle_new_popup([[maybe_unused]]struct wl_listener *listener, [[maybe_unused]]void *data)
{
  using wlr_xdg_popup_type = std::conditional_t<surfaceType == SurfaceType::xdg, wlr_xdg_popup, wlr_xdg_popup_v6>;

  wlr_xdg_popup_type *xdg_popup = static_cast<wlr_xdg_popup_type *>(data);
  popup = std::make_unique<Popup>(Popup(this, xdg_popup->base->surface));
}

template<SurfaceType surfaceType>
void View::xdg_toplevel_request_resize([[maybe_unused]]struct wl_listener *listener, [[maybe_unused]]void *data)
{
  struct wlr_xdg_toplevel_v6_resize_event *event = static_cast<struct wlr_xdg_toplevel_v6_resize_event *>(data);
  begin_interactive(CursorMode::CURSOR_RESIZE, event->edges);
};

void View::requestFullscreen()
{
  if (wlr_surface_is_xdg_surface_v6(surface))
    xdg_toplevel_request_resize<SurfaceType::xdg_v6>(nullptr, nullptr);
  else if (wlr_surface_is_xdg_surface(surface))
    xdg_toplevel_request_resize<SurfaceType::xdg>(nullptr, nullptr);
}

void View::close()
{
  if (wlr_surface_is_xdg_surface_v6(surface))
    wlr_xdg_surface_v6_send_close(wlr_xdg_surface_v6_from_wlr_surface(surface));
  else if (wlr_surface_is_xdg_surface(surface))
    {
      wlr_xdg_toplevel_send_close(wlr_xdg_surface_from_wlr_surface(surface));
      // do nothing?
    }
}

struct wlr_output *View::getWlrOutput()
{
  Server &server = Server::getInstance();
  struct wlr_box viewBox;
  {
    if (wlr_surface_is_xdg_surface_v6(surface))
      wlr_xdg_surface_v6_get_geometry(wlr_xdg_surface_v6_from_wlr_surface(surface), &viewBox);
    else if (wlr_surface_is_xdg_surface(surface))
      wlr_xdg_surface_get_geometry(wlr_xdg_surface_from_wlr_surface(surface), &viewBox);
  }

  double outputX;
  double outputY;
  wlr_output_layout_closest_point(server.outputManager.getLayout(), nullptr,
				  x.getDoubleValue() + static_cast<double>(viewBox.width/2),
				  y.getDoubleValue() + static_cast<double>(viewBox.height/2),
				  &outputX, &outputY);
  return wlr_output_layout_output_at(server.outputManager.getLayout(), outputX, outputY);
}

void View::focus_view()
{
  Server &server = Server::getInstance();
  struct wlr_seat *seat = server.seat.getSeat();
  struct wlr_surface *prev_surface = seat->keyboard_state.focused_surface;
  if (prev_surface == surface)
    {
      return;
    }
  if (prev_surface)
    {
      if (wlr_surface_is_xdg_surface_v6(seat->keyboard_state.focused_surface))
	{
	  struct wlr_xdg_surface_v6 *previous =
	    wlr_xdg_surface_v6_from_wlr_surface(seat->keyboard_state.focused_surface);
	  wlr_xdg_toplevel_v6_set_activated(previous, false);
	}
      else if (wlr_surface_is_xdg_surface(seat->keyboard_state.focused_surface))
	{
	  struct wlr_xdg_surface *previous =
	    wlr_xdg_surface_from_wlr_surface(seat->keyboard_state.focused_surface);
	  wlr_xdg_toplevel_set_activated(previous, false);
	}
    }
  struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(seat);

  {
    auto it(std::find_if(server.getViews().begin(), server.getViews().end(),
			 [this](auto const &ptr)
			 {
			   return ptr.get() == this;
			 }));
    std::unique_ptr<View> ptr(std::move(*it));

    std::move_backward(server.getViews().begin(), it, it + 1);
    server.getViews().front() = std::move(ptr);
  }
  if (wlr_surface_is_xdg_surface_v6(surface))
    wlr_xdg_toplevel_v6_set_activated(wlr_xdg_surface_v6_from_wlr_surface(surface), true);
  else if (wlr_surface_is_xdg_surface(surface))
    wlr_xdg_toplevel_set_activated(wlr_xdg_surface_from_wlr_surface(surface), true);
  wlr_seat_keyboard_notify_enter(seat, surface, keyboard->keycodes,
				 keyboard->num_keycodes, &keyboard->modifiers);
}

void View::begin_interactive(CursorMode mode, uint32_t edges)
{
  Server &server = Server::getInstance();
  struct wlr_surface *focused_surface = server.seat.getSeat()->pointer_state.focused_surface;
  if (surface != focused_surface)
    {
      return;
    }
  server.grabbed_view = this;
  server.cursor.cursor_mode = mode;
  struct wlr_box geo_box;

  if (wlr_surface_is_xdg_surface_v6(surface))
    wlr_xdg_surface_v6_get_geometry(wlr_xdg_surface_v6_from_wlr_surface(surface), &geo_box);
  else if (wlr_surface_is_xdg_surface(surface))
    wlr_xdg_surface_get_geometry(wlr_xdg_surface_from_wlr_surface(surface), &geo_box);

  if (mode == CursorMode::CURSOR_MOVE)
    {
      server.grab_x = server.cursor.cursor->x - x.getDoubleValue();
      server.grab_y = server.cursor.cursor->y - y.getDoubleValue();
    }
  else
    {
      server.grab_x = server.cursor.cursor->x + geo_box.x;
      server.grab_y = server.cursor.cursor->y + geo_box.y;
    }
  server.grab_width = geo_box.width;
  server.grab_height = geo_box.height;
  server.resize_edges = edges;
}

bool View::at(double lx, double ly, struct wlr_surface **out_surface, double *sx, double *sy)
{
  double view_sx = lx - x.getDoubleValue();
  double view_sy = ly - y.getDoubleValue();

  struct wlr_surface_state *state = &surface->current;

  double _sx, _sy;
  struct wlr_surface *_surface = nullptr;
  if (wlr_surface_is_xdg_surface_v6(surface))
    _surface = wlr_xdg_surface_v6_surface_at(wlr_xdg_surface_v6_from_wlr_surface(surface), view_sx, view_sy, &_sx, &_sy);
  else if (wlr_surface_is_xdg_surface(surface))
    _surface = wlr_xdg_surface_surface_at(wlr_xdg_surface_from_wlr_surface(surface), view_sx, view_sy, &_sx, &_sy);
  if (_surface )
    {
      *sx = _sx;
      *sy = _sy;
      *out_surface = _surface;
      return true;
    }

  return false;
}

View *View::desktop_view_at(double lx, double ly,
			    struct wlr_surface **surface, double *sx, double *sy)
{
  Server &server = Server::getInstance();
  for (auto &view : server.getViews())
    {
      if (view->at(lx, ly, surface, sx, sy))
	{
	  return view.get();
	}
    }
  return nullptr;
}
