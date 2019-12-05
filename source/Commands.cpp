# include <cstring>
# include <unistd.h>
# include <iostream>

# include "Commands.hpp"
# include "Output.hpp"
# include "XdgView.hpp"
# include <signal.h>

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

  void open_dmenu() {
    if (fork() == 0)
    {
        execl("/bin/sh", "/bin/sh", "-c", "dmenu-wl_run -i", nullptr);
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
    XdgView *view = server.getFocusedView();

    view->requestFullscreen();
  }

  void switch_window() {
    Server &server = Server::getInstance();

    if (server.getViews().size() >= 2)
      {
	if (server.getFocusedView()->windowNode != wm::nullNode)
	  {
	    std::partition(server.getViews().begin() + 1, server.getViews().end(),
			   [](auto &view) noexcept
			   {
			     return view->windowNode == wm::nullNode;
			   });
	  }

	std::unique_ptr<XdgView> &view = server.getViews()[1];

	view->focus_view();
	// focus view put the newly focused view in front
	// so we put it back to its position and then rotate
	std::iter_swap(server.getViews().begin(), server.getViews().begin() + 1);
	std::rotate(server.getViews().begin(), server.getViews().begin() + 1, server.getViews().end());
      }
  }

  void toggle_float_window() {
    Server &server = Server::getInstance();

    if (XdgView *view = server.getFocusedView())
      {
	auto &windowTree(server.getActiveWindowTree());

	if (view->windowNode != wm::nullNode)
	  {
	    wm::Container::removeFromParent(windowTree, view->windowNode);
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
	    auto rootNode(windowTree.getRootIndex());
	    auto &rootNodeData(windowTree.getData(rootNode));

	    view->windowNode = rootNodeData.getContainer().addChild(rootNode, windowTree, wm::ClientData{view});
	    view->set_tiled(~0u);
	  }
      }
  }

  void switch_container_direction() {
    Server &server = Server::getInstance();

    if (XdgView *view = server.getFocusedView())
      {
	if (view->windowNode == wm::nullNode)
	  return ;
	auto &windowTree(server.getActiveWindowTree());
	auto parent = windowTree.getParent(view->windowNode);
	auto &parentData(windowTree.getData(parent).getContainer());

	parentData.changeDirection(parent, windowTree);
      }
  }

  void close_compositor()
  {
    Server &server = Server::getInstance();
    wl_display_terminate(server.getWlDisplay());
  }

  void switch_focus_left()
  {
    WindowCommands::switch_focus_up_or_left(wm::Container::horizontalTiling);
  }

  void switch_focus_right()
  {
    WindowCommands::switch_focus_down_or_right(wm::Container::horizontalTiling);
  }

  void switch_focus_up()
  {
    WindowCommands::switch_focus_up_or_left(wm::Container::verticalTiling);
  }

  void switch_focus_down()
  {
    WindowCommands::switch_focus_down_or_right(wm::Container::verticalTiling);
  }

  void move_window_left()
  {
    WindowCommands::move_window(false, false);
  }

  void move_window_right()
  {
    WindowCommands::move_window(true, false);
  }

  void move_window_up()
  {
    WindowCommands::move_window(false, true);
  }

  void move_window_down()
  {
    WindowCommands::move_window(true, true);
  }

  void switch_workspace(int direction, void *data)
  {
    Server &server = Server::getInstance();
    std::string binding;
    std::string key = "";

    // get specific worskpace number
    if (data) {
      std::string *s = static_cast<std::string*>(data);
      binding = *s;

      delete s;
      key = binding.substr(binding.find_last_of("+") + 1);
    }

    for (auto const &output : server.outputManager.getOutputs())
    {
      auto it = std::find_if(output->getWorkspaces().begin(), output->getWorkspaces().end(),
                            [](auto &w) noexcept {
                              return w.get() == Server::getInstance().outputManager.getActiveWorkspace();
                            });
     //set next worskpace according to direction
     auto newActiveWorkspace = (it + direction)->get();

      if (direction == Workspace::RIGHT ?
          (Commands::concordances.find(key) == Commands::concordances.end() && it == output->getWorkspaces().end() - 1) :
          it == output->getWorkspaces().begin()) {
        return ;
      }

      // Go to specific workspace if it's not unsing direction
      if (Commands::concordances.find(key) != Commands::concordances.end()) {
        if (Commands::concordances[key] < output->getWorkspaces().size())
          newActiveWorkspace = output->getWorkspaces().at(Commands::concordances[key]).get();
        else
          return;
      }
     
      server.outputManager.setActiveWorkspace(newActiveWorkspace);
      if (XdgView *view = server.getFocusedView())
        view->focus_view();
    }
  }

  void switch_window_from_workspace(int direction)
  {
    Server &server = Server::getInstance();
    if (server.getViews().size() <= 0)
      return ;
    
    for (auto const &output : server.outputManager.getOutputs())
    {
      auto currentWorkspace = std::find_if(output->getWorkspaces().begin(), output->getWorkspaces().end(),
                            [](auto &w) noexcept {
                              return w.get() == Server::getInstance().outputManager.getActiveWorkspace();
                            });
      if (direction == Workspace::RIGHT ?
          currentWorkspace == output->getWorkspaces().end() - 1 :
          currentWorkspace == output->getWorkspaces().begin())
        return ;
      auto nextWorkspace = currentWorkspace + direction;
      auto &view = currentWorkspace->get()->getViews().front();

      std::unique_ptr<XdgView> newView;
    
      {
	auto &windowTree = currentWorkspace->get()->getWindowTree();

	wm::Container::removeFromParent(windowTree, view->windowNode);
	newView = std::move(view);
        currentWorkspace->get()->getViews().erase(std::find(currentWorkspace->get()->getViews().begin(), currentWorkspace->get()->getViews().end(), view));
      }
      {
        auto &windowTree = nextWorkspace->get()->getWindowTree();
        auto rootNode(windowTree.getRootIndex());
        auto &rootNodeData(windowTree.getData(rootNode));
        
        newView->windowNode = rootNodeData.getContainer().addChild(rootNode, windowTree, wm::ClientData{newView.get()});
        newView->set_tiled(~0u);
        nextWorkspace->get()->getViews().emplace_back(std::move(newView));
      }
      server.outputManager.setActiveWorkspace(nextWorkspace->get());
    }
  }

  void new_workspace(bool create_fullscreen)
  {
    Server &server = Server::getInstance();

    if (server.outputManager.workspaceCount < server.outputManager.maxWorkspaceCount)
      {
        for (auto const &output : server.outputManager.getOutputs())
          {
            auto it = std::find_if(output->getWorkspaces().begin(), output->getWorkspaces().end(),
                                   [](auto &w) noexcept {
                                     return w.get() == Server::getInstance().outputManager.getActiveWorkspace();
                                   });
            output->getWorkspaces().insert(it + 1, std::make_unique<Workspace>(*(output)));
          }
        server.outputManager.workspaceCount++;
        if (!create_fullscreen)
          switch_workspace(Workspace::RIGHT, nullptr);
      }
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

  void close_view() {
    Server &server = Server::getInstance();
    for (auto &view : server.getViews())
      {
	if ((wlr_surface_is_xdg_surface_v6(view->surface) &&
	     wlr_xdg_surface_v6_from_wlr_surface(view->surface)->role == WLR_XDG_SURFACE_V6_ROLE_TOPLEVEL &&
	     wlr_xdg_surface_v6_from_wlr_surface(view->surface)->toplevel->server_pending.activated) ||
	    (wlr_surface_is_xdg_surface(view->surface) &&
	     wlr_xdg_surface_from_wlr_surface(view->surface)->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL &&
	     wlr_xdg_surface_from_wlr_surface(view->surface)->toplevel->server_pending.activated))
	  {
	    view->close();
	    break;
	  }
      }
  }
}
