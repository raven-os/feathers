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

#include "XdgView.hpp"
#include "LayerSurface.hpp"

Output::Output(struct wlr_output *wlr_output, uint16_t workspaceCount) :
  wlr_output(wlr_output),
  fullscreenView(nullptr)
{
  refreshImage();

  for (uint16_t i = 0; i < workspaceCount; ++i)
    workspaces.emplace_back(new Workspace(*this));

  if (Server::getInstance().outputManager.getActiveWorkspace() == nullptr) {
    Server::getInstance().outputManager.setActiveWorkspace(workspaces[0].get());
  }
}

Output::~Output() noexcept = default;

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

void Output::output_frame(wl_listener *listener, void *data)
{
  Server &server = Server::getInstance();
  wlr_renderer *renderer = server.renderer;

  timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);

  if (!wlr_output_attach_render(wlr_output, NULL))
    {
      return;
    }
  int width, height;
  wlr_output_effective_resolution(wlr_output, &width, &height);
  wlr_renderer_begin(renderer, width, height);

  // float color[4] = {0.5, 0.5, 0.5, 1.0};
  // wlr_renderer_clear(renderer, color);

  if (XdgView *view = getFullscreenView())
    {
      render_data rdata{
			.output = wlr_output,
			.renderer = renderer,
			.view = static_cast<View *>(view),
			.when = &now,
			.fullscreen = true
      };

      if (wlr_surface_is_xdg_surface(view->surface))
	wlr_xdg_surface_for_each_surface(wlr_xdg_surface_from_wlr_surface(view->surface), OutputManager::render_surface, &rdata);
      else if (wlr_surface_is_xdg_surface_v6(view->surface))
	wlr_xdg_surface_v6_for_each_surface(wlr_xdg_surface_v6_from_wlr_surface(view->surface), OutputManager::render_surface, &rdata);
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


      for (int i = ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND; i <= ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM; ++i)
	for (auto &layerSurface : layers[i])
	  {
	    render_data rdata{
			      .output = wlr_output,
			      .renderer = renderer,
			      .view = static_cast<View *>(layerSurface.get()),
			      .when = &now,
			      .fullscreen = false
	    };
	    if (wlr_surface_is_layer_surface(layerSurface->surface))
	      wlr_layer_surface_v1_for_each_surface(wlr_layer_surface_v1_from_wlr_surface(layerSurface->surface), OutputManager::render_surface, &rdata);
	  }
      for (auto it = server.getViews().rbegin(); it != server.getViews().rend(); ++it)
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
	  if (wlr_surface_is_xdg_surface(view->surface))
	    wlr_xdg_surface_for_each_surface(wlr_xdg_surface_from_wlr_surface(view->surface), OutputManager::render_surface, &rdata);
	  else if (wlr_surface_is_xdg_surface_v6(view->surface))
	    wlr_xdg_surface_v6_for_each_surface(wlr_xdg_surface_v6_from_wlr_surface(view->surface), OutputManager::render_surface, &rdata);
	}
    }

  // these are above surfaces, even fullscree ones (TODO: config with albinos)
  for (int i = ZWLR_LAYER_SHELL_V1_LAYER_TOP; i <= ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY; ++i)
    for (auto &layerSurface : layers[i])
      {
	render_data rdata{
			  .output = wlr_output,
			  .renderer = renderer,
			  .view = static_cast<View *>(layerSurface.get()),
			  .when = &now,
			  .fullscreen = false
	};
	if (wlr_surface_is_layer_surface(layerSurface->surface))
	  wlr_layer_surface_v1_for_each_surface(wlr_layer_surface_v1_from_wlr_surface(layerSurface->surface), OutputManager::render_surface, &rdata);
      }

  wlr_output_render_software_cursors(wlr_output, NULL);
  wlr_renderer_end(renderer);
  wlr_output_commit(wlr_output);
  refreshImage();
  auto *box = wlr_output_layout_get_box(server.outputManager.getLayout(), wlr_output);
  {
    wm::WindowTree &windowTree = getWindowTree();

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

void Output::setFullscreenView(XdgView *view) noexcept
{
  this->fullscreenView = view;
}

wm::WindowTree &Output::getWindowTree() noexcept
{
  return Server::getInstance().outputManager.getActiveWorkspace()->getWindowTree();
}

std::vector<std::unique_ptr<Workspace>> &Output::getWorkspaces() noexcept
{
  return workspaces;
}

struct wlr_output *Output::getWlrOutput() const
{
  return wlr_output;
}

void Output::addLayerSurface(std::unique_ptr<LayerSurface> &&layerSurface)
{
  wlr_layer_surface_v1 *shell_surface = wlr_layer_surface_v1_from_wlr_surface(layerSurface->surface);

  if (shell_surface->current.layer < layers.size())
     {
       layers[shell_surface->current.layer].emplace_back(std::move(layerSurface));
     }
}

void Output::removeLayerSurface(LayerSurface *layerSurface)
{
  wlr_layer_surface_v1 *shell_surface = wlr_layer_surface_v1_from_wlr_surface(layerSurface->surface);

  auto it(std::find_if(layers[shell_surface->current.layer].begin(),layers[shell_surface->current.layer].end(), [layerSurface](auto &a) noexcept
												{
												  return a.get() == layerSurface;
												}));
  layers[shell_surface->current.layer].erase(it);
}
