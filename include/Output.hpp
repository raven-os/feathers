#ifndef OUTPUT_HPP_
# define OUTPUT_HPP_

# include "Wlroots.hpp"

class Server;

struct Output
{
  struct wl_list link;
  Server *server;
  struct wlr_output *wlr_output;
  struct wl_listener frame;
};

#endif /* !OUTPUT_HPP_ */
