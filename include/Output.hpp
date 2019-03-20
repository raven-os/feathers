#pragma once

# include "Wlroots.hpp"
# include "Listeners.hpp"

class Server;


class Output : public Listeners::OutputListeners
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
