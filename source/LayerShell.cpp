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

/// Desgtrucor is here so that LayerSurface.hpp doesn't need to be included in LayerShell.hpp
LayerShell::~LayerShell() noexcept = default;

void LayerShell::shell_surface_destroy([[maybe_unused]]struct wl_listener *listener, [[maybe_unused]]void *data)
{
  View *view = wl_container_of(listener, view, destroy);

}

void LayerShell::newSurface([[maybe_unused]]struct wl_listener *listener, void *data)
{
  wlr_layer_surface_v1 *shell_surface = static_cast<struct wlr_layer_surface_v1 *>(data);

  pending_surfaces.emplace_back(new LayerSurface(shell_surface->surface));
}
