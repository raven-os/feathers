#include "Seat.hpp"
#include "Server.hpp"

Seat::Seat(Server *server) : server(server)
{
  seat = wlr_seat_create(server->display, "seat0");
  SET_LISTENER(Seat, SeatListeners, request_cursor, seat_request_cursor);
  wl_signal_add(&seat->events.request_set_cursor, &request_cursor);
}

void Seat::seat_request_cursor(struct wl_listener *listener, void *data)
{
  struct wlr_seat_pointer_request_set_cursor_event *event = static_cast<struct wlr_seat_pointer_request_set_cursor_event *>(data);
  struct wlr_seat_client *focused_client = seat->pointer_state.focused_client;
  if (focused_client == event->seat_client)
    {
wlr_cursor_set_surface(server->cursor->cursor, event->surface, event->hotspot_x, event->hotspot_y);
    }
}

struct wlr_seat *Seat::getSeat() const noexcept
{
  return seat;
}
