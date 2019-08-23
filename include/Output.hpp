#pragma once

# include "Wlroots.hpp"
# include "Listeners.hpp"
# include "Workspace.hpp"
# include "wm/WindowTree.hpp"

class Server;

struct OutputListeners
{
  struct wl_listener frame;
};

class Output : public OutputListeners
{
public:

  Output(struct wlr_output *wlr_output, uint16_t workspaceCount);
  ~Output() = default;

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

private:
  std::vector<std::unique_ptr<Workspace>> workspaces;
  struct wlr_output *wlr_output;
  View *fullscreenView;
  struct wlr_texture *wallpaperTexture;

private:
  void output_frame(struct wl_listener *listener, void *data);
  void refreshImage();
};
