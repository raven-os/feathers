#include "wm/Container.hpp"
#include "wm/WindowTree.hpp"

#include <iostream>

/*
** ::: ::: :::       :::     :::     :::::::::  ::::    ::: ::::::::::: ::::    :::  ::::::::  ::: :::
** :+: :+: :+:       :+:   :+: :+:   :+:    :+: :+:+:   :+:     :+:     :+:+:   :+: :+:    :+: :+: :+:
** +:+ +:+ +:+       +:+  +:+   +:+  +:+    +:+ :+:+:+  +:+     +:+     :+:+:+  +:+ +:+        +:+ +:+
** +#+ +#+ +#+  +:+  +#+ +#++:++#++: +#++:++#:  +#+ +:+ +#+     +#+     +#+ +:+ +#+ :#:        +#+ +#+
** +#+ +#+ +#+ +#+#+ +#+ +#+     +#+ +#+    +#+ +#+  +#+#+#     +#+     +#+  +#+#+# +#+   +#+# +#+ +#+
**          #+#+# #+#+#  #+#     #+# #+#    #+# #+#   #+#+#     #+#     #+#   #+#+# #+#    #+#
** ### ###   ###   ###   ###     ### ###    ### ###    #### ########### ###    ####  ########  ### ###
**
** gcc's -Wconversion is disabled beyond this point, please pay careful attention to any type-conversions
** This was done because uint16_t's operator += triggers it
*/

// please don't include anything below this point
#pragma GCC diagnostic ignored "-Wconversion"

namespace wm
{
  Container::Container(Rect const &rect, bool direction) noexcept
    : rect(rect)
    , direction(direction)
  {
  }

  FixedPoint<-4, uint32_t> Container::getChildWidth([[maybe_unused]]WindowNodeIndex index, WindowTree &windowTree, WindowNodeIndex childIndex)
  {
    auto &childData(windowTree.getData(childIndex));
    auto sibling(windowTree.getSibling(childIndex));

    if (sibling == wm::nullNode)
      return rect.position[direction] + rect.size[direction] - childData.getPosition()[direction];
    else
      return (windowTree.getData(sibling)).getPosition()[direction] - childData.getPosition()[direction];
  }

  void Container::updateChildWidths(WindowNodeIndex index, WindowTree &windowTree)
  {
    for (auto childIndex : windowTree.getChildren(index))
      {
	auto &childData(windowTree.getData(childIndex));
	std::array<uint16_t, 2u> newSize;

	newSize[direction] = FixedPoint<0, uint32_t>(getChildWidth(index, windowTree, childIndex)).value;
	newSize[!direction] = FixedPoint<0, uint32_t>(rect.size[!direction]).value;
	childData.resize(childIndex, windowTree, newSize);
      }
  }

  void Container::move_impl(WindowNodeIndex index, WindowTree &windowTree, std::array<FixedPoint<-4, int32_t>, 2u> position)
  {
    return move_after_impl(windowTree, windowTree.getFirstChild(index), position);
  }

  void Container::move_after_impl(WindowTree &windowTree, WindowNodeIndex start, std::array<FixedPoint<-4, int32_t>, 2u> position)
  {
    for (auto childIndex(start); childIndex != wm::nullNode; childIndex = windowTree.getSibling(childIndex))
      {
	auto &childData(windowTree.getData(childIndex));
	auto newPosition(childData.getPosition());

	for (size_t i(0u); i != childData.getPosition().size(); ++i)
	  {
	    newPosition[i] += position[i] - rect.position[i];
	  }
	childData.move(childIndex, windowTree, newPosition);
      }
  }

  void Container::move(WindowNodeIndex index, WindowTree &windowTree, std::array<FixedPoint<-4, int32_t>, 2u> position)
  {
    move_impl(index, windowTree, position);
    rect.position = position;
  }

  void Container::resize_impl(WindowNodeIndex index, WindowTree &windowTree, std::array<FixedPoint<-4, uint32_t>, 2u> size)
  {
    auto children(windowTree.getChildren(index));
    for (auto childIndex : children)
      {
	auto &childData(windowTree.getData(childIndex));
	auto position(childData.getPosition());

	position[direction] -= rect.position[direction];
	position[direction] = (position[direction] * size[direction]) / (rect.size[direction]);
	position[direction] += rect.position[direction];

	childData.move(childIndex, windowTree, position);
      }
  }

  void Container::resize(WindowNodeIndex index, WindowTree &windowTree, std::array<uint16_t, 2u> size)
  {
    std::array<FixedPoint<-4, uint32_t>, 2u> newSize;

    for (int i = 0; i < 2; ++i)
      newSize[i] = makeFixedPoint(size[i]);
    resize(index, windowTree, newSize);
  }

  void Container::resize(WindowNodeIndex index, WindowTree &windowTree, std::array<FixedPoint<-4, uint32_t>, 2u> size)
  {
    resize_impl(index, windowTree, size);
    rect.size = size;
    updateChildWidths(index, windowTree);
  }

