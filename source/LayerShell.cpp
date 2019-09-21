#include "LayerShell.hpp"
#include "LayerSurface.hpp"
#include "Server.hpp"

LayerShell::LayerShell()
  : layer_shell(wlr_layer_shell_v1_create(Server::getInstance().getWlDisplay()))
{
  SET_LISTENER(LayerShell, LayerShellListeners, new_shell_surface, newSurface);
  wl_signal_add(&layer_shell->events.new_surface, &new_shell_surface);
}

void LayerShell::shell_surface_destroy([[maybe_unused]]struct wl_listener *listener, [[maybe_unused]]void *data)
{
  Server &server = Server::getInstance();
  View *view = wl_container_of(listener, view, destroy);

  if (server.getViews().front().get() == view)
    server.seat.getSeat()->keyboard_state.focused_surface = nullptr;
  server.getViews().erase(std::find_if(server.getViews().begin(), server.getViews().end(),
				       [view](auto const &ptr) noexcept
				       {
					 return ptr.get() == view;
				       }));
  if (!server.getViews().empty())
    {
      std::unique_ptr<View> &currentView = server.getViews().front();
      currentView->focus_view();
    }
}

void LayerShell::newSurface([[maybe_unused]]struct wl_listener *listener, void *data)
{
  wlr_layer_surface_v1 *shell_surface = static_cast<struct wlr_layer_surface_v1 *>(data);

  pending_surfaces.emplace_back(new LayerSurface(shell_surface->surface));
}
