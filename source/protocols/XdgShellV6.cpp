#include <cassert>

#include "protocols/XdgShellV6.hpp"
#include "WindowView.hpp"
#include "Server.hpp"
#include "wm/Container.hpp"

XdgShellV6::XdgShellV6() {
  xdg_shell = wlr_xdg_shell_v6_create(Server::getInstance().getWlDisplay());
  SET_LISTENER(XdgShellV6, XdgShellV6Listeners, new_xdg_surface, server_new_xdg_surface);
  wl_signal_add(&xdg_shell->events.new_surface, &new_xdg_surface);
}

void XdgShellV6::xdg_surface_destroy(wl_listener *listener, void *data)
{
  Server &server = Server::getInstance();
  WindowView *view = wl_container_of(listener, view, destroy);

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
  if (WindowView *view = server.getFocusedView())
    view->focus_view();
};

void XdgShellV6::server_new_xdg_surface(wl_listener *listener, void *data)
{
  wlr_xdg_surface_v6 *xdg_surface = static_cast<wlr_xdg_surface_v6 *>(data);

  if (xdg_surface->role == WLR_XDG_SURFACE_V6_ROLE_TOPLEVEL)
    {
      Workspace *workspace = Server::getInstance().outputManager.getActiveWorkspace();

      workspace->getViews().emplace_back(new WindowView(xdg_surface->surface, workspace));
    }
};
