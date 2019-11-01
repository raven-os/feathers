#include <cassert>
#include "LayerSurface.hpp"
#include "Server.hpp"
#include "Output.hpp"

LayerSurface::LayerSurface(wlr_surface *surface) noexcept
  : View(surface)
{
  wlr_layer_surface_v1 *shell_surface = wlr_layer_surface_v1_from_wlr_surface(surface);
  
  SET_LISTENER(LayerSurface, LayerSurfaceListeners, map, shell_surface_map);
  wl_signal_add(&shell_surface->events.map, &map);
  SET_LISTENER(LayerSurface, LayerSurfaceListeners, unmap, shell_surface_unmap);
  wl_signal_add(&shell_surface->events.unmap, &unmap);
  destroy.notify = [](wl_listener *listener, void *data) { Server::getInstance().layerShell.shell_surface_destroy(listener, data); };
  wl_signal_add(&shell_surface->events.destroy, &destroy);
  SET_LISTENER(LayerSurface, LayerSurfaceListeners, new_popup, xdg_handle_new_popup<SurfaceType::xdg>);
  wl_signal_add(&shell_surface->events.new_popup, &new_popup);
  /// According to protocol spec:
  /// "Note: the output may be NULL. In this case, it is your responsibility to assign an output before returning.
  if (shell_surface->output == nullptr)
    shell_surface->output = Server::getInstance().outputManager.getOutputs()[0]->getWlrOutput();
  wlr_layer_surface_v1_configure(shell_surface, shell_surface->client_pending.desired_width, shell_surface->client_pending.desired_height);
}

LayerSurface::~LayerSurface() noexcept
{
  wl_list_remove(&map.link);
  wl_list_remove(&unmap.link);
  wl_list_remove(&destroy.link);
  wl_list_remove(&new_popup.link);
}

void LayerSurface::shell_surface_map(wl_listener *listener, void *data)
{
  std::cout << "mapping layer surface!" << std::endl;
  Server &server(Server::getInstance());
  wlr_layer_surface_v1 *shell_surface = wlr_layer_surface_v1_from_wlr_surface(surface);

  auto &pending_surfaces(server.layerShell.pending_surfaces);
  auto it(--std::find_if(pending_surfaces.rbegin(), pending_surfaces.rend(), [this](auto &a) noexcept
									     {
									       return a.get() == this;
									     }).base()); // tiny opti with reverse iterator
      
  server.outputManager.getOutput(shell_surface->output).addLayerSurface(std::move(*it));
  pending_surfaces.erase(it);
  if (shell_surface->current.keyboard_interactive)
    {
      wlr_seat *seat = server.seat.getSeat();
      wlr_keyboard *keyboard = wlr_seat_get_keyboard(seat);

      if (wlr_surface *surface = server.getFocusedSurface();
	  !surface ||
	  (wlr_surface_is_layer_surface(surface) &&
	   wlr_layer_surface_v1_from_wlr_surface(surface)->current.layer <= shell_surface->current.layer))
	{
	  wlr_seat_keyboard_notify_enter(seat, surface, keyboard->keycodes,
					 keyboard->num_keycodes, &keyboard->modifiers);
	  server.outputManager.getOutput(shell_surface->output).setFocusedLayerSurface(this);
	}
    }
}

void LayerSurface::shell_surface_unmap(wl_listener *listenr, void *data)
{
  std::cout << "unmapping layer surface!" << std::endl;
  Server &server(Server::getInstance());
  wlr_layer_surface_v1 *shell_surface = wlr_layer_surface_v1_from_wlr_surface(surface);

  wlr_seat *seat = server.seat.getSeat();
  wlr_keyboard *keyboard = wlr_seat_get_keyboard(seat);
  wlr_surface *prev_surface = seat->keyboard_state.focused_surface;

  if (prev_surface == surface)
    {
      {
	Output &output(server.outputManager.getActiveWorkspace()->getOutput());
	for (int i = ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY; i >= ZWLR_LAYER_SHELL_V1_LAYER_TOP; --i)
	  for (auto const &layerSurfacePtr : output.getLayers()[i])
	    {
	      if (this != layerSurfacePtr.get() &&
		  wlr_layer_surface_v1_from_wlr_surface(layerSurfacePtr->surface)->current.keyboard_interactive)
		{
		  wlr_seat_keyboard_notify_enter(seat, layerSurfacePtr->surface, keyboard->keycodes, keyboard->num_keycodes, &keyboard->modifiers);
		  output.setFocusedLayerSurface(layerSurfacePtr.get());
		  goto done;
		}
	    }
      }
      if (View *view = server.getFocusedView())
	{
	  if (wlr_surface_is_xdg_surface_v6(view->surface))
	    wlr_xdg_toplevel_v6_set_activated(wlr_xdg_surface_v6_from_wlr_surface(view->surface), true);
	  else if (wlr_surface_is_xdg_surface(view->surface))
	    wlr_xdg_toplevel_set_activated(wlr_xdg_surface_from_wlr_surface(view->surface), true);
	  wlr_seat_keyboard_notify_enter(seat, view->surface, keyboard->keycodes,
					 keyboard->num_keycodes, &keyboard->modifiers);
      
	}
    }
 done:
  server.outputManager.getOutput(shell_surface->output).removeLayerSurface(this);

}

void LayerSurface::close()
{
  wlr_layer_surface_v1_close(wlr_layer_surface_v1_from_wlr_surface(surface));
}
