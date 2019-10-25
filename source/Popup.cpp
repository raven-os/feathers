#include <iostream>
#include "View.hpp"
#include "Popup.hpp"


Popup::Popup(View *child, wlr_surface *surface)
  : surface(surface)
  , child(child)
{
  SET_LISTENER(Popup, PopupListeners, new_popup, handle_new_popup);
  SET_LISTENER(Popup, PopupListeners, destroy, handle_destroy_popup);
}

Popup::~Popup()
{
}

void Popup::handle_new_popup([[maybe_unused]]wl_listener *listener, [[maybe_unused]]void *data)
{
}

void Popup::handle_destroy_popup([[maybe_unused]]wl_listener *listener, [[maybe_unused]]void *data)
{
  wl_list_remove(&new_popup.link);
  wl_list_remove(&destroy.link);
}
