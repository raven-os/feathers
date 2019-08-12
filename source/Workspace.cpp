#include "Workspace.hpp"
# include "Server.hpp"

Workspace::Workspace(Output &output) :
  output(output),
  windowTree([&]()
	     {
	       auto box = wlr_output_layout_get_box(Server::getInstance().outputManager.getLayout(), nullptr);

	       return wm::WindowData{wm::Container(wm::Rect{{{FixedPoint<0, int>(box->x),
							      FixedPoint<0, int>(box->y)}},
							    {{FixedPoint<0, int>(box->width),
							      FixedPoint<0, int>(box->height)}}})};
	     }())
{

}

Workspace::~Workspace()
{
    for (auto &view : views)
      view->close();
}
