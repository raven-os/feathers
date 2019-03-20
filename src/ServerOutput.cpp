#include "ServerOutput.hpp"
#include "Server.hpp"

ServerOutput::ServerOutput(Server *server) : server(server) {
  output_layout = wlr_output_layout_create();
  wl_list_init(&outputs);
  SET_LISTENER(ServerOutput, ServerOutputListeners, new_output, server_new_output);
  wl_signal_add(&server->backend->events.new_output, &new_output);
}

void ServerOutput::server_new_output(struct wl_listener *listener, void *data)
{
  struct wlr_output *wlr_output = static_cast<struct wlr_output*>(data);

  if (!wl_list_empty(&wlr_output->modes))
    {
      struct wlr_output_mode *mode = wl_container_of(wlr_output->modes.prev, mode, link);
      wlr_output_set_mode(wlr_output, mode);
    }

  Output *output = new Output(server, wlr_output);
  output->setFrameListener();
  //wl_signal_add(&wlr_output->events.frame, &output->frame);
  wl_list_insert(&outputs, &output->link);

  wlr_output_layout_add_auto(output_layout, wlr_output);
  wlr_output_create_global(wlr_output);
}

void ServerOutput::render_surface(struct wlr_surface *surface, int sx, int sy, void *data)
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
  wlr_output_layout_output_coords(view->server->output->getLayout(), output, &ox, &oy);
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

struct wlr_output_layout *ServerOutput::getLayout() const
{
  return output_layout;
}
