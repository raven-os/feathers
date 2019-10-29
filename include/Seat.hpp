#pragma once

# include "Wlroots.hpp"
# include "Listeners.hpp"

class Server;

struct SeatListeners
{
  wl_listener request_cursor;
  wl_listener request_set_selection;
  wl_listener request_set_primary_selection;
};

class Seat : public SeatListeners
{
public:
  Seat();
  ~Seat() = default;

  void seat_request_set_selection(wl_listener *listener, void *data);
  void seat_request_set_primary_selection(wl_listener *listener, void *data);
  void seat_request_cursor(wl_listener *listener, void *data);

  wlr_seat *getSeat() const noexcept;

private:
  wlr_seat *seat;
};
