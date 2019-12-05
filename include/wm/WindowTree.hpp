#pragma once

#include <deque>
#include <iterator>
#include <cassert>

#include "wm/WindowNodeIndex.hpp"
#include "wm/WindowData.hpp"

namespace wm
{
  class WindowTree
  {
  private:
    struct WindowNode
    {
      WindowNodeIndex parent;
      WindowNodeIndex firstChild;
      WindowNodeIndex nextSibling;
      WindowData data;
    };

    WindowNodeIndex freeList;
    std::deque<WindowNode> nodes;

    WindowNode &getNode(WindowNodeIndex nodeIndex) noexcept
    {
      assert(nodeIndex != nullNode);
      return nodes[nodeIndex.data];
    }

    WindowNode const &getNode(WindowNodeIndex nodeIndex) const noexcept
    {
      return const_cast<WindowTree &>(*this).getNode(nodeIndex);
    }

  public:
    WindowTree() = delete;
    WindowTree(WindowTree &&) = delete;
    WindowTree(WindowTree const &) = delete;

    WindowTree(WindowData &&screen);

    uint16_t getWindowCountUpperBound() const noexcept
    {
      return static_cast<uint16_t>(nodes.size());
    }

    WindowNodeIndex getRootIndex() const noexcept
    {
      return WindowNodeIndex{0};
    }

    struct Iterator
    {
      WindowTree const *windowTree;
      WindowNodeIndex nodeIndex;

      Iterator &operator++() noexcept
      {
	nodeIndex = windowTree->getSibling(nodeIndex);
	return *this;
      }

      constexpr bool operator==(Iterator const &other) const noexcept
      {
	return nodeIndex == other.nodeIndex && windowTree == other.windowTree;
      }

      constexpr bool operator!=(Iterator const &other) const noexcept
      {
	return !(*this == other);
      }

      constexpr WindowNodeIndex operator*() const noexcept
      {
	return nodeIndex;
      }
    };

    WindowNodeIndex getParent(WindowNodeIndex nodeIndex) const noexcept
    {
      return getNode(nodeIndex).parent;
    }

    WindowNodeIndex getFirstChild(WindowNodeIndex nodeIndex) const noexcept
    {
      return getNode(nodeIndex).firstChild;
    }

    WindowNodeIndex getSibling(WindowNodeIndex nodeIndex) const noexcept
    {
      return getNode(nodeIndex).nextSibling;
    }

    WindowNodeIndex getPrevSibling(WindowNodeIndex nodeIndex) const noexcept
    {
      WindowNodeIndex result = getFirstChild(getParent(nodeIndex));

      while (getSibling(result) != nodeIndex)
	result = getSibling(result);
      return result;
    }

    struct IteratorPair
    {
      Iterator _begin;
      Iterator _end;

      constexpr auto begin() const noexcept
      {
	return _begin;
      }

      constexpr auto end() const noexcept
      {
	return _end;
      }
    };

    void exchangeChildren(WindowNodeIndex first, WindowNodeIndex second) noexcept
    {
      std::swap(getNode(first).firstChild, getNode(second).firstChild);
    }

    IteratorPair getChildren(WindowNodeIndex nodeIndex) const noexcept
    {
      return {Iterator{this, getFirstChild(nodeIndex)}, Iterator{this, nullNode}};
    }

    WindowData &getData(WindowNodeIndex nodeIndex) noexcept
    {
      return getNode(nodeIndex).data;
    }

    WindowData const &getData(WindowNodeIndex nodeIndex) const noexcept
    {
      return const_cast<WindowTree &>(*this).getData(nodeIndex);
    }

    WindowNodeIndex allocateIndex();

    // sets index to null node so that you don't forget to :)
    void removeIndex(WindowNodeIndex &index) noexcept;

    WindowNodeIndex addChild(WindowNodeIndex parent);
    WindowNodeIndex addChildAfter(WindowNodeIndex parent, WindowNodeIndex index);
    void dump();
  };
}
