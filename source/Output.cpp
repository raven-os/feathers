#include "Output.hpp"
#include "Server.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Output::Output(Server *server, struct wlr_output *wlr_output) :
  server(server),
  wlr_output(wlr_output),
  fullscreen(false),
  windowTree([&]()
	     {
	       // todo: get output size
	       return wm::WindowData{wm::Container{{{{0, 0}}, {{1920, 1080}}}}};
	     }())
{

  int width, height, channels;
  // TODO load user wallpaper
  unsigned char *image = stbi_load("wallpaper.jpg",
				   &width,
				   &height,
				   &channels,
				   STBI_rgb_alpha);
  wallpaperTexture =
    wlr_texture_from_pixels(server->renderer, WL_SHM_FORMAT_ABGR8888, width * 4,
			    width, height, image);
  stbi_image_free(image);
}

void Output::output_frame([[maybe_unused]]struct wl_listener *listener, [[maybe_unused]]void *data)
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

  float color[4] = {0.5, 0.5, 0.5, 1.0};
  wlr_renderer_clear(renderer, color);

  // render wallpaper
  wlr_render_texture(renderer, wallpaperTexture, wlr_output->transform_matrix, 0, 0, 1.0f);

  for (auto it = server->views.rbegin(); it != server->views.rend(); ++it)
    {
      auto &view(*it);

      if (!view->mapped)
	{
	  continue;
	}
      render_data rdata;
      rdata.output = wlr_output;
      rdata.view = view.get();
      rdata.renderer = renderer;
      rdata.when = &now;
      wlr_xdg_surface_v6_for_each_surface(view->xdg_surface, ServerOutput::render_surface, &rdata);
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

void Output::setFullscreen(bool fullscreen)
{
  this->fullscreen = fullscreen;
}

bool Output::getFullscreen() const
{
  return fullscreen;
}

struct wlr_output *Output::getWlrOutput() const
{
  return wlr_output;
}
