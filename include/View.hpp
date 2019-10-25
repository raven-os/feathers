#pragma once

#include <array>
#include <vector>
#include <memory>

# include "Wlroots.hpp"
# include "ServerCursor.hpp"
# include "Popup.hpp"
# include "wm/WindowNodeIndex.hpp"
# include "util/FixedPoint.hpp"

namespace wm
{
  struct WindowTree;
}

class Server;

struct ViewListeners
{
  wl_listener new_popup;
  wl_listener map;
  wl_listener unmap;
  wl_listener destroy;
  wl_listener request_move;
  wl_listener request_resize;
  wl_listener request_fullscreen;
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
  View(wlr_surface *surface);
  ~View();

  template<SurfaceType surfaceType>
  void xdg_surface_map(wl_listener *listener, void *data);

  template<SurfaceType surfaceType>
  void xdg_surface_unmap(wl_listener *listener, void *data);

  template<SurfaceType surfaceType>
  void xdg_toplevel_request_move(wl_listener *listener, void *data);

  template<SurfaceType surfaceType>
  void xdg_toplevel_request_resize(wl_listener *listener, void *data);

  template<SurfaceType surfaceType>
  void xdg_toplevel_request_fullscreen(wl_listener *listener, void *data);

  template<SurfaceType surfaceType>
  void xdg_handle_new_popup(wl_listener *listenr, void *data);

  template<SurfaceType surfaceType>
  void set_tiled(uint32_t edges);

  void set_tiled(uint32_t edges);

  void requestFullscreen();
  void close();
  void focus_view();

  static View *desktop_view_at(double lx, double ly,
			wlr_surface **surface, double *sx, double *sy);

  wlr_output *getWlrOutput();

  wlr_surface *surface;
  bool mapped;
  FixedPoint<-4, int> x, y;
  std::unique_ptr<Popup> popup;

  std::array<int, 2u> previous_size;
  // while this is null the window is floating
  wm::WindowNodeIndex windowNode{wm::nullNode};
  bool fullscreen{false};

  void resize(wm::WindowNodeIndex, wm::WindowTree &, std::array<uint16_t, 2u> size);
  void resize(std::array<uint16_t, 2u> size);
  void move(wm::WindowNodeIndex, wm::WindowTree &, std::array<FixedPoint<-4, int32_t>, 2u> position);
  void move(std::array<FixedPoint<-4, int32_t>, 2u> position);
  std::array<FixedPoint<-4, int32_t>, 2u> getPosition() const noexcept;
  std::array<uint16_t, 2u> getSize() const noexcept;
  std::array<FixedPoint<-4, int32_t>, 2u> getMinSize() const noexcept;
  std::array<FixedPoint<-4, int32_t>, 2u> getMinSize(wm::WindowNodeIndex, wm::WindowTree &) const noexcept;

private:
  void begin_interactive(CursorMode mode, uint32_t edges);
  bool at(double lx, double ly, wlr_surface **surface, double *sx, double *sy);
};
