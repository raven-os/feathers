#ifndef SEAT_HPP_
# define SEAT_HPP_

# include "Wlroots.hpp"

namespace Seat
{
  void seat_request_cursor(struct wl_listener *listener, void *data);
};

#endif /* !SEAT_HPP_ */
