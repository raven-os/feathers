#pragma once

# include "Wlroots.hpp"

# include "View.hpp"
# include "Listeners.hpp"

struct LayerShellListeners
{
  struct wl_listener new_shell_surface;
  struct wl_listener destroy;
};

class LayerSurface;

class LayerShell : public LayerShellListeners
{
public:
  LayerShell();
  ~LayerShell() noexcept;

  void newSurface([[maybe_unused]]struct wl_listener *listener, void *data);
  void shell_surface_destroy([[maybe_unused]]struct wl_listener *listener, [[maybe_unused]]void *data);

  std::vector<std::unique_ptr<LayerSurface>> pending_surfaces; // surfaces that aren't mapped yet, so we can't put them in the right layer yet.

private:
  wlr_layer_shell_v1 *layer_shell;
};
