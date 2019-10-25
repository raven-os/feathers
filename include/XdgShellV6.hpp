#pragma once

# include "Wlroots.hpp"

// Rename fields which use reserved names
#define class class_
#define namespace namespace_
#define delete delete_
#define static
// wlroots does not do this itself

extern "C" {

# include <wlr/types/wlr_xdg_shell_v6.h>

}

// make sure to undefine these again
#undef class
#undef namespace
#undef static
#undef delete

# include "View.hpp"
# include "Listeners.hpp"

class Server;

struct XdgShellV6Listeners
{
  wl_listener new_popup;
  wl_listener new_xdg_surface;
};

class XdgShellV6 : public XdgShellV6Listeners
{
public:
  XdgShellV6();
  ~XdgShellV6() = default;

  void xdg_surface_destroy(wl_listener *listener, void *data);
  void server_new_xdg_surface(wl_listener *listener, void *data);
  void xdg_handle_new_popup(wl_listener *listenr, void *data);

private:
  struct wlr_xdg_shell_v6 *xdg_shell;
};
