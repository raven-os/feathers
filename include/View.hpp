#pragma once

#include <array>
#include <vector>
#include <memory>

# include "Wlroots.hpp"
# include "ServerCursor.hpp"
# include "Popup.hpp"
# include "wm/WindowNodeIndex.hpp"

class Server;

struct ViewListeners
{
  struct wl_listener new_popup;
  struct wl_listener map;
  struct wl_listener unmap;
  struct wl_listener destroy;
  struct wl_listener request_move;
  struct wl_listener request_resize;
  struct wl_listener request_fullscreen;
};

class View : public ViewListeners
{
public:
  View(Server *server, struct wlr_xdg_surface_v6 *xdg_surface);
  ~View();

  void xdg_surface_map(struct wl_listener *listener, void *data);
  void xdg_surface_unmap(struct wl_listener *listener, void *data);
  void xdg_toplevel_request_move(struct wl_listener *listener, void *data);
  void xdg_toplevel_request_resize(struct wl_listener *listener, void *data);
  void xdg_toplevel_request_fullscreen(struct wl_listener *listener, void *data);
  void xdg_handle_new_popup(struct wl_listener *listenr, void *data);

  void close();
  void focus_view();

  static View *desktop_view_at(Server *server, double lx, double ly,
			struct wlr_surface **surface, double *sx, double *sy);

  struct wlr_output *getOutput();

  Server *server;
  struct wlr_xdg_surface_v6 *xdg_surface;
  bool mapped;
  int x, y;
  std::unique_ptr<Popup> popup;

  std::array<int16_t, 2u> previous_size;
  // while this is null the window is floating
  wm::WindowNodeIndex windowNode{wm::nullNode};

private:
  void begin_interactive(CursorMode mode, uint32_t edges);
  bool at(double lx, double ly, struct wlr_surface **surface, double *sx, double *sy);

};

namespace ServerView
{
  bool view_at(View *view, double lx, double ly,
	       struct wlr_surface **surface, double *sx, double *sy);
  View *desktop_view_at(Server *server, double lx, double ly,
			struct wlr_surface **surface, double *sx, double *sy);
}
