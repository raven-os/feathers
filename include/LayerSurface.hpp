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

struct LayerSurfaceListeners
{
  struct wl_listener map;
  struct wl_listener unmap;
  // struct wl_listener set_anchor;
  struct wl_listener destroy;
  struct wl_listener new_popup;
};

class LayerSurface : public LayerSurfaceListeners
{
public:
  LayerSurface(struct wlr_surface *surface) noexcept;
  ~LayerSurface() noexcept;

  void shell_surface_map(struct wl_listener *listenr, void *data);
  void shell_surface_unmap(struct wl_listener *listenr, void *data);
  void shell_surface_new_popup(struct wl_listener *listenr, void *data);

  void requestFullscreen();
  void close();

  static LayerSurface *desktop_view_at(double lx, double ly,
				       struct wlr_surface **surface, double *sx, double *sy);

  struct wlr_output *getWlrOutput();

  struct wlr_surface *surface;
  FixedPoint<-4, int> x, y;
  std::unique_ptr<Popup> popup;

  void resize(wm::WindowNodeIndex, wm::WindowTree &, std::array<uint16_t, 2u> size);
  void resize(std::array<uint16_t, 2u> size);
  void move(wm::WindowNodeIndex, wm::WindowTree &, std::array<FixedPoint<-4, int32_t>, 2u> position);
  void move(std::array<FixedPoint<-4, int32_t>, 2u> position);
  std::array<FixedPoint<-4, int32_t>, 2u> getPosition() const noexcept;
  std::array<uint16_t, 2u> getSize() const noexcept;

private:
  void begin_interactive(CursorMode mode, uint32_t edges);
  bool at(double lx, double ly, struct wlr_surface **surface, double *sx, double *sy);
};
