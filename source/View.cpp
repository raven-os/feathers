#include <cassert>

#include "View.hpp"
#include "Server.hpp"
#include "Output.hpp"
#include "XdgView.hpp"

View::View(wlr_surface *surface) noexcept :
  surface(surface),
  mapped(false),
  x(0),
  y(0)
{
}

View::~View() noexcept = default;

bool View::at(double lx, double ly, wlr_surface **out_surface, double *sx, double *sy)
{
  double view_sx = lx - x.getDoubleValue();
  double view_sy = ly - y.getDoubleValue();

  double _sx, _sy;
  wlr_surface *_surface = nullptr;
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
			    wlr_surface **surface, double *sx, double *sy)
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

template<SurfaceType surfaceType>
void View::xdg_handle_new_popup(wl_listener *listener, void *data)
{
  using wlr_xdg_popup_type = std::conditional_t<surfaceType == SurfaceType::xdg, wlr_xdg_popup, wlr_xdg_popup_v6>;

  wlr_xdg_popup_type *xdg_popup = static_cast<wlr_xdg_popup_type *>(data);
  popup = std::make_unique<Popup>(xdg_popup->base->surface);
}

template
void View::xdg_handle_new_popup<SurfaceType::xdg>(wl_listener *listener, void *data);

template
void View::xdg_handle_new_popup<SurfaceType::xdg_v6>(wl_listener *listener, void *data);


void View::focus_view()
{
  Server &server = Server::getInstance();
  wlr_seat *seat = server.seat.getSeat();
  wlr_keyboard *keyboard = wlr_seat_get_keyboard(seat);
  wlr_surface *prev_surface = seat->keyboard_state.focused_surface;
  if (prev_surface == surface)
    return;

  server.seat.unfocusPrevious();
  {
    auto it(std::find_if(server.getViews().begin(), server.getViews().end(),
			 [this](auto const &ptr) noexcept
			 {
			   return ptr.get() == this;
			 }));
    std::unique_ptr<XdgView> ptr(std::move(*it));

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
