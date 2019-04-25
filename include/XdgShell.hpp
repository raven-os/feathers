#pragma once

# include "Wlroots.hpp"
# include "View.hpp"
# include "Listeners.hpp"

class Server;

struct XdgShellListeners
{
  struct wl_listener new_xdg_surface;
};

class XdgShell : public XdgShellListeners
{
public:
  XdgShell(Server *server);
  ~XdgShell() = default;

  void xdg_surface_destroy(struct wl_listener *listener, void *data);
  void server_new_xdg_surface(struct wl_listener *listener, void *data);

private:
  Server *server;
  struct wlr_xdg_shell_v6 *xdg_shell;
};
