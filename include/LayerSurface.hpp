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
  wl_listener map;
  wl_listener unmap;
  // struct wl_listener set_anchor;
  wl_listener destroy;
  wl_listener new_popup;
};

class LayerSurface : public LayerSurfaceListeners
{
public:
  LayerSurface(wlr_surface *surface) noexcept;
  ~LayerSurface() noexcept;

  void shell_surface_map(wl_listener *listenr, void *data);
  void shell_surface_unmap(wl_listener *listenr, void *data);
  void shell_surface_new_popup(wl_listener *listenr, void *data);

  void requestFullscreen();
  void close();

  static LayerSurface *desktop_view_at(double lx, double ly,
				       wlr_surface **surface, double *sx, double *sy);

  wlr_output *getWlrOutput();

  wlr_surface *surface;
  FixedPoint<-4, int> x, y;
  std::unique_ptr<Popup> popup;

  void resize(wm::WindowNodeIndex, wm::WindowTree &, std::array<uint16_t, 2u> size);
  void resize(std::array<uint16_t, 2u> size);
  void move(wm::WindowNodeIndex, wm::WindowTree &, std::array<FixedPoint<-4, int32_t>, 2u> position);
  void move(std::array<FixedPoint<-4, int32_t>, 2u> position);
  std::array<FixedPoint<-4, int32_t>, 2u> getPosition() const noexcept;
  std::array<uint16_t, 2u> getSize() const noexcept;

// private:
//   void begin_interactive(CursorMode mode, uint32_t edges);
//   bool at(double lx, double ly, struct wlr_surface **surface, double *sx, double *sy);
};
