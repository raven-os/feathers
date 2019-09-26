#include <iostream>
#include "View.hpp"
#include "Popup.hpp"


Popup::Popup(wlr_surface *surface)
  : surface(surface)
{
  SET_LISTENER(Popup, PopupListeners, new_popup, handle_new_popup);
  SET_LISTENER(Popup, PopupListeners, destroy, handle_destroy_popup);
}

Popup::~Popup() = default;

void Popup::handle_new_popup(wl_listener *listener, void *data)
{
  if (wlr_surface_is_xdg_surface_v6(surface))
    popup = std::make_unique<Popup>(static_cast<wlr_xdg_popup_v6 *>(data)->base->surface);
  else if (wlr_surface_is_xdg_surface(surface))
    popup = std::make_unique<Popup>(static_cast<wlr_xdg_popup *>(data)->base->surface);
}

void Popup::handle_destroy_popup(wl_listener *listener, void *data)
{
  wl_list_remove(&new_popup.link);
  wl_list_remove(&destroy.link);
}
