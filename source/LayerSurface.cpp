#include <cassert>
#include "LayerSurface.hpp"
#include "Server.hpp"

LayerSurface::LayerSurface(struct wlr_surface *surface) noexcept
  : surface(surface)
  , x(0)
  , y(0)
{
  wlr_layer_surface_v1 *shell_surface = wlr_layer_surface_v1_from_wlr_surface(surface);
  
  SET_LISTENER(LayerSurface, LayerSurfaceListeners, map, shell_surface_map);
  wl_signal_add(&shell_surface->events.map, &map);
  SET_LISTENER(LayerSurface, LayerSurfaceListeners, unmap, shell_surface_unmap);
  wl_signal_add(&shell_surface->events.unmap, &unmap);
  // external function (not class member) so manual assignement necessary
  destroy.notify = [](wl_listener *listener, void *data) { Server::getInstance().layerShell.shell_surface_destroy(listener, data); };
  wl_signal_add(&shell_surface->events.destroy, &destroy);
  SET_LISTENER(LayerSurface, LayerSurfaceListeners, new_popup, shell_surface_new_popup);
  wl_signal_add(&shell_surface->events.new_popup, &new_popup);
  // SET_LISTENER(LayerSurface, LayerSurfaceListeners, request_move, xdg_toplevel_request_move<SurfaceType::xdg>);
  // wl_signal_add(&toplevel->events.request_move, &request_move);
  // SET_LISTENER(LayerSurface, LayerSurfaceListeners, request_resize, xdg_toplevel_request_resize<SurfaceType::xdg>);
  // wl_signal_add(&toplevel->events.request_resize, &request_resize);
  // SET_LISTENER(LayerSurface, LayerSurfaceListeners, request_fullscreen, xdg_toplevel_request_fullscreen<SurfaceType::xdg>);
  // SET_LISTENER(LayerSurface, LayerSurfaceListeners, new_popup, xdg_handle_new_popup<SurfaceType::xdg>);
  // wl_signal_add(&shell_surface->events.new_popup, &new_popup);
}

LayerSurface::~LayerSurface() noexcept
{
  wl_list_remove(&map.link);
  wl_list_remove(&unmap.link);
  wl_list_remove(&destroy.link);
  wl_list_remove(&new_popup.link);
}

void LayerSurface::shell_surface_map(struct wl_listener *listenr, void *data)
{
}

void LayerSurface::shell_surface_unmap(struct wl_listener *listenr, void *data)
{
}

void LayerSurface::shell_surface_new_popup(struct wl_listener *listenr, void *data)
{
}


void LayerSurface::close()
{
  wlr_layer_surface_v1_close(wlr_layer_surface_v1_from_wlr_surface(surface));
}

// bool LayerSurface::at(double lx, double ly, struct wlr_surface **out_surface, double *sx, double *sy)
// {
//   double view_sx = lx - x.getDoubleValue();
//   double view_sy = ly - y.getDoubleValue();

//   struct wlr_surface_state *state = &surface->current;

//   double _sx, _sy;
//   struct wlr_surface *_surface = nullptr;
//   if (wlr_surface_is_xdg_surface_v6(surface))
//     _surface = wlr_xdg_surface_v6_surface_at(wlr_xdg_surface_v6_from_wlr_surface(surface), view_sx, view_sy, &_sx, &_sy);
//   else if (wlr_surface_is_xdg_surface(surface))
//     _surface = wlr_xdg_surface_surface_at(wlr_xdg_surface_from_wlr_surface(surface), view_sx, view_sy, &_sx, &_sy);
//   if (_surface )
//     {
//       *sx = _sx;
//       *sy = _sy;
//       *out_surface = _surface;
//       return true;
//     }

//   return false;
// }

// LayerSurface *LayerSurface::desktop_view_at(double lx, double ly,
// 			    struct wlr_surface **surface, double *sx, double *sy)
// {
//   Server &server = Server::getInstance();
//   for (auto &view : server.getLayerSurfaces())
//     {
//       if (view->at(lx, ly, surface, sx, sy))
// 	{
// 	  return view.get();
// 	}
//     }
//   return nullptr;
// }
