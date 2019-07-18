#pragma once

# include "Wlroots.hpp"
# include "Listeners.hpp"

class Server;
class View;

struct PopupListeners
{
    struct wl_listener new_popup;
    struct wl_listener destroy;
};

class Popup : public PopupListeners
{
  public:
    Popup(View *child, struct wlr_xdg_surface_v6 *xdg_surface);
    ~Popup();

    struct wlr_xdg_surface_v6 *xdg_surface;

  private:
    View *child;

    void handle_new_popup(struct wl_listener *listener, void *data);
    void handle_destroy_popup(struct wl_listener *listener, void *data);
};
