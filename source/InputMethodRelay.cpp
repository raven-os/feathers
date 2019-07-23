#include "Server.hpp"
#include "InputMethodRelay.hpp"
#include "TextInput.hpp"

void InputMethodRelay::send_im_done(struct wlr_text_input_v3 *input)
{
  if (!input_method)
    {
      wlr_log(WLR_INFO, "Sending IM_DONE but im is gone");
      return;
    }

  wlr_input_method_v2_send_surrounding_text(input_method,
					    input->current.surrounding.text,
					    input->current.surrounding.cursor,
					    input->current.surrounding.anchor);
  wlr_input_method_v2_send_text_change_cause(input_method,
					     input->current.text_change_cause);
  wlr_input_method_v2_send_content_type(input_method,
					input->current.content_type.hint,
					input->current.content_type.purpose);
  wlr_input_method_v2_send_done(input_method);
}

void InputMethodRelay::handle_text_input_enable(struct wl_listener *listener, void *data)
{
  if (input_method == NULL)
    {
      wlr_log(WLR_INFO, "Enabling text input when input method is gone");
      return;
    }
  for (auto const& text_input : text_inputs)
    {
      if (text_input->input == data)
	{
	  wlr_input_method_v2_send_activate(input_method);
	  send_im_done(text_input->input);
	  break;
	}
    }
}

void InputMethodRelay::handle_text_input_commit(struct wl_listener *listener, void *data)
{
  for (auto const& text_input : text_inputs)
    {
      if (text_input->input == data)
	{
	  if (!text_input->input->current_enabled) {
	    wlr_log(WLR_INFO, "Inactive text input tried to commit an update");
	    return;
	  }
	  wlr_log(WLR_DEBUG, "Text input committed update");
	  if (input_method == NULL) {
	    wlr_log(WLR_INFO, "Text input committed, but input method is gone");
	    return;
	  }
	  send_im_done(text_input->input);
	  break;
	}
    }
}

void InputMethodRelay::handle_text_input_disable(struct wl_listener *listener, void *data)
{
  for (auto const& text_input : text_inputs)
    {
      if (text_input->input == data)
	{
	  if (input_method == NULL)
	    {
	      wlr_log(WLR_DEBUG, "Disabling text input, but input method is gone");
	      return;
	    }
	  wlr_input_method_v2_send_deactivate(input_method);
	  send_im_done(text_input->input);
	  break;
	}
    }
}

void InputMethodRelay::handle_text_input_destroy(struct wl_listener *listener,
						 void *data) {
  for (auto text_input = text_inputs.begin(); text_input != text_inputs.end(); ++text_input)
    {
      if ((*text_input)->input == data)
	{
	  if ((*text_input)->input->current_enabled) {
	    if (input_method == NULL)
	      {
		wlr_log(WLR_DEBUG, "Disabling text input, but input method is gone");
		return;
	      }
	    wlr_input_method_v2_send_deactivate(input_method);
	    send_im_done((*text_input)->input);
	  }
	  wl_list_remove(&(*text_input)->pending_focused_surface_destroy.link);
	  wl_list_init(&(*text_input)->pending_focused_surface_destroy.link);
	  (*text_input)->pending_focused_surface = NULL;
	  text_inputs.erase(text_input);
	  break;
	}
    }
}

void InputMethodRelay::handle_text_input([[maybe_unused]]struct wl_listener *listener,
					 [[maybe_unused]]void *data)
{
  struct wlr_text_input_v3 *input = static_cast<struct wlr_text_input_v3*>(data);

  text_inputs.emplace_back(std::make_unique<TextInput>(TextInput(*this, input)));

  SET_LISTENER(InputMethodRelay, InputMethodRelayListeners, text_input_enable, handle_text_input_enable);
  wl_signal_add(&input->events.enable, &text_input_enable);

  SET_LISTENER(InputMethodRelay, InputMethodRelayListeners, text_input_commit, handle_text_input_commit);
  wl_signal_add(&input->events.commit, &text_input_commit);

  SET_LISTENER(InputMethodRelay, InputMethodRelayListeners, text_input_disable, handle_text_input_disable);
  wl_signal_add(&input->events.disable, &text_input_disable);

  SET_LISTENER(InputMethodRelay, InputMethodRelayListeners, text_input_destroy, handle_text_input_destroy);
  wl_signal_add(&input->events.destroy, &text_input_destroy);
}

