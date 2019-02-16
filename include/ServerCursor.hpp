#ifndef CURSOR_HPP_
# define CURSOR_HPP_

# include "Wlroots.hpp"

class Server;

enum CursorMode
  {
   CURSOR_PASSTHROUGH,
   CURSOR_MOVE,
   CURSOR_RESIZE,
  };

class ServerCursor
{
public:
  ServerCursor(Server *server);
  ~ServerCursor();

  void server_cursor_motion(struct wl_listener *listener, void *data);
  void server_cursor_motion_absolute(struct wl_listener *listener, void *data);
  void server_cursor_button(struct wl_listener *listener, void *data);
  void server_cursor_axis(struct wl_listener *listener, void *data);

  struct wlr_cursor *cursor;
  struct wlr_xcursor_manager *cursor_mgr;
  struct wl_listener cursor_motion;
  struct wl_listener cursor_motion_absolute;
  struct wl_listener cursor_button;
  struct wl_listener cursor_axis;
  CursorMode cursor_mode;

private:
  Server *server;

  void process_cursor_move(uint32_t time);
  void process_cursor_resize(uint32_t time);
  void process_cursor_motion(uint32_t time);

};

#endif /* !CURSOR_HPP_ */
