#pragma once

# include "Wlroots.hpp"
# include "Listeners.hpp"
# include "wm/WindowTree.hpp"

class Server;

struct OutputListeners
{
  struct wl_listener frame;
};

class Output : public OutputListeners
{
public:

  Output(Server *server, struct wlr_output *wlr_output);
  ~Output() = default;

  void setFrameListener();
  void setFullscreenView(View *view) noexcept;

  View *getFullscreenView() const noexcept
  {
    return fullscreenView;
  }

  struct wlr_output *getWlrOutput() const;

  wm::WindowTree &getWindowTree() noexcept
  {
    return windowTree;
  }

  wm::WindowTree const &getWindowTree() const noexcept
  {
    return windowTree;
  }

  wlr_box saved;

private:
  Server *server;
  struct wlr_output *wlr_output;
  View *fullscreenView;
  wm::WindowTree windowTree;
  struct wlr_texture *wallpaperTexture;

private:
  void output_frame(struct wl_listener *listener, void *data);
  void refreshImage();
};
