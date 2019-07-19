#pragma once

# include "Wlroots.hpp"
# include "Listeners.hpp"

class Server;

enum CursorMode
  {
   CURSOR_PASSTHROUGH,
   CURSOR_MOVE,
   CURSOR_RESIZE,
  };

struct ServerCursorListeners
{
  struct wl_listener cursor_motion;
  struct wl_listener cursor_motion_absolute;
  struct wl_listener cursor_button;
  struct wl_listener cursor_axis;
  struct wl_listener cursor_frame;
};

class ServerCursor : public ServerCursorListeners
{
public:
  ServerCursor();
  ~ServerCursor() = default;

  void server_cursor_motion(struct wl_listener *listener, void *data);
  void server_cursor_motion_absolute(struct wl_listener *listener, void *data);
  void server_cursor_button(struct wl_listener *listener, void *data);
  void server_cursor_frame(struct wl_listener *, void *);
  void server_cursor_axis(struct wl_listener *listener, void *data);


  struct wlr_cursor *cursor;
  struct wlr_xcursor_manager *cursor_mgr;
  CursorMode cursor_mode;

private:
  void process_cursor_move(uint32_t time);
  void process_cursor_resize(uint32_t time);
  void process_cursor_motion(uint32_t time);

};
