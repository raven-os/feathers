#include "wm/WindowTree.hpp"
#include <iostream>

namespace wm
{
  WindowTree::WindowTree(WindowData &&screen)
    : freeList(nullNode)
  {
    nodes.emplace_back(WindowNode{nullNode, nullNode, nullNode, std::move(screen)});
  }

  WindowNodeIndex WindowTree::allocateIndex()
  {
    if (freeList == nullNode)
      {
	WindowNodeIndex result(static_cast<uint16_t>(nodes.size()));

	nodes.emplace_back();
	return result;
      }
    else
      {
	WindowNodeIndex result(freeList);

	freeList = getSibling(freeList);
	return result;
      }
  }

  void WindowTree::removeIndex(WindowNodeIndex index) noexcept
  {
    WindowNodeIndex child(getFirstChild(getParent(index)));

    if (child == index)
      getNode(getParent(index)).firstChild = getSibling(index);
    else
      {
	while (getSibling(child) != index)
	  child = getSibling(child);
	getNode(child).nextSibling = getSibling(index);
      }
    getNode(index).nextSibling = freeList;
    freeList = index;
  }

  WindowNodeIndex WindowTree::addChild(WindowNodeIndex parent)
  {
    WindowNodeIndex result(allocateIndex());

    getNode(result).parent = parent;
    getNode(result).nextSibling = getFirstChild(parent);
    getNode(result).firstChild = nullNode;
    getNode(parent).firstChild = result;
    return result;
  }

  WindowNodeIndex WindowTree::addChildAfter(WindowNodeIndex parent, WindowNodeIndex index)
  {
    WindowNodeIndex result(allocateIndex());

    getNode(result).parent = parent;
    getNode(result).nextSibling = getNode(index).nextSibling;
    getNode(result).firstChild = nullNode;
    getNode(index).nextSibling = result;
    return result;
  }

  void WindowTree::dump()
  {
    struct rec : public WindowTree
    {
      void func(std::string prefix, WindowNodeIndex index)
      {
	std::cout << prefix << "* " << index.data << std::endl;
	prefix += " ";
	for(auto child: getChildren(index))
	  {
	    func(prefix, child);
	  }
      }
    };
    static_cast<rec *>(this)->func(std::string(""), getRootIndex());
  }
}
