#pragma once

# include "Wlroots.hpp"
# include "ServerCursor.hpp"

class Server;

class View : public Listeners::ViewListeners
{
public:
  View(Server *server, struct wlr_xdg_surface *xdg_surface);
  ~View() = default;

  struct wl_list link;

  //void setMapListeners(void (*func)(struct wl_listener *listener, void *data));

  Server *server;
  struct wlr_xdg_surface *xdg_surface;
  bool mapped;
  int x, y;
private:
};

namespace ServerView
{
  void focus_view(View *view, struct wlr_surface *surface);
  bool view_at(View *view, double lx, double ly,
	       struct wlr_surface **surface, double *sx, double *sy);
  View *desktop_view_at(Server *server, double lx, double ly,
			struct wlr_surface **surface, double *sx, double *sy);
  void begin_interactive(View *view, CursorMode mode, uint32_t edges);
}
