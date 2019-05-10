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
    if (server->views.size() >= 1)
      {
  std::unique_ptr<View> &view = server->views.front();
  auto const &output =
    std::find_if(server->output.getOutputs().begin(), server->output.getOutputs().end(),
           [&view](auto &out) {
       return out->getWlrOutput() == view->getOutput();
           })
    ->get();

  if (!output->getFullscreen())
    {
      wlr_xdg_surface_v6_get_geometry(view->xdg_surface, &output->saved);
      output->saved.x = view->x;
      output->saved.y = view->y;
      struct wlr_box *outputBox = wlr_output_layout_get_box(view->server->output.getLayout(), view->getOutput());
      wlr_xdg_toplevel_v6_set_size(view->xdg_surface, outputBox->width, outputBox->height);
      view->x = 0;
      view->y = 0;
      wlr_xdg_toplevel_v6_set_fullscreen(view->xdg_surface, true);
    }
  else
    {
      wlr_xdg_toplevel_v6_set_fullscreen(view->xdg_surface, false);
      wlr_xdg_toplevel_v6_set_size(view->xdg_surface, output->saved.width, output->saved.height);
      view->x = output->saved.x;
      view->y = output->saved.y;
    }
  output->setFullscreen(!output->getFullscreen());
      }
  }

  void switch_window(Server *server) {
    if (server->views.size() >= 2)
      {
	std::unique_ptr<View> &view = server->views[1];
  	ServerView::focus_view(view.get(), view->xdg_surface->surface);
	// focus view put the newly focused view in front
	// so we put it back to its position and then rotate
	std::iter_swap(server->views.begin(), server->views.begin() + 1);
	std::rotate(server->views.begin(), server->views.begin() + 1, server->views.end());
      }
  }

  void close_compositor(Server *server) {
    wl_display_terminate(server->getWlDisplay());
  }
}
