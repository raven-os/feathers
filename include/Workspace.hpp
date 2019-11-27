#pragma once

# include "Wlroots.hpp"
# include "wm/WindowTree.hpp"

class Output;

class Workspace
{
public:
  Workspace() = delete;
  Workspace(Output &output);
  Workspace(Workspace &&) = delete;
  Workspace(Workspace const &) = delete;

  ~Workspace();

  wm::WindowTree &getWindowTree() noexcept
  {
    return windowTree;
  }

  std::vector<std::unique_ptr<XdgView>> &getViews()
  {
    return views;
  };

  Output const &getOutput() const noexcept
  {
    return output;
  }

  Output &getOutput() noexcept
  {
    return output;
  }

  static constexpr int RIGHT = 1;
  static constexpr int LEFT = -1;

private:
  Output &output;

  std::vector<std::unique_ptr<XdgView>> views;
  wm::WindowTree windowTree;
};
