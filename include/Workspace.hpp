#pragma once

# include "Wlroots.hpp"
# include "wm/WindowTree.hpp"

class Workspace
{
public:
  Workspace();
  Workspace(Workspace &&) = delete;
  Workspace(Workspace const &) = delete;

  ~Workspace() = default;

private:
  std::vector<std::unique_ptr<View>> views;
  wm::WindowTree windowTree;
};
