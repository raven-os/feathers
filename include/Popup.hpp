#pragma once

# include "Wlroots.hpp"
# include "Listeners.hpp"

class Server;
class View;

struct PopupListeners
{
    wl_listener new_popup;
    wl_listener destroy;
};

class Popup : public PopupListeners
{
public:
  Popup(wlr_surface *surface);
  ~Popup();

  wlr_surface *surface;
  std::unique_ptr<Popup> popup;
  
private:
  void handle_new_popup(wl_listener *listener, void *data);
  void handle_destroy_popup(wl_listener *listener, void *data);
};
