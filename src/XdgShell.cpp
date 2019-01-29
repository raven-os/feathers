#include "XdgShell.hpp"
#include "Server.hpp"

XdgShell::XdgShell(struct wl_display *display)
  : xdg_shell(wlr_xdg_shell_create(display))
{
  static const auto xdg_surface_map = [](struct wl_listener *listener, void *data)
  {
    /* Called when the surface is mapped, or ready to display on-screen. */
    View *view = wl_container_of(listener, view, map);
    view->mapped = true;
    // TODO
    // focus_view(view, view->xdg_surface->surface);
  };

  static const auto xdg_surface_unmap = [](struct wl_listener *listener, void *data)
  {
    /* Called when the surface is unmapped, and should no longer be shown. */
    View *view = wl_container_of(listener, view, unmap);
    view->mapped = false;
  };

  static const auto xdg_surface_destroy = [](struct wl_listener *listener, void *data)
  {
    /* Called when the surface is destroyed and should never be shown again. */
    View *view = wl_container_of(listener, view, destroy);
    wl_list_remove(&view->link);
    free(view);
  };

  static const auto xdg_toplevel_request_move = [](struct wl_listener *listener, void *data)
  {
    /* This event is raised when a client would like to begin an interactive
     * move, typically because the user clicked on their client-side
     * decorations. Note that a more sophisticated compositor should check the
     * provied serial against a list of button press serials sent to this
     * client, to prevent the client from requesting this whenever they want. */
    View *view = wl_container_of(listener, view, request_move);
    // TODO
    // begin_interactive(view, TINYWL_CURSOR_MOVE, 0);
  };

  static const auto xdg_toplevel_request_resize = [](struct wl_listener *listener, void *data) {
    /* This event is raised when a client would like to begin an interactive
     * resize, typically because the user clicked on their client-side
     * decorations. Note that a more sophisticated compositor should check the
     * provied serial against a list of button press serials sent to this
     * client, to prevent the client from requesting this whenever they want. */
    struct wlr_xdg_toplevel_resize_event *event = static_cast<struct wlr_xdg_toplevel_resize_event *>(data);
    View *view = wl_container_of(listener, view, request_resize);
    // begin_interactive(view, TINYWL_CURSOR_RESIZE, event->edges);
  };

  static const auto server_new_xdg_surface = [](struct wl_listener *listener, void *data)
  {
    /* This event is raised when wlr_xdg_shell receives a new xdg surface from a
     * client, either a toplevel (application window) or popup. */
    Server *server = wl_container_of(listener, server, views);
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
  };

  new_xdg_surface.notify = server_new_xdg_surface;
  wl_signal_add(&xdg_shell->events.new_surface, &new_xdg_surface);
}

XdgShell::~XdgShell()
{

}
