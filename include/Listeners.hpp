#pragma once

namespace Listeners {

  #define SET_LISTENER(TYPE, SUBTYPE, NAME, TARGET)			\
    NAME.notify = [](wl_listener *listener, void *data)			\
  		{							\
  		  TYPE *that(static_cast<TYPE *>(wl_container_of(listener, static_cast<SUBTYPE *>(nullptr), NAME))); \
  		  that->TARGET(listener, data);				\
  		};							\


    struct ServerCursorListeners
    {
      struct wl_listener cursor_motion;
      struct wl_listener cursor_motion_absolute;
      struct wl_listener cursor_button;
      struct wl_listener cursor_axis;
    };

    struct ServerInputListeners
    {
        struct wl_listener new_input;
    };
}
