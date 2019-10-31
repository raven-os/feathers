# include <cstring>
# include <unistd.h>
# include <iostream>

# include "Commands.hpp"
# include "Output.hpp"

namespace
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

    if (server.getViews().empty())
      return ;

    std::unique_ptr<View> &view = server.getViews().front();
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

void switch_focus_up_or_left(bool parallelDirection)
{
  Server &server = Server::getInstance();

  if (server.getViews().empty())
    return ;

  std::unique_ptr<View> &view = server.getViews().front();
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

namespace Commands
{
  void open_terminal() {
    Server &server = Server::getInstance();
    if (fork() == 0)
      {
	std::string term = server.configuration.getOnce("terminal");

	if (!term.empty())
	  {
	    std::string command(server.configuration.getOnce("terminal"));

	    command += " || weston-terminal"; // fall back on weston-terminal in case of error
	    std::cout << command << std::endl;
	    execl("/bin/sh", "/bin/sh", "-c", command.c_str(), nullptr);
	  }
	else // no configured term, let's use weston-terminal
	  {
	    execl("/bin/sh", "/bin/sh", "-c", "weston-terminal", nullptr);
	  }
      }
  }

  void open_config_editor() {
    Server &server = Server::getInstance();

    if (fork() == 0)
    {
      std::unordered_map<std::string, std::string> commands;

      commands["gnome-terminal"] = "-- $PWD/openConfig.sh";
      commands["xfce4-terminal"] =  "-e $PWD/openConfig.sh";
      commands["weston-terminal"] = "--shell=$PWD/openConfig.sh";
      commands["fake-terminal"] = "fake";

      std::array<std::string, 3u> terminals{
	{
	 server.configuration.getOnce("config_terminal"),
	 server.configuration.getOnce("terminal"),
	 "weston-terminal"
	}
      };

      std::stringstream command;

      for (auto &term : terminals)
	if (!term.empty())
	  {
	    command << term << " " << commands[term] << " || ";
	  }
      command << "echo";

      execl("/bin/bash", "/bin/bash", "-c", command.str().c_str(), nullptr);
    }
  }

  void toggle_fullscreen() {
    Server &server = Server::getInstance();

    if (server.getViews().size() <= 0)
      return ;
    std::unique_ptr<View> &view = server.getViews().front();

    view->requestFullscreen();
  }

  void switch_window() {
    Server &server = Server::getInstance();

    if (server.getViews().size() >= 2)
      {
	if (server.getViews().front()->windowNode != wm::nullNode)
	  {
	    std::partition(server.getViews().begin() + 1, server.getViews().end(),
			   [](auto &view) noexcept
			   {
			     return view->windowNode == wm::nullNode;
			   });
	  }

	std::unique_ptr<View> &view = server.getViews()[1];

	view->focus_view();
	// focus view put the newly focused view in front
	// so we put it back to its position and then rotate
	std::iter_swap(server.getViews().begin(), server.getViews().begin() + 1);
	std::rotate(server.getViews().begin(), server.getViews().begin() + 1, server.getViews().end());
      }
  }

  void toggle_float_window() {
    Server &server = Server::getInstance();

    if (server.getViews().size() <= 0)
      return ;
    std::unique_ptr<View> &view = server.getViews().front();

    auto &windowTree(server.getActiveWindowTree());
    auto rootNode(windowTree.getRootIndex());
    auto &rootNodeData(windowTree.getData(rootNode));

    if (view->windowNode != wm::nullNode)
      {
	rootNodeData.getContainer().removeChild(rootNode, windowTree, view->windowNode);
	view->x = 10_FP;
	view->y = 10_FP;
	if (wlr_surface_is_xdg_surface_v6(view->surface))
	  wlr_xdg_toplevel_v6_set_size(wlr_xdg_surface_v6_from_wlr_surface(view->surface), view->previous_size[0], view->previous_size[1]);
	else if (wlr_surface_is_xdg_surface(view->surface))
	  wlr_xdg_toplevel_set_size(wlr_xdg_surface_from_wlr_surface(view->surface), view->previous_size[0], view->previous_size[1]);

	view->windowNode = wm::nullNode;
	view->set_tiled(0);
      }
    else
      {
	view->windowNode = rootNodeData.getContainer().addChild(rootNode, windowTree, wm::ClientData{view.get()});
	view->set_tiled(~0u);
      }
  }

  void switch_container_direction() {
    Server &server = Server::getInstance();

    if (server.getViews().size() <= 0)
      return ;
    std::unique_ptr<View> &view = server.getViews().front();

    if (view->windowNode == wm::nullNode)
      return ;
    auto &windowTree(server.getActiveWindowTree());
    auto parent = windowTree.getParent(view->windowNode);
    auto &parentData(windowTree.getData(parent).getContainer());

    parentData.changeDirection(parent, windowTree);
  }

  void close_compositor()
  {
    Server &server = Server::getInstance();
    wl_display_terminate(server.getWlDisplay());
  }

  void switch_focus_up()
  {
    switch_focus_up_or_left(wm::Container::verticalTiling);
  }

  void switch_focus_left()
  {
    switch_focus_up_or_left(wm::Container::horizontalTiling);
  }

  void switch_focus_down()
  {
    switch_focus_down_or_right(wm::Container::verticalTiling);
  }

  void switch_focus_right()
  {
    switch_focus_down_or_right(wm::Container::horizontalTiling);
  }

  void switch_workspace(int direction)
  {
    Server &server = Server::getInstance();

    for (auto const &output : server.outputManager.getOutputs())
    {
      auto it = std::find_if(output->getWorkspaces().begin(), output->getWorkspaces().end(),
                            [](auto &w) noexcept {
                              return w.get() == Server::getInstance().outputManager.getActiveWorkspace();
                            });

      if (direction == Workspace::RIGHT ?
          it == output->getWorkspaces().end() - 1 :
          it == output->getWorkspaces().begin())
        return ;
      auto newActiveWorkspace = it + direction;
      server.outputManager.setActiveWorkspace(newActiveWorkspace->get());
      if (server.getViews().size() > 0)
        server.getViews().front()->focus_view();
    }
  }

  void switch_window_from_workspace(int direction)
  {
    Server &server = Server::getInstance();

    auto &oldwindowTree(server.getActiveWindowTree());
    auto oldrootNode(oldwindowTree.getRootIndex());
    auto &oldrootNodeData(oldwindowTree.getData(oldrootNode));

    if (server.getViews().size() <= 0)
      return ;
    std::unique_ptr<View> &view = server.getViews().front();
    oldrootNodeData.getContainer().removeChild(oldrootNode, oldwindowTree, view->windowNode);
    auto &oldViews = server.getViews();
    
    switch_workspace(direction);
    
    auto &windowTree(server.getActiveWindowTree());
    auto rootNode(windowTree.getRootIndex());
    auto &rootNodeData(windowTree.getData(rootNode));

    auto it = std::find(oldViews.begin(), oldViews.end(), view);
    it->get()->windowNode = rootNodeData.getContainer().addChild(rootNode, windowTree, wm::ClientData{it->get()});
    View *Nview = new View(*(it->get()));
    oldViews.erase(it);
    server.getViews().emplace_back(Nview);
   // view->set_tiled(~0u);
   // view->focus_view();
  }

  void new_workspace()
  {
    Server &server = Server::getInstance();

    for (auto const &output : server.outputManager.getOutputs())
    {
      auto it = std::find_if(output->getWorkspaces().begin(), output->getWorkspaces().end(),
                            [](auto &w) noexcept {
                              return w.get() == Server::getInstance().outputManager.getActiveWorkspace();
                            });
      output->getWorkspaces().insert(it + 1, std::make_unique<Workspace>(*(output)));
    }
    server.outputManager.workspaceCount++;
    switch_workspace(Workspace::RIGHT);
  }

  void close_workspace()
  {
    Server &server = Server::getInstance();

    if (server.outputManager.workspaceCount == 2)
      return ;
    for (auto const &output : server.outputManager.getOutputs())
    {
      auto it = std::find_if(output->getWorkspaces().begin(), output->getWorkspaces().end(),
                            [](auto &w) noexcept {
                              return w.get() == Server::getInstance().outputManager.getActiveWorkspace();
                            });
      auto newActiveWorkspace = it + (it ==  output->getWorkspaces().begin() ? 1 : -1);
      server.outputManager.setActiveWorkspace(newActiveWorkspace->get());
      output->getWorkspaces().erase(it);
    }
    server.outputManager.workspaceCount--;
  }
}
