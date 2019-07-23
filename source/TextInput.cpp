#include "Server.hpp"
#include "TextInput.hpp"

void TextInput::handle_pending_focused_surface_destroy([[maybe_unused]]struct wl_listener *listener,
						       [[maybe_unused]]void *data)
{
  pending_focused_surface = nullptr;
}

TextInput::TextInput(InputMethodRelay& relay, struct wlr_text_input_v3 *wlr_text_input) :
  relay(relay),
  input(wlr_text_input)
{
  SET_LISTENER(TextInput, TextInputListeners, pending_focused_surface_destroy, handle_pending_focused_surface_destroy);
  wl_list_init(&pending_focused_surface_destroy.link);
}
