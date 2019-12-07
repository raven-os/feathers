#pragma once

#include "View.hpp"

struct ViewListeners
{
  wl_listener new_popup;
  wl_listener map;
  wl_listener unmap;
  wl_listener destroy;
  wl_listener request_move;
  wl_listener request_resize;
  wl_listener request_fullscreen;
  wl_listener request_maximize;
};

class XdgView : public View, public ViewListeners
{
public:
  XdgView(wlr_surface *surface, Workspace *workspace) noexcept;
  XdgView(XdgView const &) = delete;
  XdgView(XdgView &&) = delete;
  ~XdgView();

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
  void set_tiled(uint32_t edges);

  void set_tiled(uint32_t edges);

  void requestFullscreen();
  void close();
  void begin_interactive(CursorMode mode, uint32_t edges);

  wlr_output *getWlrOutput();
  
  void resize(wm::WindowNodeIndex, wm::WindowTree &, std::array<uint16_t, 2u> size);
  void resize(std::array<uint16_t, 2u> size);
  void move(wm::WindowNodeIndex, wm::WindowTree &, std::array<FixedPoint<-4, int32_t>, 2u> position);
  void move(std::array<FixedPoint<-4, int32_t>, 2u> position);
  std::array<FixedPoint<-4, int32_t>, 2u> getPosition() const noexcept;
  std::array<uint16_t, 2u> getSize() const noexcept;
  std::array<FixedPoint<-4, int32_t>, 2u> getMinSize() const noexcept;
  std::array<FixedPoint<-4, int32_t>, 2u> getMinSize(wm::WindowNodeIndex, wm::WindowTree &) const noexcept;

  // while the two following fields are null the window is in floating mode
  wm::WindowNodeIndex windowNode{wm::nullNode};
  Workspace *workspace{nullptr};
  bool fullscreen{false};
  std::array<int, 2u> previous_size;

  static XdgView *desktop_view_at(double lx, double ly,
				  wlr_surface **surface, double *sx, double *sy);
};
