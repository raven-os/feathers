#include "Output.hpp"
#include "Server.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wcast-qual"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wuseless-cast"
#pragma GCC diagnostic ignored "-Wswitch-default"
#pragma GCC diagnostic ignored "-Wstrict-overflow"
#pragma GCC diagnostic ignored "-Wtype-limits"
#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"
#pragma GCC diagnostic pop

Output::Output(struct wlr_output *wlr_output) :
  wlr_output(wlr_output),
  fullscreenView(nullptr),
  windowTree([&]()
	     {
	       auto box = wlr_output_layout_get_box(Server::getInstance().outputManager.getLayout(), wlr_output);

	       return wm::WindowData{wm::Container(wm::Rect{{{FixedPoint<0, int>(box->x),
							      FixedPoint<0, int>(box->y)}},
							    {{FixedPoint<0, int>(box->width),
							      FixedPoint<0, int>(box->height)}}})};
	     }())
{
  refreshImage();
}

void Output::refreshImage()
{
  Server &server = Server::getInstance();
  char const *setting("background_image");

  server.configuration.poll();
  if (server.configuration.consumeChanged(setting))
    {
      int width, height, channels;
      char const *imagePath = server.configuration.get(setting);
      unsigned char *image = stbi_load(imagePath,
				       &width,
				       &height,
				       &channels,
				       STBI_rgb_alpha);
      if (!image)
	{
	  image = stbi_load("wallpaper.jpg",
			    &width,
			    &height,
			    &channels,
			    STBI_rgb_alpha);
	}

      wallpaperTexture =
	wlr_texture_from_pixels(server.renderer, WL_SHM_FORMAT_ABGR8888, width * 4,
				width, height, image);
      stbi_image_free(image);
    }
}

void Output::output_frame([[maybe_unused]]struct wl_listener *listener, [[maybe_unused]]void *data)
{
  Server &server = Server::getInstance();
  struct wlr_renderer *renderer = server.renderer;

  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);

  if (!wlr_output_make_current(wlr_output, NULL))
    {
      return;
    }
  int width, height;
  wlr_output_effective_resolution(wlr_output, &width, &height);
  wlr_renderer_begin(renderer, width, height);

  // float color[4] = {0.5, 0.5, 0.5, 1.0};
  // wlr_renderer_clear(renderer, color);

  if (View *view = getFullscreenView())
    {
      render_data rdata{
			.output = wlr_output,
			.renderer = renderer,
			.view = view,
			.when = &now,
			.fullscreen = true
      };

      wlr_xdg_surface_v6_for_each_surface(view->xdg_surface, OutputManager::render_surface, &rdata);
    }
  else
    {
      // render wallpaper
      {
	std::array<float, 9> transform;
	std::copy(wlr_output->transform_matrix, wlr_output->transform_matrix + 9, transform.begin());

	std::array<int, 2> size;
	wlr_texture_get_size(wallpaperTexture, &size[0], &size[1]);

	wlr_matrix_scale(transform.data(), float(width) / float(size[0]), float(height) / float(size[1]));
	wlr_render_texture(renderer, wallpaperTexture, transform.data(), 0, 0, 1.0f);
      }

      for (auto it = server.views.rbegin(); it != server.views.rend(); ++it)
	{
	  auto &view(*it);

	  if (!view->mapped)
	    {
	      continue;
	    }
	  render_data rdata{
	    .output = wlr_output,
	      .renderer = renderer,
	      .view = view.get(),
	      .when = &now,
	      .fullscreen = false
	      };
	  wlr_xdg_surface_v6_for_each_surface(view->xdg_surface, OutputManager::render_surface, &rdata);
	}
    }

  wlr_output_render_software_cursors(wlr_output, NULL);
  wlr_renderer_end(renderer);
  wlr_output_swap_buffers(wlr_output, NULL, NULL);
  refreshImage();
  auto *box = wlr_output_layout_get_box(server.outputManager.getLayout(), wlr_output);
  {
    std::array<FixedPoint<-4, int>, 2> pos{{FixedPoint<0, int>(box->x), FixedPoint<0, int>(box->y)}};
    if (windowTree.getData(windowTree.getRootIndex()).getPosition() != pos)
      windowTree.getData(windowTree.getRootIndex()).move(windowTree.getRootIndex(), windowTree, pos);
    if (windowTree.getData(windowTree.getRootIndex()).getSize() != std::array<uint16_t, 2u>{uint16_t(box->width), uint16_t(box->height)})
      windowTree.getData(windowTree.getRootIndex()).resize(windowTree.getRootIndex(), windowTree, {uint16_t(box->width), uint16_t(box->height)});
  }
}

void Output::setFrameListener()
{
    SET_LISTENER(Output, OutputListeners, frame, output_frame);
    wl_signal_add(&wlr_output->events.frame, &frame);
}

void Output::setFullscreenView(View *view) noexcept
{
  this->fullscreenView = view;
}

struct wlr_output *Output::getWlrOutput() const
{
  return wlr_output;
}
