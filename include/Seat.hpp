#pragma once

# include "Wlroots.hpp"
# include "Listeners.hpp"

class Server;

struct SeatListeners
{
  struct wl_listener request_cursor;
};

class Seat : public SeatListeners
{
public:
  Seat();
  ~Seat() = default;

  void seat_request_cursor(struct wl_listener *listener, void *data);
  struct wlr_seat *getSeat() const noexcept;

private:
  Server &server;
  struct wlr_seat *seat;
};
