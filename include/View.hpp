#pragma once

#include <array>
#include <vector>
#include <memory>

# include "Wlroots.hpp"
# include "ServerCursor.hpp"
# include "Popup.hpp"
# include "wm/WindowNodeIndex.hpp"
# include "util/FixedPoint.hpp"

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

enum class SurfaceType
  {
   xdg,
   xdg_v6,
   unkonwn
  };

class View : public ViewListeners
{
public:
  View(struct wlr_surface *surface);
  ~View();

  template<SurfaceType surfaceType>
  void xdg_surface_map(struct wl_listener *listener, void *data);

  template<SurfaceType surfaceType>
  void xdg_surface_unmap(struct wl_listener *listener, void *data);

  template<SurfaceType surfaceType>
  void xdg_toplevel_request_move(struct wl_listener *listener, void *data);

  template<SurfaceType surfaceType>
  void xdg_toplevel_request_resize(struct wl_listener *listener, void *data);

  template<SurfaceType surfaceType>
  void xdg_toplevel_request_fullscreen(struct wl_listener *listener, void *data);

  template<SurfaceType surfaceType>
  void xdg_handle_new_popup(struct wl_listener *listenr, void *data);

  void requestFullscreen();
  void close();
  void focus_view();

  static View *desktop_view_at(double lx, double ly,
			struct wlr_surface **surface, double *sx, double *sy);

  struct wlr_output *getWlrOutput();

  struct wlr_surface *surface;
  bool mapped;
  FixedPoint<-4, int> x, y;
  std::unique_ptr<Popup> popup;

  std::array<int, 2u> previous_size;
  // while this is null the window is floating
  wm::WindowNodeIndex windowNode{wm::nullNode};
  bool fullscreen{false};

private:
  void begin_interactive(CursorMode mode, uint32_t edges);
  bool at(double lx, double ly, struct wlr_surface **surface, double *sx, double *sy);
};
