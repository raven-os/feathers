#pragma once

# include "Wlroots.hpp"

namespace ServerOutput
{
  void render_surface(struct wlr_surface *surface, int sx, int sy, void *data);
  void output_frame(struct wl_listener *listener, void *data);
  void server_new_output(struct wl_listener *listener, void *data);
};

/* Used to move all of the data necessary to render a surface from the top-level
 * frame handler to the per-surface render function. */
struct render_data
{
  struct wlr_output *output;
  struct wlr_renderer *renderer;
  struct View *view;
  struct timespec *when;
};
