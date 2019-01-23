#include <unistd.h>
#include "Server.hpp"

static void render_surface(struct wlr_surface *surface, int sx, int sy, void *data)
{
  /* This function is called for every surface that needs to be rendered. */
  render_data *rdata = static_cast<render_data*>(data);
  View *view = rdata->view;
  struct wlr_output *output = rdata->output;

  /* We first obtain a wlr_texture, which is a GPU resource. wlroots
   * automatically handles negotiating these with the client. The underlying
   * resource could be an opaque handle passed from the client, or the client
   * could have sent a pixel buffer which we copied to the GPU, or a few other
   * means. You don't have to worry about this, wlroots takes care of it. */
  struct wlr_texture *texture = wlr_surface_get_texture(surface);
  if (!texture)
    {
      return;
    }

  /* The view has a position in layout coordinates. If you have two displays,
   * one next to the other, both 1080p, a view on the rightmost display might
   * have layout coordinates of 2000,100. We need to translate that to
   * output-local coordinates, or (2000 - 1920). */
  double ox = 0, oy = 0;
  wlr_output_layout_output_coords(
				  view->server->output_layout, output, &ox, &oy);
  ox += view->x + sx, oy += view->y + sy;

  /* We also have to apply the scale factor for HiDPI outputs. This is only
   * part of the puzzle, TinyWL does not fully support HiDPI. */
  struct wlr_box box = {
	.x = ox * output->scale,
	.y = oy * output->scale,
	.width = surface->current.width * output->scale,
	.height = surface->current.height * output->scale,
  };

  /*
   * Those familiar with OpenGL are also familiar with the role of matricies
   * in graphics programming. We need to prepare a matrix to render the view
   * with. wlr_matrix_project_box is a helper which takes a box with a desired
   * x, y coordinates, width and height, and an output geometry, then
   * prepares an orthographic projection and multiplies the necessary
   * transforms to produce a model-view-projection matrix.
   *
   * Naturally you can do this any way you like, for example to make a 3D
   * compositor.
   */
  float matrix[9];
  enum wl_output_transform transform = wlr_output_transform_invert(surface->current.transform);
  wlr_matrix_project_box(matrix, &box, transform, 0,
			 output->transform_matrix);

  /* This takes our matrix, the texture, and an alpha, and performs the actual
   * rendering on the GPU. */
  wlr_render_texture_with_matrix(rdata->renderer, texture, matrix, 1);

  /* This lets the client know that we've displayed that frame and it can
   * prepare another one now if it likes. */
  wlr_surface_send_frame_done(surface, rdata->when);
}

static void output_frame(struct wl_listener *listener, void *data)
{
  /* This function is called every time an output is ready to display a frame,
   * generally at the output's refresh rate (e.g. 60Hz). */
  Output *output = wl_container_of(listener, output, frame);
  struct wlr_renderer *renderer = output->server->renderer;

  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);

  /* wlr_output_make_current makes the OpenGL context current. */
  if (!wlr_output_make_current(output->wlr_output, NULL))
    {
      return;
    }
  /* The "effective" resolution can change if you rotate your outputs. */
  int width, height;
  wlr_output_effective_resolution(output->wlr_output, &width, &height);
  /* Begin the renderer (calls glViewport and some other GL sanity checks) */
  wlr_renderer_begin(renderer, width, height);

  float color[4] = {0.3, 0.3, 0.3, 1.0};
  wlr_renderer_clear(renderer, color);

  /* Each subsequent window we render is rendered on top of the last. Because
   * our view list is ordered front-to-back, we iterate over it backwards. */
  View *view;
  wl_list_for_each_reverse(view, &output->server->views, link) {
    if (!view->mapped)
      {
	/* An unmapped view should not be rendered. */
	continue;
      }
    render_data rdata;
    rdata.output = output->wlr_output;
    rdata.view = view;
    rdata.renderer = renderer;
    rdata.when = &now;
    /* This calls our render_surface function for each surface among the
     * xdg_surface's toplevel and popups. */
    wlr_xdg_surface_for_each_surface(view->xdg_surface, render_surface, &rdata);
  }

  /* Hardware cursors are rendered by the GPU on a separate plane, and can be
   * moved around without re-rendering what's beneath them - which is more
   * efficient. However, not all hardware supports hardware cursors. For this
   * reason, wlroots provides a software fallback, which we ask it to render
   * here. wlr_cursor handles configuring hardware vs software cursors for you,
   * and this function is a no-op when hardware cursors are in use. */
  wlr_output_render_software_cursors(output->wlr_output, NULL);

  /* Conclude rendering and swap the buffers, showing the final frame
   * on-screen. */
  wlr_renderer_end(renderer);
  wlr_output_swap_buffers(output->wlr_output, NULL, NULL);
}

