#include <iostream>
#include "View.hpp"
#include "Popup.hpp"


Popup::Popup(View *child, struct wlr_xdg_surface_v6 *xdg_surface)
  :xdg_surface(xdg_surface)
  , child(child)
{
  SET_LISTENER(Popup, PopupListeners, new_popup, handle_new_popup);
  SET_LISTENER(Popup, PopupListeners, destroy, handle_destroy_popup);
}

Popup::~Popup()
{
}

void Popup::handle_new_popup(struct wl_listener *listener, void *data)
{
}

void Popup::handle_destroy_popup(struct wl_listener *listener, void *data)
{
  wl_list_remove(&new_popup.link);
  wl_list_remove(&destroy.link);
}
