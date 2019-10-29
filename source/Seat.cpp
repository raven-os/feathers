#include "Seat.hpp"
#include "Server.hpp"

Seat::Seat()
{
  seat = wlr_seat_create(Server::getInstance().getWlDisplay(), "seat0");

  SET_LISTENER(Seat, SeatListeners, request_cursor, seat_request_cursor);
  wl_signal_add(&seat->events.request_set_cursor, &request_cursor);

  SET_LISTENER(Seat, SeatListeners, request_set_selection, seat_request_set_selection);
  wl_signal_add(&seat->events.request_set_selection, &request_set_selection);

  SET_LISTENER(Seat, SeatListeners, request_set_primary_selection, seat_request_set_primary_selection);
  wl_signal_add(&seat->events.request_set_primary_selection, &request_set_primary_selection);
}

void Seat::seat_request_cursor(wl_listener *listener, void *data)
{
  wlr_seat_pointer_request_set_cursor_event *event = static_cast<wlr_seat_pointer_request_set_cursor_event *>(data);
  wlr_seat_client *focused_client = seat->pointer_state.focused_client;
  if (focused_client == event->seat_client)
    {
      wlr_cursor_set_surface(Server::getInstance().cursor.cursor, event->surface, event->hotspot_x, event->hotspot_y);
    }
}

void Seat::seat_request_set_selection(wl_listener *listener, void *data)
{
  wlr_seat_request_set_selection_event *event =
    static_cast<wlr_seat_request_set_selection_event *>(data);
  wlr_seat_set_selection(seat, event->source, event->serial);
}

void Seat::seat_request_set_primary_selection(wl_listener *listener, void *data)
{
  wlr_seat_request_set_primary_selection_event *event =
    static_cast<wlr_seat_request_set_primary_selection_event *>(data);
  wlr_seat_set_primary_selection(seat, event->source, event->serial);
}

wlr_seat *Seat::getSeat() const noexcept
{
  return seat;
}