  WindowNodeIndex Container::addChild(WindowNodeIndex index, WindowTree &windowTree, ClientData &&newChildWindowData)
  {
    {
      auto children{windowTree.getChildren(index)};
      uint16_t count(0u);

      for ([[maybe_unused]]auto _ : children)
	++count;
      {
        auto position{rect.position};
	auto size{rect.size};

	position[direction] += rect.size[direction] / FixedPoint<0u>(count + 1);
	size[direction] = (size[direction] * FixedPoint<0u>(count)) / FixedPoint<0u>(count + 1);

	resize_impl(index, windowTree, size);
	move_impl(index, windowTree, position);
      }
    }
    {
      auto newChild(windowTree.addChild(index));
      auto &childData(windowTree.getData(newChild));

      childData.data = std::move(newChildWindowData);
      childData.move(newChild, windowTree, rect.position);

      updateChildWidths(index, windowTree);
      return newChild;
    }
  }

  WindowNodeIndex Container::addChild(WindowNodeIndex index, WindowTree &windowTree, WindowNodeIndex prev, ClientData &&newChildWindowData)
  {
    uint16_t count(0u);
    uint16_t offset(0u);

    {
      auto childIndex(windowTree.getFirstChild(index));
      for (; childIndex != prev; childIndex = windowTree.getSibling(childIndex))
	++offset;
      count = offset;
      offset += 1;
      for (; childIndex != wm::nullNode; childIndex = windowTree.getSibling(childIndex))
	++count;
    }
    {
      auto position{rect.position};
      auto size{rect.size};

      position[direction] += rect.size[direction] / FixedPoint<0u>(count + 1);
      size[direction] = (size[direction] * FixedPoint<0u>(count)) / FixedPoint<0u>(count + 1);

      resize_impl(index, windowTree, size);
      move_after_impl(windowTree, windowTree.getSibling(prev), position);
    }
    {
      auto newChild(windowTree.addChildAfter(index, prev));
      auto &childData(windowTree.getData(newChild));

      childData.data = std::move(newChildWindowData);
      {
	auto position{rect.position};

	position[direction] += (FixedPoint<0u>(offset) * rect.size[direction]) / FixedPoint<0u>(count + 1);
	childData.move(newChild, windowTree, position);
      }
      updateChildWidths(index, windowTree);
      return newChild;
    }
  }

  void Container::removeChild(WindowNodeIndex index, WindowTree &windowTree, WindowNodeIndex childIndex)
  {
    auto removedWidth(getChildWidth(index, windowTree, childIndex));
    auto childIndex2{windowTree.getSibling(childIndex)};

    windowTree.removeIndex(childIndex);

    for (; childIndex2 != wm::nullNode; childIndex2 = windowTree.getSibling(childIndex2))
      {
	auto &childData(windowTree.getData(childIndex2));
	auto position(childData.getPosition());

	position[direction] -= removedWidth;
	childData.move(childIndex2, windowTree, position);
      }


    {
      auto size{rect.size};

      rect.size[direction] = size[direction] - removedWidth;
      resize(index, windowTree, size);
    }
    if (windowTree.getFirstChild(index) == wm::nullNode && windowTree.getRootIndex() != index)
      {
	auto parentNode(windowTree.getParent(index));

	windowTree.getData(parentNode).getContainer().removeChild(parentNode, windowTree, index);
      }
  }

  void Container::changeDirection(WindowNodeIndex index, WindowTree &windowTree)
  {
    for (auto child : windowTree.getChildren(index))
      {
	auto &childData(windowTree.getData(child));
	auto position(childData.getPosition());

	position[!direction] = ((position[direction] - rect.position[direction]) * rect.size[!direction]) / rect.size[direction]
	  + rect.position[!direction];
	position[direction] = rect.position[direction];
	childData.move(child, windowTree, position);
      }
    direction ^= 1;
    updateChildWidths(index, windowTree);
  }

  std::array<FixedPoint<-4, int32_t>, 2u> Container::getPosition() const noexcept
  {
    return rect.position;
  }

  std::array<uint16_t, 2u> Container::getSize() const noexcept
  {
    std::array<uint16_t, 2u> size;

    for (uint32_t i = 0; i < size.size(); ++i)
      size[i] = FixedPoint<0, uint32_t>(rect.size[i]).value;
    return size;
  }

  std::array<FixedPoint<-4, int32_t>, 2u> Container::getMinSize(WindowNodeIndex index, WindowTree &windowTree) const noexcept
  {
    std::array<FixedPoint<-4, int32_t>, 2u> result{0_FP, 0_FP};

    for (auto child : windowTree.getChildren(index))
      {
	auto &childData(windowTree.getData(child));
	auto childMinSize(childData.getMinSize(child, windowTree));

	result[direction] += childMinSize[direction];
	result[!direction] = std::max(childMinSize[!direction], result[!direction]);
      }
    return result;
  }


}
