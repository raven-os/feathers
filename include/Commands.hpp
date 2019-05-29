#pragma once

# include "Wlroots.hpp"
# include "Server.hpp"

namespace Commands
{
  void open_terminal() {
    if (fork() == 0)
    {
      execl("/bin/sh", "/bin/sh", "-c", "weston-terminal", nullptr);
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
}
