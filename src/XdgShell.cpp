#include "XdgShell.hpp"
#include "Server.hpp"

XdgShell::XdgShell(Server *server, struct wl_display *display) : server(server) {
  xdg_shell = wlr_xdg_shell_create(display);
  SET_LISTENER(XdgShell, XdgShellListeners, new_xdg_surface, server_new_xdg_surface);
  wl_signal_add(&xdg_shell->events.new_surface, &new_xdg_surface);
}

void XdgShell::xdg_surface_map(struct wl_listener *listener, void *data)
{
  view->mapped = true;
  ServerView::focus_view(view, view->xdg_surface->surface);
};

void XdgShell::xdg_surface_unmap(struct wl_listener *listener, void *data)
{
  view->mapped = false;
};

void XdgShell::xdg_surface_destroy(struct wl_listener *listener, void *data)
{
  wl_list_remove(&view->link);
  //Maybe delete the view here ?
};

void XdgShell::xdg_toplevel_request_move(struct wl_listener *listener, void *data)
{
  ServerView::begin_interactive(view, CursorMode::CURSOR_MOVE, 0);
};

void XdgShell::xdg_toplevel_request_resize(struct wl_listener *listener, void *data)
{
  struct wlr_xdg_toplevel_resize_event *event = static_cast<struct wlr_xdg_toplevel_resize_event *>(data);
  ServerView::begin_interactive(view, CursorMode::CURSOR_RESIZE, event->edges);
};

void XdgShell::server_new_xdg_surface(struct wl_listener *listener, void *data)
{
  struct wlr_xdg_surface *xdg_surface = static_cast<struct wlr_xdg_surface *>(data);
  if (xdg_surface->role != WLR_XDG_SURFACE_ROLE_TOPLEVEL)
    {
      return;
    }

  view = new View(server, xdg_surface);

  view->map.notify = xdg_surface_map;
  wl_signal_add(&xdg_surface->events.map, &view->map);
  view->unmap.notify = xdg_surface_unmap;
  wl_signal_add(&xdg_surface->events.unmap, &view->unmap);
  view->destroy.notify = xdg_surface_destroy;
  wl_signal_add(&xdg_surface->events.destroy, &view->destroy);

  struct wlr_xdg_toplevel *toplevel = xdg_surface->toplevel;
  view->request_move.notify = xdg_toplevel_request_move;
  wl_signal_add(&toplevel->events.request_move, &view->request_move);
  view->request_resize.notify = xdg_toplevel_request_resize;
  wl_signal_add(&toplevel->events.request_resize, &view->request_resize);

  wl_list_insert(&server->views, &view->link);
};
