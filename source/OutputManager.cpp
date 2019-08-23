#include "OutputManager.hpp"
#include "Server.hpp"

OutputManager::OutputManager() {
  output_layout = wlr_output_layout_create();
  SET_LISTENER(OutputManager, OutputManagerListeners, new_output, server_new_output);
  wl_signal_add(&Server::getInstance().backend->events.new_output, &new_output);
}

void OutputManager::server_new_output([[maybe_unused]]struct wl_listener *listener, void *data)
{
  struct wlr_output *wlr_output = static_cast<struct wlr_output*>(data);

  if (!wl_list_empty(&wlr_output->modes))
    {
      struct wlr_output_mode *mode = wl_container_of(wlr_output->modes.prev, mode, link);
      wlr_output_set_mode(wlr_output, mode);
    }

  wlr_output_layout_add_auto(output_layout, wlr_output);

  std::unique_ptr<Output> output(new Output(wlr_output, workspaceCount));
  output->setFrameListener();
  outputs.emplace_back(std::move(output));

  wlr_output_create_global(wlr_output);
}

void OutputManager::render_surface(struct wlr_surface *surface, int sx, int sy, void *data)
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
  wlr_output_layout_output_coords(Server::getInstance().outputManager.getLayout(), output, &ox, &oy);
  ox += sx;
  oy += sy;
  if (!rdata->fullscreen)
    {
      ox += view->x.getDoubleValue();
      oy += view->y.getDoubleValue();
    }

  struct wlr_box box = {
			.x = int(ox * output->scale),
			.y = int(oy * output->scale),
			.width = int(float(surface->current.width) * output->scale),
			.height = int(float(surface->current.height) * output->scale),
  };

  float matrix[9];
  enum wl_output_transform transform = wlr_output_transform_invert(surface->current.transform);
  wlr_matrix_project_box(matrix, &box, transform, 0,
			 output->transform_matrix);

  wlr_render_texture_with_matrix(rdata->renderer, texture, matrix, 1);

  wlr_surface_send_frame_done(surface, rdata->when);
}

struct wlr_output_layout *OutputManager::getLayout() const noexcept
{
  return output_layout;
}

std::vector<std::unique_ptr<Output>> const& OutputManager::getOutputs() const
{
  return outputs;
}

Output &OutputManager::getOutput(wlr_output *wlr_output) noexcept
{
  return *std::find_if(getOutputs().begin(), getOutputs().end(),
		       [&wlr_output](auto &out) noexcept {
			 return out->getWlrOutput() == wlr_output;
		       })->get();
}

Output const &OutputManager::getOutput(wlr_output *wlr_output) const noexcept
{
  return const_cast<OutputManager *>(this)->getOutput(wlr_output);
}
