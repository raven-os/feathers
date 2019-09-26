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
}

void LayerSurface::shell_surface_unmap(wl_listener *listenr, void *data)
{
  std::cout << "unmapping layer surface!" << std::endl;
  Server &server(Server::getInstance());
  wlr_layer_surface_v1 *shell_surface = wlr_layer_surface_v1_from_wlr_surface(surface);

  server.outputManager.getOutput(shell_surface->output).removeLayerSurface(this);
}

void LayerSurface::close()
{
  wlr_layer_surface_v1_close(wlr_layer_surface_v1_from_wlr_surface(surface));
}
