#include "wm/WindowData.hpp"
#include "Server.hpp"

namespace wm
{
  WindowData::WindowData(Container &&container)
    : data(std::in_place_type_t<std::unique_ptr<Container>>{}, new Container(std::move(container)))
  {
  }

  WindowData::WindowData(View *view)
    : data{view}
  {
  }

  void WindowData::resize(WindowNodeIndex index, WindowTree &windowTree, std::array<uint16_t, 2u> size)
  {
    std::visit([&](auto &data)
	       {
		 data->resize(index, windowTree, size);
	       }, data);
  }

  void WindowData::move(WindowNodeIndex index, WindowTree &windowTree, std::array<FixedPoint<-4, int32_t>, 2u> position)
  {
    std::visit([&](auto &data)
	       {
		 data->move(index, windowTree, position);
	       }, data);
  }

  std::array<FixedPoint<-4, int32_t>, 2u> WindowData::getPosition() const noexcept
  {
    return std::visit([](auto &data) noexcept
		      {
			return data->getPosition();
		      }, data);
  }

  std::array<uint16_t, 2u> WindowData::getSize() const noexcept
  {
    return std::visit([](auto &data) noexcept
		      {
			return data->getSize();
		      }, data);
  }

  std::array<FixedPoint<-4, int32_t>, 2u> WindowData::getMinSize(WindowNodeIndex index, WindowTree &windowTree) const noexcept
  {
    return std::visit([&](auto &data) noexcept
		      {
			return data->getMinSize(index, windowTree);
		      }, data);
  }

  Container &WindowData::getContainer() noexcept
  {
    return *std::get<std::unique_ptr<Container>>(data);
  }

  Container const &WindowData::getContainer() const noexcept
  {
    return *std::get<std::unique_ptr<Container>>(data);
  }
}
