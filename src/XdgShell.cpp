#include "XdgShell.hpp"
#include "Server.hpp"

XdgShell::XdgShell(Server *server) : server(server) {
  xdg_shell = wlr_xdg_shell_v6_create(server->display);
  SET_LISTENER(XdgShell, XdgShellListeners, new_xdg_surface, server_new_xdg_surface);
  wl_signal_add(&xdg_shell->events.new_surface, &new_xdg_surface);
}

void XdgShell::xdg_surface_map([[maybe_unused]]struct wl_listener *listener, [[maybe_unused]]void *data)
{
  View *view = wl_container_of(listener, view, map);
  view->mapped = true;
  ServerView::focus_view(view, view->xdg_surface->surface);
};

void XdgShell::xdg_surface_unmap([[maybe_unused]]struct wl_listener *listener, [[maybe_unused]]void *data)
{
  View *view = wl_container_of(listener, view, unmap);
  view->mapped = false;
};

void XdgShell::xdg_surface_destroy([[maybe_unused]]struct wl_listener *listener, [[maybe_unused]]void *data)
{
  View *view = wl_container_of(listener, view, destroy);
  wl_list_remove(&view->link);
  delete view;
};

void XdgShell::xdg_toplevel_request_move([[maybe_unused]]struct wl_listener *listener, [[maybe_unused]]void *data)
{
  View *view = wl_container_of(listener, view, request_move);
  ServerView::begin_interactive(view, CursorMode::CURSOR_MOVE, 0);
};

void XdgShell::xdg_toplevel_request_resize([[maybe_unused]]struct wl_listener *listener, [[maybe_unused]]void *data)
{
  View *view = wl_container_of(listener, view, request_resize);
  struct wlr_xdg_toplevel_v6_resize_event *event = static_cast<struct wlr_xdg_toplevel_v6_resize_event *>(data);
  ServerView::begin_interactive(view, CursorMode::CURSOR_RESIZE, event->edges);
};

void XdgShell::server_new_xdg_surface([[maybe_unused]]struct wl_listener *listener, [[maybe_unused]]void *data)
{
  struct wlr_xdg_surface_v6 *xdg_surface = static_cast<struct wlr_xdg_surface_v6 *>(data);

  if (xdg_surface->role != WLR_XDG_SURFACE_V6_ROLE_TOPLEVEL)
    {
      return;
    }
  View *view = new View(server, xdg_surface);

  view->setListeners();
  wl_list_insert(&server->views, &view->link);
};
