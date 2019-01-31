#ifndef SERVERINPUT_HPP_
# define SERVERINPUT_HPP_

# include "Server.hpp"

namespace ServerInput
{
  void server_new_input(struct wl_listener *listener, void *data);
};

#endif /* !SERVERINPUT_HPP_ */
