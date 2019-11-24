#pragma once

# include "Wlroots.hpp"
# include "Listeners.hpp"
# include "Workspace.hpp"

#include <vector>
#include <memory>

class View;

/* Used to move all of the data necessary to render a surface from the top-level
 * frame handler to the per-surface render function. */
struct render_data
{
  wlr_output *output;
  wlr_renderer *renderer;
  View *view;
  timespec *when;
  bool fullscreen;
};

class LayerSurface;

struct OutputManagerListeners
{
  wl_listener new_output;
};

class Output;

class OutputManager : public OutputManagerListeners
{
public:
  OutputManager();
  ~OutputManager() noexcept;

  void output_frame(wl_listener *listener, void *data);
  void server_new_output(wl_listener *listener, void *data);
  static void render_surface(wlr_surface *surface, int sx, int sy, void *data);

  wlr_output_layout *getLayout() const noexcept;
  std::vector<std::unique_ptr<Output>> const& getOutputs() const;

  Workspace *getActiveWorkspace() const noexcept
  {
    return activeWorkspace;
  }

  void setActiveWorkspace(Workspace *w) noexcept
  {
    activeWorkspace = w;
  }

  Output &getOutput(wlr_output *wlr_output) noexcept;
  Output const &getOutput(wlr_output *wlr_output) const noexcept;

  uint16_t workspaceCount = 2;
  uint16_t maxWorkspaceCount = 9;
private:
  Workspace *activeWorkspace = nullptr;

  wlr_output_layout *output_layout;
  wlr_xdg_output_manager_v1 *output_manager;
  std::vector<std::unique_ptr<Output>> outputs;
};
