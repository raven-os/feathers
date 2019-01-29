#ifndef XDGSHELL_HPP_
# define XDGSHELL_HPP_

# include "Wlroots.hpp"

class Server;

class XdgShell
{
public:
  XdgShell(struct wl_display *display, Server *server);
  ~XdgShell();

  Server *server;
  struct wlr_xdg_shell *xdg_shell;
  struct wl_listener new_xdg_surface;
};

#endif /* !XDGSHELL_HPP_ */
