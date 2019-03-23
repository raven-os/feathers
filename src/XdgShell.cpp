#include "XdgShell.hpp"
#include "Server.hpp"

XdgShell::XdgShell(Server *server, struct wl_display *display) : server(server) {
  xdg_shell = wlr_xdg_shell_create(display);
  SET_LISTENER(XdgShell, XdgShellListeners, new_xdg_surface, server_new_xdg_surface);
  wl_signal_add(&xdg_shell->events.new_surface, &new_xdg_surface);
}

void XdgShell::xdg_surface_map([[maybe_unused]]struct wl_listener *listener, [[maybe_unused]]void *data)
{
  view->mapped = true;
  ServerView::focus_view(view, view->xdg_surface->surface);
};

void XdgShell::xdg_surface_unmap([[maybe_unused]]struct wl_listener *listener, [[maybe_unused]]void *data)
{
  view->mapped = false;
};

void XdgShell::xdg_surface_destroy([[maybe_unused]]struct wl_listener *listener, [[maybe_unused]]void *data)
{
  wl_list_remove(&view->link);
  // TODO: Maybe delete the view here ?
};

void XdgShell::xdg_toplevel_request_move([[maybe_unused]]struct wl_listener *listener, [[maybe_unused]]void *data)
{
  ServerView::begin_interactive(view, CursorMode::CURSOR_MOVE, 0);
};

void XdgShell::xdg_toplevel_request_resize([[maybe_unused]]struct wl_listener *listener, [[maybe_unused]]void *data)
{
  struct wlr_xdg_toplevel_resize_event *event = static_cast<struct wlr_xdg_toplevel_resize_event *>(data);
  ServerView::begin_interactive(view, CursorMode::CURSOR_RESIZE, event->edges);
};

void XdgShell::server_new_xdg_surface([[maybe_unused]]struct wl_listener *listener, [[maybe_unused]]void *data)
{
  struct wlr_xdg_surface *xdg_surface = static_cast<struct wlr_xdg_surface *>(data);

  if (xdg_surface->role != WLR_XDG_SURFACE_ROLE_TOPLEVEL)
    {
      return;
    }
  view = new View(server, xdg_surface);

  view->setListeners();
  wl_list_insert(&server->views, &view->link);
};
