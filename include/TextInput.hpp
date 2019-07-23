#ifndef TEXTINPUT_HPP_
# define TEXTINPUT_HPP_

# include "Wlroots.hpp"

class InputMethodRelay;

struct TextInputListeners
{
  struct wl_listener pending_focused_surface_destroy;
};

class TextInput : public TextInputListeners
{
public:
  TextInput(InputMethodRelay& relay, struct wlr_text_input_v3 *wlr_text_input);
  ~TextInput() = default;

  InputMethodRelay& relay;

  struct wlr_text_input_v3 *input;
  struct wlr_surface *pending_focused_surface;

private:
  void handle_pending_focused_surface_destroy(struct wl_listener *listener, void *data);
};

#endif /* !TEXTINPUT_HPP_ */
