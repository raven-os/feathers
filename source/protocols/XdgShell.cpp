#include <cassert>

#include "protocols/XdgShell.hpp"
#include "XdgView.hpp"
#include "Server.hpp"
#include "wm/Container.hpp"

XdgShell::XdgShell() {
  xdg_shell = wlr_xdg_shell_create(Server::getInstance().getWlDisplay());
  SET_LISTENER(XdgShell, XdgShellListeners, new_xdg_surface, server_new_xdg_surface);
  wl_signal_add(&xdg_shell->events.new_surface, &new_xdg_surface);
}

void XdgShell::xdg_surface_destroy(wl_listener *listener, void *data)
{
  Server &server = Server::getInstance();
  XdgView *view = wl_container_of(listener, view, destroy);

  if (server.getFocusedView() == view)
    {
      server.seat.getSeat()->keyboard_state.focused_surface = nullptr;
    }
  auto &views(view->workspace->getViews());

  views.erase(std::find_if(views.begin(), views.end(),
			   [view](auto const &ptr) noexcept
			   {
			     return ptr.get() == view;
			   }));
  if (XdgView *view = server.getFocusedView())
    view->focus_view();
};

void XdgShell::server_new_xdg_surface(wl_listener *listener, void *data)
{
  wlr_xdg_surface *xdg_surface = static_cast<wlr_xdg_surface *>(data);

  if (xdg_surface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL)
    {
      Workspace *workspace = Server::getInstance().outputManager.getActiveWorkspace();

      workspace->getViews().emplace_back(new XdgView(xdg_surface->surface, workspace));
    }
};
