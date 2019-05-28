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

  Output(Server *server, struct wlr_output *wlr_output, struct wlr_output_layout *wlr_output_layout);
  ~Output() = default;

  void setFrameListener();
  void setFullscreen(bool fullscreen);
  bool getFullscreen() const;
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
  bool fullscreen;
  wm::WindowTree windowTree;
  struct wlr_texture *wallpaperTexture;

private:
  void output_frame(struct wl_listener *listener, void *data);
  void refreshImage();
};
