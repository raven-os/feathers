#pragma once

# include "Wlroots.hpp"
# include "View.hpp"
# include "Listeners.hpp"

class Server;

struct XdgShellListeners
{
  struct wl_listener new_popup;
  struct wl_listener new_xdg_surface;
};

class XdgShell : public XdgShellListeners
{
public:
  XdgShell();
  ~XdgShell() = default;

  void xdg_surface_destroy(struct wl_listener *listener, void *data);
  void server_new_xdg_surface(struct wl_listener *listener, void *data);
  void xdg_handle_new_popup(struct wl_listener *listenr, void *data);

private:
  struct wlr_xdg_shell_v6 *xdg_shell;
};
