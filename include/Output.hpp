#pragma once

# include "Wlroots.hpp"
# include "Listeners.hpp"

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
  void setFullscreen(bool fullscreen);
  bool getFullscreen() const;
  struct wlr_output *getWlrOutput() const;

  wlr_box saved;

private:
  Server *server;
  struct wlr_output *wlr_output;
  bool fullscreen;

private:
  void output_frame(struct wl_listener *listener, void *data);
};
