#include "Commands.hpp"
#include "Output.hpp"
#include "WindowView.hpp"

namespace Commands
{
  namespace WindowCommands
  {
    auto getContainerFocusedView(wm::WindowTree &windowTree, wm::WindowNodeIndex containerNode, wm::WindowNodeIndex viewNode)
    {
      Server &server = Server::getInstance();

      for (auto &tmpView : server.getViews())
	{
	  if (tmpView->windowNode != viewNode)
	    {
	      auto tmpContainerNode(windowTree.getParent(tmpView->windowNode));

	      while (tmpContainerNode != windowTree.getRootIndex() && tmpContainerNode != containerNode)
		tmpContainerNode = windowTree.getParent(tmpContainerNode);
	      if (tmpContainerNode == containerNode)
		{
		  return tmpView->windowNode;
		}
	    }
	}
      return wm::nullNode;
    }

    void switch_focus_down_or_right(bool parallelDirection)
    {
      Server &server = Server::getInstance();

      if (auto view = server.getFocusedView())
	{
	  auto viewNode(view->windowNode);
	  auto &output(server.outputManager.getOutput(view->getWlrOutput()));

	  if (view->windowNode == wm::nullNode || output.getFullscreenView())
	    return ;

	  auto &windowTree(server.getActiveWindowTree());
	  auto containerNode(windowTree.getParent(viewNode));
	  auto *container(&windowTree.getData(containerNode).getContainer());

	  if (container->direction == !parallelDirection)
	    {
	      if (containerNode != windowTree.getRootIndex())
		{
		  viewNode = containerNode;
		  containerNode = windowTree.getParent(containerNode);
		  container = &windowTree.getData(containerNode).getContainer();
		}
	      else
		return;
	    }
	  auto newViewNode(windowTree.getSibling(viewNode));

	  if (newViewNode == wm::nullNode && containerNode != windowTree.getRootIndex())
	    {
	      auto tmpNode(containerNode);
	      auto newContainerNode(windowTree.getParent(tmpNode));
	      auto *newContainer(&windowTree.getData(newContainerNode).getContainer());

	      while (newContainerNode != windowTree.getRootIndex() &&
		     (newContainer->direction == !parallelDirection ||
		      windowTree.getSibling(tmpNode) == wm::nullNode))
		{
		  tmpNode = newContainerNode;
		  newContainerNode = windowTree.getParent(tmpNode);
		  newContainer = &windowTree.getData(newContainerNode).getContainer();
		}
	      auto siblingNode(windowTree.getSibling(tmpNode));

	      if (siblingNode == wm::nullNode || newContainer->direction == !parallelDirection)
		return;
	      if (std::holds_alternative<std::unique_ptr<wm::Container>>(windowTree.getData(siblingNode).data))
		newViewNode = getContainerFocusedView(windowTree, siblingNode, viewNode);
	      else
		newViewNode = siblingNode;
	    }
	  else if (newViewNode != wm::nullNode && std::holds_alternative<std::unique_ptr<wm::Container>>(windowTree.getData(newViewNode).data))
	    {
	      newViewNode = getContainerFocusedView(windowTree, newViewNode, viewNode);
	    }
	  if (newViewNode != viewNode && newViewNode != wm::nullNode)
	    {
	      auto &newView(std::get<wm::ClientData>(windowTree.getData(newViewNode).data));

	      newView->focus_view();
	    }
	}
    }

