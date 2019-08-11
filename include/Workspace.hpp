#pragma once

# include "Wlroots.hpp"
# include "wm/WindowTree.hpp"

class Output;

class Workspace
{
public:
  Workspace(Output &output, int id);
  Workspace(Workspace &&) = delete;
  Workspace(Workspace const &) = delete;

  ~Workspace() = default;

  wm::WindowTree &getWindowTree() noexcept
  {
    return windowTree;
  }

  std::vector<std::unique_ptr<View>> &getViews()
  {
    return views;
  };

  int id = -1;

private:
  Output &output;

  std::vector<std::unique_ptr<View>> views;
  wm::WindowTree windowTree;
};
