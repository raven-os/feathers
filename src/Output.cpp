#include "Output.hpp"
#include "Server.hpp"

Output::Output(Server *server, struct wlr_output *wlr_output) : server(server), wlr_output(wlr_output)
{

}

void Output::output_frame(struct wl_listener *listener, void *data)
{
  struct wlr_renderer *renderer = server->renderer;

  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);

  if (!wlr_output_make_current(wlr_output, NULL))
    {
      return;
    }
  int width, height;
  wlr_output_effective_resolution(wlr_output, &width, &height);
  wlr_renderer_begin(renderer, width, height);

  float color[4] = {0.3, 0.3, 0.3, 1.0};
  wlr_renderer_clear(renderer, color);

  View *view;
  wl_list_for_each_reverse(view, &server->views, link) {
    if (!view->mapped)
      {
	continue;
      }
    render_data rdata;
    rdata.output = wlr_output;
    rdata.view = view;
    rdata.renderer = renderer;
    rdata.when = &now;
    wlr_xdg_surface_for_each_surface(view->xdg_surface, ServerOutput::render_surface, &rdata);
  }

  wlr_output_render_software_cursors(wlr_output, NULL);
  wlr_renderer_end(renderer);
  wlr_output_swap_buffers(wlr_output, NULL, NULL);
}

void Output::setFrameListener()
{
    SET_LISTENER(Output, OutputListeners, frame, output_frame);
    wl_signal_add(&wlr_output->events.frame, &frame);
}