void InputMethodRelay::handle_input_method_commit(struct wl_listener *listener, void *data)
{
  for (auto const& text_input : text_inputs)
    {
      if (text_input->input->focused_surface)
	{
	  struct wlr_input_method_v2 *context = static_cast<struct wlr_input_method_v2*>(data);
	  if (context->current.preedit.text)
	    {
	      wlr_text_input_v3_send_preedit_string(text_input->input,
						    context->current.preedit.text,
						    context->current.preedit.cursor_begin,
						    context->current.preedit.cursor_end);
	    }
	  if (context->current.commit_text)
	    {
	      wlr_text_input_v3_send_commit_string(text_input->input,
						   context->current.commit_text);
	    }
	  // TODO ??!!!
	  // if (context->current.delete.before_length
	  //     || context->current.delete.after_length) {
	  //   wlr_text_input_v3_send_delete_surrounding_text(text_input->input,
	  // 						   context->current.delete.before_length,
	  // 						   context->current.delete.after_length);
	  // }
	  wlr_text_input_v3_send_done(text_input->input);
	  break;
	}
    }
}

void InputMethodRelay::handle_input_method_destroy(struct wl_listener *listener, void *data)
{
  struct wlr_input_method_v2 *context = static_cast<struct wlr_input_method_v2*>(data);
  input_method = NULL;
  for (auto const& text_input : text_inputs)
    {
      if (text_input->input->focused_surface)
	{
	  text_input->pending_focused_surface = text_input->input->focused_surface;
	  wl_signal_add(&text_input->input->focused_surface->events.destroy,
			&text_input->pending_focused_surface_destroy);
	  wlr_text_input_v3_send_leave(text_input->input);
	  break;
	}
    }
}

void InputMethodRelay::handle_input_method(struct wl_listener *listener, void *data)
{
  struct wlr_input_method_v2 *input_method = static_cast<struct wlr_input_method_v2*>(data);
  if (Server::getInstance().seat.getSeat() != input_method->seat)
    {
      return;
    }

  if (input_method != nullptr)
    {
      wlr_log(WLR_INFO, "Attempted to connect second input method to a seat");
      wlr_input_method_v2_send_unavailable(input_method);
      return;
    }

  this->input_method = input_method;

  SET_LISTENER(InputMethodRelay, InputMethodRelayListeners, input_method_commit, handle_input_method_commit);
  wl_signal_add(&input_method->events.commit, &input_method_commit);

  SET_LISTENER(InputMethodRelay, InputMethodRelayListeners, input_method_destroy, handle_input_method_destroy);
  wl_signal_add(&input_method->events.destroy, &input_method_destroy);

  for (auto const& text_input : text_inputs)
    {
      if (text_input->pending_focused_surface)
	{
	  wlr_text_input_v3_send_enter(text_input->input,
				       text_input->pending_focused_surface);
	  wl_list_remove(&text_input->pending_focused_surface_destroy.link);
	  wl_list_init(&text_input->pending_focused_surface_destroy.link);
	  text_input->pending_focused_surface = NULL;
	  break;
	}
    }
}

InputMethodRelay::InputMethodRelay() :
  input_method_manager(wlr_input_method_manager_v2_create(Server::getInstance().getWlDisplay())),
  text_input_manager(wlr_text_input_manager_v3_create(Server::getInstance().getWlDisplay()))
{
  SET_LISTENER(InputMethodRelay, InputMethodRelayListeners, text_input_new, handle_text_input);
  wl_signal_add(&text_input_manager->events.text_input, &text_input_new);

  SET_LISTENER(InputMethodRelay, InputMethodRelayListeners, input_method_new, handle_input_method);
  wl_signal_add(&input_method_manager->events.input_method, &input_method_new);
}
