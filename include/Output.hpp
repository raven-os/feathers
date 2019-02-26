#pragma once

# include "Wlroots.hpp"

class Server;

struct Output
{
  struct wl_list link;
  Server *server;
  struct wlr_output *wlr_output;
  struct wl_listener frame;
};
