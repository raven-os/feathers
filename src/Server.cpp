#include <unistd.h>
#include "Server.hpp"
#include "XdgShell.hpp"
#include "ServerCursor.hpp"
#include "ServerOutput.hpp"
#include "ServerInput.hpp"
#include "Seat.hpp"

Server::Server()
{
  wlr_log_init(WLR_DEBUG, NULL);

  display = wl_display_create();
  // nullptr can be replaced with a custom renderer
  backend = wlr_backend_autocreate(display, nullptr);
  renderer = wlr_backend_get_renderer(backend);

  wlr_renderer_init_wl_display(renderer, display);
  wlr_compositor_create(display, renderer);
  wlr_data_device_manager_create(display);

  output_layout = wlr_output_layout_create();

  wl_list_init(&outputs);
  new_output.notify = ServerOutput::server_new_output;
  wl_signal_add(&backend->events.new_output, &new_output);


  wl_list_init(&views);
  xdg_shell = wlr_xdg_shell_create(display);
  new_xdg_surface.notify = XdgShell::server_new_xdg_surface;
  wl_signal_add(&xdg_shell->events.new_surface, &new_xdg_surface);

  cursor = new ServerCursor(this);
  // cursor = wlr_cursor_create();
  // wlr_cursor_attach_output_layout(cursor, output_layout);
  //
  // cursor_mgr = wlr_xcursor_manager_create(nullptr, 24);
  // wlr_xcursor_manager_load(cursor_mgr, 1);
  //
  // cursor_motion.notify = ServerCursor::server_cursor_motion;
  // wl_signal_add(&cursor->events.motion, &cursor_motion);
  // cursor_motion_absolute.notify = ServerCursor::server_cursor_motion_absolute;
  // wl_signal_add(&cursor->events.motion_absolute, &cursor_motion_absolute);
  // cursor_button.notify = ServerCursor::server_cursor_button;
  // wl_signal_add(&cursor->events.button, &cursor_button);
  // cursor_axis.notify = ServerCursor::server_cursor_axis;
  // wl_signal_add(&cursor->events.axis, &cursor_axis);

  wl_list_init(&keyboards);
  new_input.notify = ServerInput::server_new_input;
  wl_signal_add(&backend->events.new_input, &new_input);
  seat = wlr_seat_create(display, "seat0");
  request_cursor.notify = Seat::seat_request_cursor;
  wl_signal_add(&seat->events.request_set_cursor, &request_cursor);
}

Server::~Server()
{
  wl_display_destroy_clients(display);
  wl_display_destroy(display);
}

void Server::run()
{
  const char *socket = wl_display_add_socket_auto(display);
  if (!socket)
    {
      wlr_backend_destroy(backend);
      // TODO THROW
    }

  if (!wlr_backend_start(backend))
    {
      wlr_backend_destroy(backend);
      wl_display_destroy(display);
      // TODO THROW
    }

  setenv("WAYLAND_DISPLAY", socket, true);
  if (fork() == 0)
    {
      execl("/bin/sh", "/bin/sh", "-c", "gnome-terminal", nullptr);
    }
  wlr_log(WLR_INFO, "Running Wayland compositor on WAYLAND_DISPLAY=%s",
	  socket);
  wl_display_run(display);
}
