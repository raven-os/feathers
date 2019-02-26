#pragma once

# include "Wlroots.hpp"

namespace Seat
{
  void seat_request_cursor(struct wl_listener *listener, void *data);
};
