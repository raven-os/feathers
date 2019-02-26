#pragma once

# include "Wlroots.hpp"

class Server;

namespace XdgShell
{
  void xdg_surface_map(struct wl_listener *listener, void *data);
  void xdg_surface_unmap(struct wl_listener *listener, void *data);
  void xdg_surface_destroy(struct wl_listener *listener, void *data);
  void xdg_toplevel_request_move(struct wl_listener *listener, void *data);
  void xdg_toplevel_request_resize(struct wl_listener *listener, void *data);
  void server_new_xdg_surface(struct wl_listener *listener, void *data);
};
