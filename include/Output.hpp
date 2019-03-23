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

  struct wl_list link;

  void setFrameListener();

private:
  Server *server;
  struct wlr_output *wlr_output;

private:
  void output_frame(struct wl_listener *listener, void *data);
};
