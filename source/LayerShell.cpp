#include "LayerShell.hpp"
#include "LayerSurface.hpp"
#include "Server.hpp"
#include "Output.hpp"

LayerShell::LayerShell()
  : layer_shell(wlr_layer_shell_v1_create(Server::getInstance().getWlDisplay()))
{
  SET_LISTENER(LayerShell, LayerShellListeners, new_shell_surface, newSurface);
  wl_signal_add(&layer_shell->events.new_surface, &new_shell_surface);
}

/// Destructor is here so that LayerSurface.hpp doesn't need to be included in LayerShell.hpp
LayerShell::~LayerShell() noexcept = default;

void LayerShell::shell_surface_destroy(wl_listener *listener, void *data)
{
  // already deallocated, nothing to do
}

void LayerShell::newSurface(wl_listener *listener, void *data)
{
  wlr_layer_surface_v1 *shell_surface = static_cast<wlr_layer_surface_v1 *>(data);

  pending_surfaces.emplace_back(std::make_unique<LayerSurface>(shell_surface->surface));
}
