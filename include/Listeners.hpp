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

    struct ServerOutputListeners
    {
        struct wl_listener new_output;
    };

    struct OutputListeners
    {
        struct wl_listener frame;
    };

    struct KeyboardListeners
    {
      struct wl_listener modifiers;
      struct wl_listener key;
    };

    struct SeatListeners
    {
      struct wl_listener request_cursor;
    };

    struct XdgShellListeners
    {
      struct wl_listener new_xdg_surface;
    };

    struct ViewListeners
    {
      struct wl_listener map;
      struct wl_listener unmap;
      struct wl_listener destroy;
      struct wl_listener request_move;
      struct wl_listener request_resize;
    };
}
