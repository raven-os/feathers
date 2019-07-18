#include "wm/WindowData.hpp"
#include "Server.hpp"

namespace wm
{
  void WindowData::resize(WindowNodeIndex index, WindowTree &windowTree, std::array<uint16_t, 2u> size)
  {
    std::visit([&](auto &data)
	       {
		 data.resize(index, windowTree, size);
	       }, data);
  }

  void WindowData::move(WindowNodeIndex index, WindowTree &windowTree, std::array<FixedPoint<-4, int32_t>, 2u> position)
  {
    std::visit([&](auto &data)
	       {
		 data.move(index, windowTree, position);
	       }, data);
  }

  std::array<FixedPoint<-4, int32_t>, 2u> WindowData::getPosition() const noexcept
  {
    return std::visit([](auto &data) noexcept
		      {
			return data.getPosition();
		      }, data);
  }

  std::array<uint16_t, 2u> WindowData::getSize() const noexcept
  {
    return std::visit([](auto &data) noexcept
		      {
			return data.getSize();
		      }, data);
  }

  void ClientData::resize(WindowNodeIndex, WindowTree &, std::array<uint16_t, 2u> size)
  {
    if (!view->fullscreen)
      wlr_xdg_toplevel_v6_set_size(view->xdg_surface, size[0], size[1]);
    else
      {
	auto &output(Server::getInstance().output.getOutput(view->getOutput()));

        output.saved.width = size[0];
	output.saved.height = size[1];
      }
  }

  void ClientData::move(WindowNodeIndex, WindowTree &, std::array<FixedPoint<-4, int32_t>, 2u> position)
  {
    struct wlr_box box[1];
    wlr_xdg_surface_v6_get_geometry(view->xdg_surface, box);

    (view->x = position[0]) -= FixedPoint<0, int32_t>(box->x);
    (view->y = position[1]) -= FixedPoint<0, int32_t>(box->y);
  }

  std::array<FixedPoint<-4, int32_t>, 2u> ClientData::getPosition() const noexcept
  {
    struct wlr_box box[1];
    wlr_xdg_surface_v6_get_geometry(view->xdg_surface, box);

    std::array<FixedPoint<-4, int32_t>, 2u> result{{view->x, view->y}};

    result[0] += FixedPoint<0, int32_t>(box->x);
    result[1] += FixedPoint<0, int32_t>(box->y);
    return result;
  }

  std::array<uint16_t, 2u> ClientData::getSize() const noexcept
  {
    struct wlr_box box[1];
    wlr_xdg_surface_v6_get_geometry(view->xdg_surface, box);

    return {box->width, box->height};
  }

  Container &WindowData::getContainer() noexcept
  {
    return std::get<Container>(data);
  }

  Container const &WindowData::getContainer() const noexcept
  {
    return std::get<Container>(data);
  }
}
