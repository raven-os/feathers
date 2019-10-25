#pragma once

# include "Wlroots.hpp"

# include "View.hpp"
# include "Listeners.hpp"

class Server;

struct XdgShellListeners
{
  wl_listener new_popup;
  wl_listener new_xdg_surface;
};

class XdgShell : public XdgShellListeners
{
public:
  XdgShell();
  ~XdgShell() = default;

  void xdg_surface_destroy(wl_listener *listener, void *data);
  void server_new_xdg_surface(wl_listener *listener, void *data);
  void xdg_handle_new_popup(wl_listener *listenr, void *data);

private:
  wlr_xdg_shell *xdg_shell;
};