    void switch_focus_up_or_left(bool parallelDirection)
    {
      Server &server = Server::getInstance();

      if (auto view = server.getFocusedView())
	{
	  auto viewNode(view->windowNode);
	  auto &output(server.outputManager.getOutput(view->getWlrOutput()));

	  if (view->windowNode == wm::nullNode || output.getFullscreenView())
	    return ;

	  auto &windowTree(server.getActiveWindowTree());
	  auto containerNode(windowTree.getParent(viewNode));
	  auto *container(&windowTree.getData(containerNode).getContainer());

	  if (container->direction == !parallelDirection)
	    {
	      if (containerNode != windowTree.getRootIndex())
		{
		  viewNode = containerNode;
		  containerNode = windowTree.getParent(containerNode);
		  container = &windowTree.getData(containerNode).getContainer();
		}
	      else
		return;
	    }
	  auto newViewNode(windowTree.getFirstChild(containerNode));

	  if (newViewNode == viewNode && containerNode != windowTree.getRootIndex())
	    {
	      auto tmpNode(containerNode);
	      auto newContainerNode(windowTree.getParent(tmpNode));
	      auto *newContainer(&windowTree.getData(newContainerNode).getContainer());

	      while (newContainerNode != windowTree.getRootIndex() &&
		     (newContainer->direction == !parallelDirection ||
		      windowTree.getFirstChild(newContainerNode) == tmpNode))
		{
		  tmpNode = newContainerNode;
		  newContainerNode = windowTree.getParent(tmpNode);
		  newContainer = &windowTree.getData(newContainerNode).getContainer();
		}
	      auto siblingNode(windowTree.getFirstChild(newContainerNode));

	      if (siblingNode == tmpNode || newContainer->direction == !parallelDirection)
		return;
	      if (std::holds_alternative<std::unique_ptr<wm::Container>>(windowTree.getData(siblingNode).data))
		{
		  while (windowTree.getSibling(siblingNode) != tmpNode && windowTree.getSibling(siblingNode) != wm::nullNode)
		    siblingNode = windowTree.getSibling(siblingNode);
		  newViewNode = getContainerFocusedView(windowTree, siblingNode, viewNode);
		}
	      else
		newViewNode = siblingNode;
	    }
	  else if (newViewNode != viewNode)
	    {
	      auto tmpNode(windowTree.getSibling(newViewNode));

	      while (tmpNode != viewNode && tmpNode != wm::nullNode)
		{
		  newViewNode = tmpNode;
		  tmpNode = windowTree.getSibling(newViewNode);
		}
	      if (std::holds_alternative<std::unique_ptr<wm::Container>>(windowTree.getData(newViewNode).data))
		{
		  while (windowTree.getSibling(newViewNode) != tmpNode && windowTree.getSibling(newViewNode) != wm::nullNode)
		    newViewNode = windowTree.getSibling(newViewNode);
		  newViewNode = getContainerFocusedView(windowTree, newViewNode, viewNode);
		}
	    }
	  if (newViewNode != viewNode)
	    {
	      auto &newView(std::get<wm::ClientData>(windowTree.getData(newViewNode).data));

	      newView->focus_view();
	    }
	}
    }

    void move_window(bool moveForward, bool moveVertical)
    {
      Server &server = Server::getInstance();

      if (auto view = server.getFocusedView())
	{
	  auto viewNode(view->windowNode);
	  auto &output(server.outputManager.getOutput(view->getWlrOutput()));

	  if (view->windowNode == wm::nullNode || output.getFullscreenView())
	    return ;

	  auto &windowTree(server.getActiveWindowTree());
	  auto containerNode(windowTree.getParent(viewNode));
	  auto *container(&windowTree.getData(containerNode).getContainer());
	  wm::WindowNodeIndex newViewNode;

	  if (container->direction ^ moveVertical)
	    {
	      if (containerNode == windowTree.getRootIndex())
		{
		  return ;
		}
	      auto newContainerNode = windowTree.getParent(containerNode);
	      auto *newContainer(&windowTree.getData(newContainerNode).getContainer());

	      container->removeChild(containerNode, windowTree, viewNode);
	      if (moveForward)
		{
		  newContainer->addChild(newContainerNode, windowTree, containerNode, wm::ClientData{view});
		}
	      else if (windowTree.getFirstChild(newContainerNode) != containerNode)
		{
		  newContainer->addChild(newContainerNode, windowTree, windowTree.getPrevSibling(containerNode), wm::ClientData{view});
		}
	      else
		{
		  newContainer->addChild(newContainerNode, windowTree, wm::ClientData{view});
		}
	      return ;
	    }
	  if (moveForward)
	    {
	      newViewNode = windowTree.getSibling(viewNode);
	      if (newViewNode == wm::nullNode)
		{
		  return ;
		}
	    }
	  else
	    {
	      auto firstChildViewNode(windowTree.getFirstChild(containerNode));

	      newViewNode = firstChildViewNode;
	      if (newViewNode == viewNode)
		{
		  return ;
		}
	      else
		{
		  auto tmpNode(windowTree.getSibling(newViewNode));

		  while (tmpNode != viewNode && tmpNode != wm::nullNode)
		    {
		      newViewNode = tmpNode;
		      tmpNode = windowTree.getSibling(newViewNode);
		    }
		}
	    }
	  {
	    auto viewNodePos(windowTree.getData(viewNode).getPosition());
	    auto newViewNodePos(windowTree.getData(newViewNode).getPosition());

	    windowTree.getData(newViewNode).move(newViewNode, windowTree, viewNodePos);
	    windowTree.getData(viewNode).move(viewNode, windowTree, newViewNodePos);
	    windowTree.getData(viewNode).data.swap(windowTree.getData(newViewNode).data);
	  }
	  try
	    {
	      std::get<wm::ClientData>(windowTree.getData(newViewNode).data)->windowNode = newViewNode;
	    } catch (...) {}
	  try
	    {
	      std::get<wm::ClientData>(windowTree.getData(viewNode).data)->windowNode = viewNode;
	    } catch (...) {}
	  windowTree.exchangeChildren(viewNode, newViewNode);
	}
    }
  }
}
