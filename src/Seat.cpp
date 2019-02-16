#include "Seat.hpp"
#include "Server.hpp"

namespace Seat
{
  void seat_request_cursor(struct wl_listener *listener, void *data)
  {
    Server *server = wl_container_of(listener, server, request_cursor);
    struct wlr_seat_pointer_request_set_cursor_event *event = static_cast<struct wlr_seat_pointer_request_set_cursor_event *>(data);
    struct wlr_seat_client *focused_client = server->seat->pointer_state.focused_client;
    if (focused_client == event->seat_client)
      {
	wlr_cursor_set_surface(server->cursor->cursor, event->surface, event->hotspot_x, event->hotspot_y);
      }
  }
}
