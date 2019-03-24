#pragma once

# include "Wlroots.hpp"
# include "ServerCursor.hpp"

class Server;

struct ViewListeners
{
  struct wl_listener map;
  struct wl_listener unmap;
  struct wl_listener destroy;
  struct wl_listener request_move;
  struct wl_listener request_resize;
};

struct View : public ViewListeners
{
public:
  View(Server *server, struct wlr_xdg_surface_v6 *xdg_surface);
  ~View() = default;

  struct wl_list link;

  void setListeners();

  Server *server;
  struct wlr_xdg_surface_v6 *xdg_surface;
  bool mapped;
  int x, y;
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
