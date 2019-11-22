#pragma once

# include "Wlroots.hpp"

extern "C" {

# include <wlr/xwayland.h>

}

struct XWaylandListeners {

};

class XWayland : XWaylandListeners
{
    public:
        XWayland();
        ~XWayland() = default;

};