static void server_new_output(struct wl_listener *listener, void *data)
{
  /* This event is rasied by the backend when a new output (aka a display or
   * monitor) becomes available. */
  Server *server = wl_container_of(listener, server, new_output);
  struct wlr_output *wlr_output = static_cast<struct wlr_output*>(data);

  /* Some backends don't have modes. DRM+KMS does, and we need to set a mode
   * before we can use the output. The mode is a tuple of (width, height,
   * refresh rate), and each monitor supports only a specific set of modes. We
   * just pick the first, a more sophisticated compositor would let the user
   * configure it or pick the mode the display advertises as preferred. */
  if (!wl_list_empty(&wlr_output->modes))
    {
      struct wlr_output_mode *mode = wl_container_of(wlr_output->modes.prev, mode, link);
      wlr_output_set_mode(wlr_output, mode);
    }

  /* Allocates and configures our state for this output */
  Output *output = new Output();
  output->wlr_output = wlr_output;
  output->server = server;
  /* Sets up a listener for the frame notify event. */
  output->frame.notify = output_frame;
  wl_signal_add(&wlr_output->events.frame, &output->frame);
  wl_list_insert(&server->outputs, &output->link);

  /* Adds this to the output layout. The add_auto function arranges outputs
   * from left-to-right in the order they appear. A more sophisticated
   * compositor would let the user configure the arrangement of outputs in the
   * layout. */
  wlr_output_layout_add_auto(server->output_layout, wlr_output);

  /* Creating the global adds a wl_output global to the display, which Wayland
   * clients can see to find out information about the output (such as
   * DPI, scale factor, manufacturer, etc). */
  wlr_output_create_global(wlr_output);
}

static void xdg_surface_map(struct wl_listener *listener, void *data)
{
  /* Called when the surface is mapped, or ready to display on-screen. */
  View *view = wl_container_of(listener, view, map);
  view->mapped = true;
  // TODO
  // focus_view(view, view->xdg_surface->surface);
}

static void xdg_surface_unmap(struct wl_listener *listener, void *data)
{
  /* Called when the surface is unmapped, and should no longer be shown. */
  View *view = wl_container_of(listener, view, unmap);
  view->mapped = false;
}

static void xdg_surface_destroy(struct wl_listener *listener, void *data)
{
  /* Called when the surface is destroyed and should never be shown again. */
  View *view = wl_container_of(listener, view, destroy);
  wl_list_remove(&view->link);
  free(view);
}

static void xdg_toplevel_request_move(struct wl_listener *listener, void *data)
{
  /* This event is raised when a client would like to begin an interactive
   * move, typically because the user clicked on their client-side
   * decorations. Note that a more sophisticated compositor should check the
   * provied serial against a list of button press serials sent to this
   * client, to prevent the client from requesting this whenever they want. */
  View *view = wl_container_of(listener, view, request_move);
  // TODO
  // begin_interactive(view, TINYWL_CURSOR_MOVE, 0);
}

