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
  ~LayerShell() = default;

  void newSurface([[maybe_unused]]struct wl_listener *listener, void *data);
  void shell_surface_destroy([[maybe_unused]]struct wl_listener *listener, [[maybe_unused]]void *data);

private:
  wlr_layer_shell_v1 *layer_shell;
  std::vector<LayerSurface *> pending_surfaces; // surfaces that aren't mapped yet, so we can't put them in the right layer yet.
};
