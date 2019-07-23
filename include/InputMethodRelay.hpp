#ifndef INPUTMETHODRELAY_HPP_
# define INPUTMETHODRELAY_HPP_

# include <vector>
# include <memory>

# include "Wlroots.hpp"
# include "TextInput.hpp"

struct InputMethodRelayListeners
{
  struct wl_listener text_input_new;
  struct wl_listener text_input_enable;
  struct wl_listener text_input_commit;
  struct wl_listener text_input_disable;
  struct wl_listener text_input_destroy;

  struct wl_listener input_method_new;
  struct wl_listener input_method_commit;
  struct wl_listener input_method_destroy;
};

class InputMethodRelay : public InputMethodRelayListeners
{
public:
  InputMethodRelay();
  ~InputMethodRelay() = default;

  std::vector<std::unique_ptr<TextInput>> text_inputs;
  struct wlr_input_method_v2 *input_method;

  struct wlr_input_method_manager_v2 *input_method_manager;
  struct wlr_text_input_manager_v3 *text_input_manager;

private:
  void handle_text_input(struct wl_listener *listener, void *data);
  void handle_text_input_enable(struct wl_listener *listener, void *data);
  void handle_text_input_commit(struct wl_listener *listener, void *data);
  void handle_text_input_disable(struct wl_listener *listener, void *data);
  void handle_text_input_destroy(struct wl_listener *listener, void *data);

  void handle_input_method(struct wl_listener *listener, void *data);
  void handle_input_method_commit(struct wl_listener *listener, void *data);
  void handle_input_method_destroy(struct wl_listener *listener, void *data);

  TextInput *getTextInput(InputMethodRelay *relay, struct wlr_text_input_v3 *wlr_text_input);
  void send_im_done(struct wlr_text_input_v3 *input);
};

#endif /* !INPUTMETHODRELAY_HPP_ */