static void xdg_toplevel_request_resize(struct wl_listener *listener, void *data) {
  /* This event is raised when a client would like to begin an interactive
   * resize, typically because the user clicked on their client-side
   * decorations. Note that a more sophisticated compositor should check the
   * provied serial against a list of button press serials sent to this
   * client, to prevent the client from requesting this whenever they want. */
  struct wlr_xdg_toplevel_resize_event *event = static_cast<struct wlr_xdg_toplevel_resize_event *>(data);
  View *view = wl_container_of(listener, view, request_resize);
  // begin_interactive(view, TINYWL_CURSOR_RESIZE, event->edges);
}

static void server_new_xdg_surface(struct wl_listener *listener, void *data)
{
  /* This event is raised when wlr_xdg_shell receives a new xdg surface from a
   * client, either a toplevel (application window) or popup. */
  Server *server = wl_container_of(listener, server, new_xdg_surface);
  struct wlr_xdg_surface *xdg_surface = static_cast<struct wlr_xdg_surface *>(data);
  if (xdg_surface->role != WLR_XDG_SURFACE_ROLE_TOPLEVEL)
    {
      return;
    }

  /* Allocate a tinywl_view for this surface */
  View *view = new View();
  view->server = server;
  view->xdg_surface = xdg_surface;

  /* Listen to the various events it can emit */
  view->map.notify = xdg_surface_map;
  wl_signal_add(&xdg_surface->events.map, &view->map);
  view->unmap.notify = xdg_surface_unmap;
  wl_signal_add(&xdg_surface->events.unmap, &view->unmap);
  view->destroy.notify = xdg_surface_destroy;
  wl_signal_add(&xdg_surface->events.destroy, &view->destroy);

  /* cotd */
  struct wlr_xdg_toplevel *toplevel = xdg_surface->toplevel;
  view->request_move.notify = xdg_toplevel_request_move;
  wl_signal_add(&toplevel->events.request_move, &view->request_move);
  view->request_resize.notify = xdg_toplevel_request_resize;
  wl_signal_add(&toplevel->events.request_resize, &view->request_resize);

  /* Add it to the list of views. */
  wl_list_insert(&server->views, &view->link);
}

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

  // WIP
  wl_list_init(&outputs);
  new_output.notify = server_new_output;
  wl_signal_add(&backend->events.new_output, &new_output);

  // WIP
  wl_list_init(&views);
  xdg_shell = wlr_xdg_shell_create(display);
  new_xdg_surface.notify = server_new_xdg_surface;
  wl_signal_add(&xdg_shell->events.new_surface, &new_xdg_surface);

  // TODO Cursor
  // cursor = wlr_cursor_create();
  // wlr_cursor_attach_output_layout(cursor, output_layout);

  // cursor_mgr = wlr_xcursor_manager_create(NULL, 24);
  // wlr_xcursor_manager_load(cursor_mgr, 1);

  // cursor_motion.notify = server_cursor_motion;
  // wl_signal_add(&cursor->events.motion, &cursor_motion);
  // cursor_motion_absolute.notify = server_cursor_motion_absolute;
  // wl_signal_add(&cursor->events.motion_absolute, &cursor_motion_absolute);
  // cursor_button.notify = server_cursor_button;
  // wl_signal_add(&cursor->events.button, &cursor_button);
  // cursor_axis.notify = server_cursor_axis;
  // wl_signal_add(&cursor->events.axis, &cursor_axis);

  // TODO Seat
  // wl_list_init(&keyboards);
  // new_input.notify = server_new_input;
  // wl_signal_add(&backend->events.new_input, &new_input);
  // seat = wlr_seat_create(display, "seat0");
  // request_cursor.notify = seat_request_cursor;
  // wl_signal_add(&seat->events.request_set_cursor, &request_cursor);

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
  // if (startup_cmd) {
  if (fork() == 0)
    {
      execl("/bin/sh", "/bin/sh", "-c", "gnome-terminal", nullptr);
    }
  // }
  wlr_log(WLR_INFO, "Running Wayland compositor on WAYLAND_DISPLAY=%s",
	  socket);
  wl_display_run(display);
}

Server::~Server()
{
  wl_display_destroy_clients(display);
  wl_display_destroy(display);
}
