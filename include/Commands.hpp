#pragma once

# include <cstring>

# include "Wlroots.hpp"
# include "Server.hpp"

namespace
{
  auto getContainerFocusedView(Server *server, wm::WindowTree &windowTree, wm::WindowNodeIndex containerNode, wm::WindowNodeIndex viewNode)
  {
    for (auto &tmpView: server->views)
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
  
  void switch_focus_down_or_right(Server *server, bool parallelDirection)
  {
    if (server->views.empty())
      return ;

    std::unique_ptr<View> &view = server->views.front();
    auto viewNode(view->windowNode);
    auto &output(server->output.getOutput(view->getOutput()));

    if (view->windowNode == wm::nullNode || output.getFullscreenView())
      return ;

    auto &windowTree(output.getWindowTree());
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
	if (std::holds_alternative<wm::Container>(windowTree.getData(siblingNode).data))
	  newViewNode = getContainerFocusedView(server, windowTree, siblingNode, viewNode);
	else
	  newViewNode = siblingNode;
      }
    else if (newViewNode != wm::nullNode && std::holds_alternative<wm::Container>(windowTree.getData(newViewNode).data))
      {
	newViewNode = getContainerFocusedView(server, windowTree, newViewNode, viewNode);
      }
    if (newViewNode != viewNode && newViewNode != wm::nullNode)
      {
	auto &newView(std::get<wm::ClientData>(windowTree.getData(newViewNode).data).view);

	newView->focus_view();
      }
  }

void switch_focus_up_or_left(Server *server, bool parallelDirection)
{
  if (server->views.empty())
    return ;

  std::unique_ptr<View> &view = server->views.front();
  auto viewNode(view->windowNode);
  auto &output(server->output.getOutput(view->getOutput()));

  if (view->windowNode == wm::nullNode || output.getFullscreenView())
    return ;

  auto &windowTree(output.getWindowTree());
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
      if (std::holds_alternative<wm::Container>(windowTree.getData(siblingNode).data))
	{
	  while (windowTree.getSibling(siblingNode) != tmpNode && windowTree.getSibling(siblingNode) != wm::nullNode)
	    siblingNode = windowTree.getSibling(siblingNode);
	  newViewNode = getContainerFocusedView(server, windowTree, siblingNode, viewNode);
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
      if (std::holds_alternative<wm::Container>(windowTree.getData(newViewNode).data))
	{
	  while (windowTree.getSibling(newViewNode) != tmpNode && windowTree.getSibling(newViewNode) != wm::nullNode)
	    newViewNode = windowTree.getSibling(newViewNode);
	  newViewNode = getContainerFocusedView(server, windowTree, newViewNode, viewNode);
	}
    }
  if (newViewNode != viewNode)
    {
      auto &newView(std::get<wm::ClientData>(windowTree.getData(newViewNode).data).view);

      newView->focus_view();
    }
}
}

namespace Commands
{
  void open_terminal(Server *server) {
    if (fork() == 0)
      {
	std::string term = server->configuration.getOnce("terminal");

	if (!term.empty())
	  {
	    std::string command(server->configuration.getOnce("terminal"));

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

  void open_config_editor(Server *server) {
    if (fork() == 0)
    {
      std::unordered_map<std::string, std::string> commands;

      commands["gnome-terminal"] = "-- $PWD/openConfig.sh";
      commands["xfce4-terminal"] =  "-e $PWD/openConfig.sh";
      commands["weston-terminal"] = "--shell=$PWD/openConfig.sh";
      commands["fake-terminal"] = "fake";

      std::array<std::string, 3u> terminals{
	{
	 server->configuration.getOnce("config_terminal"),
	 server->configuration.getOnce("terminal"),
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

  void toggle_fullscreen(Server *server) {
    if (server->views.size() <= 0)
      return ;
    std::unique_ptr<View> &view = server->views.front();

    view->xdg_toplevel_request_fullscreen(nullptr, nullptr);
  }

  void switch_window(Server *server) {
    if (server->views.size() >= 2)
      {
	if (server->views.front()->windowNode != wm::nullNode)
	  {
	    std::partition(server->views.begin() + 1, server->views.end(),
			   [](auto &view) noexcept
			   {
			     return view->windowNode == wm::nullNode;
			   });
	  }
      
	std::unique_ptr<View> &view = server->views[1];

	view->focus_view();
	// focus view put the newly focused view in front
	// so we put it back to its position and then rotate
	std::iter_swap(server->views.begin(), server->views.begin() + 1);
	std::rotate(server->views.begin(), server->views.begin() + 1, server->views.end());
      }
  }

  void toggle_float_window(Server *server) {
    if (server->views.size() <= 0)
      return ;
    std::unique_ptr<View> &view = server->views.front();

    auto &output = server->output.getOutput(view->getOutput());
    auto &windowTree(output.getWindowTree());
    auto rootNode(windowTree.getRootIndex());
    auto &rootNodeData(windowTree.getData(rootNode));

    if (view->windowNode != wm::nullNode) {
      struct wlr_box box[1];

      rootNodeData.getContainer().removeChild(rootNode, windowTree, view->windowNode);
      view->x = 10_FP;
      view->y = 10_FP;
      wlr_xdg_surface_v6_get_geometry(view->xdg_surface, box);
      wlr_xdg_toplevel_v6_set_size(view->xdg_surface, view->previous_size[0], view->previous_size[1]);
      view->windowNode = wm::nullNode;
    }
    else
      view->windowNode = rootNodeData.getContainer().addChild(rootNode, windowTree, wm::ClientData{view.get()});
  }

  void switch_container_direction(Server *server) {
    if (server->views.size() <= 0)
      return ;
    std::unique_ptr<View> &view = server->views.front();

    if (view->windowNode == wm::nullNode)
      return ;
    auto &output = server->output.getOutput(view->getOutput());
    auto &windowTree(output.getWindowTree());
    auto parent = windowTree.getParent(view->windowNode);
    auto &parentData(windowTree.getData(parent).getContainer());

    parentData.changeDirection(parent, windowTree);
  }

  void close_compositor(Server *server) {
    wl_display_terminate(server->getWlDisplay());
  }

  void switch_focus_up(Server *server)
  {
    switch_focus_up_or_left(server, wm::Container::verticalTilling);
  }

  void switch_focus_left(Server *server)
  {
    switch_focus_up_or_left(server, wm::Container::horizontalTilling);
  }

  void switch_focus_down(Server *server)
  {
    switch_focus_down_or_right(server, wm::Container::verticalTilling);
  }

  void switch_focus_right(Server *server)
  {
    switch_focus_down_or_right(server, wm::Container::horizontalTilling);
  }
}
