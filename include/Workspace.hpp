#pragma once

# include "Wlroots.hpp"
# include "wm/WindowTree.hpp"

class Output;

class Workspace
{
public:
  Workspace() = default;
  Workspace(Output &output);
  Workspace(Workspace &&) = delete;
  Workspace(Workspace const &) = delete;

  ~Workspace();

  wm::WindowTree &getWindowTree() noexcept
  {
    return windowTree;
  }

  std::vector<std::unique_ptr<View>> &getViews()
  {
    return views;
  };

private:
  Output &output;

  std::vector<std::unique_ptr<View>> views;
  wm::WindowTree windowTree;
};
