#pragma once

# include "Wlroots.hpp"
# include "Listeners.hpp"
# include "Workspace.hpp"
# include "wm/WindowTree.hpp"

class Server;
class LayerSurface;

struct OutputListeners
{
  wl_listener frame;
};

class Output : public OutputListeners
{
public:

  Output(struct wlr_output *wlr_output, uint16_t workspaceCount);
  ~Output() noexcept;

  void setFrameListener();
  void setFullscreenView(View *view) noexcept;

  View *getFullscreenView() const noexcept
  {
    return fullscreenView;
  }

  std::vector<std::unique_ptr<Workspace>> &getWorkspaces() noexcept;
  wm::WindowTree &getWindowTree() noexcept;

  struct wlr_output *getWlrOutput() const;

  wlr_box saved;

  void addLayerSurface(std::unique_ptr<LayerSurface> &&layerSurface);
  void removeLayerSurface(LayerSurface *layerSurface);
private:
  std::vector<std::unique_ptr<Workspace>> workspaces;
  struct wlr_output *wlr_output;
  View *fullscreenView;
  struct wlr_texture *wallpaperTexture;
  std::array<std::vector<std::unique_ptr<LayerSurface>>, 4> layers;

private:
  void output_frame(wl_listener *listener, void *data);
  void refreshImage();
  
};
