#include "ServerOutput.hpp"
#include "Server.hpp"

namespace ServerOutput
{
  void render_surface(struct wlr_surface *surface, int sx, int sy, void *data)
  {
    render_data *rdata = static_cast<render_data*>(data);
    View *view = rdata->view;
    struct wlr_output *output = rdata->output;

    struct wlr_texture *texture = wlr_surface_get_texture(surface);
    if (!texture)
      {
	return;
      }

    double ox = 0, oy = 0;
    wlr_output_layout_output_coords(view->server->output_layout, output, &ox, &oy);
    ox += view->x + sx, oy += view->y + sy;

    struct wlr_box box = {
	.x = ox * output->scale,
	.y = oy * output->scale,
	.width = surface->current.width * output->scale,
	.height = surface->current.height * output->scale,
    };

    float matrix[9];
    enum wl_output_transform transform = wlr_output_transform_invert(surface->current.transform);
    wlr_matrix_project_box(matrix, &box, transform, 0,
			   output->transform_matrix);

    wlr_render_texture_with_matrix(rdata->renderer, texture, matrix, 1);

    wlr_surface_send_frame_done(surface, rdata->when);
  }

  void output_frame(struct wl_listener *listener, void *data)
  {
    Output *output = wl_container_of(listener, output, frame);
    struct wlr_renderer *renderer = output->server->renderer;

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    if (!wlr_output_make_current(output->wlr_output, NULL))
      {
	return;
      }
    int width, height;
    wlr_output_effective_resolution(output->wlr_output, &width, &height);
    wlr_renderer_begin(renderer, width, height);

    float color[4] = {0.3, 0.3, 0.3, 1.0};
    wlr_renderer_clear(renderer, color);

    View *view;
    wl_list_for_each_reverse(view, &output->server->views, link) {
      if (!view->mapped)
	{
	  continue;
	}
      render_data rdata;
      rdata.output = output->wlr_output;
      rdata.view = view;
      rdata.renderer = renderer;
      rdata.when = &now;
      wlr_xdg_surface_for_each_surface(view->xdg_surface, render_surface, &rdata);
    }

    wlr_output_render_software_cursors(output->wlr_output, NULL);
    wlr_renderer_end(renderer);
    wlr_output_swap_buffers(output->wlr_output, NULL, NULL);
  }

  void server_new_output(struct wl_listener *listener, void *data)
  {
    Server *server = wl_container_of(listener, server, new_output);
    struct wlr_output *wlr_output = static_cast<struct wlr_output*>(data);

    if (!wl_list_empty(&wlr_output->modes))
      {
	struct wlr_output_mode *mode = wl_container_of(wlr_output->modes.prev, mode, link);
	wlr_output_set_mode(wlr_output, mode);
      }

    Output *output = new Output();
    output->wlr_output = wlr_output;
    output->server = server;
    output->frame.notify = output_frame;
    wl_signal_add(&wlr_output->events.frame, &output->frame);
    wl_list_insert(&server->outputs, &output->link);

    wlr_output_layout_add_auto(server->output_layout, wlr_output);
    wlr_output_create_global(wlr_output);
  }
}
