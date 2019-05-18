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
  uint16_t Container::getChildWidth(WindowNodeIndex index, WindowTree &windowTree, WindowNodeIndex childIndex)
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
	auto newSize(rect.size);

	newSize[direction] = getChildWidth(index, windowTree, childIndex);
	childData.resize(childIndex, windowTree, newSize);
      }
  }

  void Container::move_impl(WindowNodeIndex index, WindowTree &windowTree, std::array<int16_t, 2u> position)
  {
    return move_after_impl(windowTree, windowTree.getFirstChild(index), position);
  }

  void Container::move_after_impl(WindowTree &windowTree, WindowNodeIndex start, std::array<int16_t, 2u> position)
  {
    for (auto childIndex(start); childIndex != wm::nullNode; childIndex = windowTree.getSibling(childIndex))
      {
	auto &childData(windowTree.getData(childIndex));
	auto newPosition(childData.getPosition());

	for (size_t i(0u); i != childData.getPosition().size(); ++i)
	  {
	    uint16_t offset(uint16_t(position[i] - rect.position[i]));
	    newPosition[i] += offset;
	  }
	childData.move(childIndex, windowTree, newPosition);
      }
  }

  void Container::move(WindowNodeIndex index, WindowTree &windowTree, std::array<int16_t, 2u> position)
  {
    move_impl(index, windowTree, position);
    rect.position = position;
  }

  void Container::resize_impl(WindowNodeIndex index, WindowTree &windowTree, std::array<uint16_t, 2u> size)
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

	position[direction] += rect.size[direction] / (count + 1);
	size[direction] = (size[direction] * count) / (count + 1);

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

      position[direction] += rect.size[direction] / (count + 1);
      size[direction] = (size[direction] * count) / (count + 1);

      resize_impl(index, windowTree, size);
      move_after_impl(windowTree, windowTree.getSibling(prev), position);
    }
    {
      auto newChild(windowTree.addChildAfter(index, prev));
      auto &childData(windowTree.getData(newChild));

      childData.data = std::move(newChildWindowData);
      {
	auto position{rect.position};

	position[direction] += (offset * rect.size[direction]) / (count + 1);
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
  }

  std::array<int16_t, 2u> Container::getPosition() const noexcept
  {
    return rect.position;
  }
  
  std::array<uint16_t, 2u> Container::getSize() const noexcept
  {
    return rect.size;
  }

}
