#include <unistd.h>
#include <iostream>
#include <algorithm>
#include <memory>

#include "Keyboard.hpp"
#include "Commands.hpp"
#include "Server.hpp"
#include "XdgView.hpp"

std::map<std::string, uint32_t> modifiersLst = {
  {"Alt", WLR_MODIFIER_ALT},
  {"Ctrl", WLR_MODIFIER_CTRL},
  {"Shift", WLR_MODIFIER_SHIFT}
};

Keyboard::Keyboard(wlr_input_device *device)
  : keymap(nullptr)
  , device(device)
{
  key_repeat_source = wl_event_loop_add_timer(Server::getInstance().wl_event_loop, keyboard_handle_repeat, this);

  default_meta_keys["$mod"] = "Alt";
  //debug = true;
  default_shortcuts["$mod+Return"] = {"Terminal", [](void*){ Commands::open_terminal(); }};
  default_shortcuts["$mod+d"] = {"d-menu", [](void*){ Commands::open_dmenu(); }};
  default_shortcuts["$mod+F4"] = {"destroy", [](void*){ Commands::close_view(); }};
  default_shortcuts["$mod+F2"] = {"Toggle fullscreen", [](void*){ Commands::toggle_fullscreen(); }};
  default_shortcuts["$mod+Tab"] = {"Switch window", [](void*){ Commands::switch_window(); }};
  default_shortcuts["$mod+Space"] = {"Toggle float", [](void*){ Commands::toggle_float_window(); }};
  default_shortcuts["$mod+E"] = {"Switch position", [](void*){ Commands::switch_container_direction(); }};
  default_shortcuts["$mod+H"] = {"Open below", [](void*){ Server::getInstance().openType = OpenType::below; }};
  default_shortcuts["$mod+V"] = {"Open right", [](void*){ Server::getInstance().openType = OpenType::right; }};
  default_shortcuts["$mod+Up"] = {"Switch focus up", [](void*){ Commands::switch_focus_up(); }};
  default_shortcuts["$mod+Left"] = {"Switch focus left", [](void*){ Commands::switch_focus_left(); }};
  default_shortcuts["$mod+Down"] = {"Switch focus Down", [](void*){ Commands::switch_focus_down(); }};
  default_shortcuts["$mod+Right"] = {"Switch focus Right", [](void*){ Commands::switch_focus_right(); }};
  default_shortcuts["Ctrl+[$mod+Down,$mod+Right,1,2,3,4,5,6,7,8,9,&,é,\",\',(,-,è,_,ç,)]"] = {"Switch workspace (left to right)", [](void *data){ Commands::switch_workspace(Workspace::RIGHT, data); }};
  default_shortcuts["Ctrl+$mod+[Up,Left]"] = {"Switch workspace (right to left)", [](void*){ Commands::switch_workspace(Workspace::LEFT, nullptr); }};
  default_shortcuts["Shift+Right"] = {"Switch window to right workspace", [](void*){ Commands::switch_window_from_workspace(Workspace::RIGHT); }};
  default_shortcuts["Shift+Left"] = {"Switch window to left workspace", [](void*){ Commands::switch_window_from_workspace(Workspace::LEFT); }};
  default_shortcuts["Ctrl+$mod+w"] = {"New workspace", [](void*){ Commands::new_workspace(false); }};
  default_shortcuts["Ctrl+W"] = {"Close workspace", [](void*){ Commands::close_workspace(nullptr); }};
  default_shortcuts["$mod+J"] = {"Move window left", [](void *){ Commands::move_window_left(); }};
  default_shortcuts["$mod+L"] = {"Move window right", [](void *){ Commands::move_window_right(); }};
  default_shortcuts["$mod+I"] = {"Move window up", [](void *){ Commands::move_window_up(); }};
  default_shortcuts["$mod+K"] = {"Move window down", [](void *){ Commands::move_window_down(); }};
  //Allowing keyboard debug
  default_shortcuts["Ctrl+$mod+D"] = {"Debug", [this](void*){debug = !debug;}};
  default_shortcuts["$mod+Escape"] = {"Leave", [](void*){ Commands::close_compositor(); }};
  default_shortcuts["$mod+F1"] = {"Open config editor", [](void*){ Commands::open_config_editor(); }};
  for (auto const &default_shortcut : default_shortcuts)
    {
      shortcuts[default_shortcut.first] = default_shortcut.second;
    }
  parse_shortcuts();
}

Keyboard::~Keyboard() {
  if (keymap) {
    xkb_keymap_unref(keymap);
  }
  wl_list_remove(&key.link);
  wl_list_remove(&modifiers.link);
  wl_event_source_remove(key_repeat_source);
}

void Keyboard::update_shortcuts()
{
  Server &server(Server::getInstance());
  std::map<std::string, binding> shortcuts_tmp;

  for (auto it = shortcuts.begin(); it != shortcuts.end();)
    {
      if (server.configuration.consumeChanged(it->second.name.data()))
	{
	  std::string newShortcutKey(server.configuration.get(it->second.name.data()));
	  std::vector<std::string> splitShortcut(this->split_shortcut(it->first));
	  std::string shortcutKeyTmp("");

	  for (std::string tmp : splitShortcut) {
	    if (tmp.length() && shortcutKeyTmp.length() && shortcutKeyTmp.back() != '+')
	      shortcutKeyTmp += "+";
	    if (tmp.length() && tmp[0] == '$')
	      {
		std::string meta_key_value(server.configuration.get(tmp.substr(1).data()));

		if (meta_key_value != "")
		  shortcutKeyTmp += server.configuration.get(tmp.substr(1).data());
		else
		  {
		    auto it(default_meta_keys.find(tmp));

		    if (it != default_meta_keys.end())
		      {
			shortcutKeyTmp += it->second;
		      }
		  }
	      }
	    else
	      {
		shortcutKeyTmp += tmp;
	      }
	  }
	  if (shortcutKeyTmp != "")
	    {
	      shortcuts_tmp[shortcutKeyTmp] = it->second;
	    }
	  else
	    {
	      for (auto &default_shortcut : default_shortcuts)
		{
		  if (default_shortcut.second.name == it->second.name)
		    {
		      shortcuts_tmp[default_shortcut.first] = default_shortcut.second;
		      break;
		    }
		}
	    }
	  parse_shortcuts();
	}
      else
	shortcuts_tmp[it->first] = it->second;
      it++;
    }
  shortcuts.swap(shortcuts_tmp);
}

void Keyboard::parse_shortcuts()
{
  for (auto it = shortcuts.begin(); it != shortcuts.end();)
  {
    std::string mod = it->first;
    size_t i = mod.find("[");
    std::string keys = "";

    if (i != std::string::npos) {
      mod[i] = 0;
      keys = mod.substr(i + 1, mod.find("]") - (i + 1));

      std::replace(keys.begin(), keys.end(), ',', ' ');
      std::stringstream ss(keys);
      std::string tmp;
      while (ss >> tmp) {
        shortcuts.insert(std::pair<std::string, binding>(mod.data() + tmp, {it->second.name, it->second.action}));
      }
      it = shortcuts.erase(it);
    }
    else
      ++it;
  }
}

std::vector<std::string> Keyboard::split_shortcut(std::string key)
{
  std::vector<std::string> splitStr;
  size_t i = 0;
  size_t j = key.find("+");
  uint32_t sum = 0;
  uint32_t mod = 0;

  while (j != std::string::npos) {
    key[j] = 0;
    splitStr.push_back(key.data() + i);
    i = j + 1;
    j = key.find("+", j + 1);
  }
  splitStr.push_back(key.data() + i);
  return (splitStr);
}

std::string Keyboard::get_active_binding()
{
  update_shortcuts();
  for (auto const & shortcut : shortcuts) {
    std::string key = shortcut.first;
    std::vector<std::string> splitStr;
    size_t i = 0;
    size_t j = key.find("+");
    uint32_t sum = 0;
    uint32_t mod = 0;

    while (j != std::string::npos) {
      key[j] = 0;
      splitStr.push_back(key.data() + i);
      i = j + 1;
      j = key.find("+", j + 1);
    }
    splitStr.push_back(key.data() + i);

    for (std::string tmp : splitStr) {
      if (modifiersLst.find(tmp) != modifiersLst.end())
        mod |= modifiersLst[tmp];
      else
        sum += xkb_keysym_from_name(tmp.c_str(), XKB_KEYSYM_CASE_INSENSITIVE);
    }

    if (debug)
      std::cout << "Shortcuts Code: " << sum << " + " << mod << " -> " << shortcut.second.name << std::endl;

    std::cout << shortcut.first << std::endl;
    std::cout << keycodes_states.last_raw_modifiers << " " << mod << " " << sum << " " << keycodes_states.sum << std::endl;
    if (keycodes_states.last_raw_modifiers == mod && sum == keycodes_states.sum)
      return shortcut.first;
  }
  return "";
}

void Keyboard::keyboard_handle_modifiers(wl_listener *listener, void *data)
{
  wlr_seat *seat = Server::getInstance().seat.getSeat();
  wlr_seat_set_keyboard(seat, device);
  wlr_seat_keyboard_notify_modifiers(seat,
				     &device->keyboard->modifiers);
}

void Keyboard::keyboard_handle_key(wl_listener *listener, void *data)
{
  configureKeyRepeat();

  wlr_event_keyboard_key *event = static_cast<wlr_event_keyboard_key *>(data);
  wlr_seat *seat = Server::getInstance().seat.getSeat();

  uint32_t keycode = event->keycode + 8;
  const xkb_keysym_t *syms;
  int nsyms = xkb_state_key_get_syms(device->keyboard->xkb_state, keycode, &syms);
  std::string binding = "";

  bool handled = false;
  uint32_t modifiers = wlr_keyboard_get_modifiers(device->keyboard);
  keycodes_states.update_state(event, keycode, modifiers, device->keyboard->xkb_state);
  char name[30] = {0};
  //std::cout << keycode << " " <<  "'A' sym:" <<xkb_keysym_from_name("a", XKB_KEYSYM_CASE_INSENSITIVE) << " " << xkb_state_key_get_utf32(device->keyboard->xkb_state, keycode) << std::endl;
  if (event->state == WLR_KEY_PRESSED)
    {
      for (int i = 0; i < nsyms; i++)
	{
	  if (debug) {
	    xkb_keysym_get_name(syms[i], name, 30);
	    std::cout << name << ": " << syms[i] << std::endl;
	  }

	  binding = get_active_binding();
	}
    }

  if (binding.size() > 0 && device->keyboard->repeat_info.delay > 0) {
    repeatBinding = binding;
    if (wl_event_source_timer_update(key_repeat_source, device->keyboard->repeat_info.delay) < 0) {
      std::cerr << "failed to set key repeat timer" << std::endl;
    }
  }
  else if (repeatBinding.size() > 0) {
    disarm_key_repeat();
  }

  if (binding.size() > 0) {
    shortcuts[binding].action(new std::string(binding));
    handled = true;
  }

  if (!handled)
    {
      wlr_seat_set_keyboard(seat, device);
      wlr_seat_keyboard_notify_key(seat, event->time_msec,
				   event->keycode, event->state);
    }
}

void Keyboard::configureKeyRepeat()
{
  Server &server(Server::getInstance());
  if (server.configuration.consumeChanged("key repeat rate") | // no shortcircuiting to clear both flags
      server.configuration.consumeChanged("key repeat delay"))
    {
      int repeat_rate = server.configuration.getTyped<unsigned int>("key repeat rate", 25u);
      int repeat_delay = server.configuration.getTyped<unsigned int>("key repeat delay", 600u);

      wlr_keyboard_set_repeat_info(device->keyboard, repeat_rate, repeat_delay);
    }
}

void Keyboard::configure() {
  xkb_rule_names rules = { NULL, NULL, NULL, NULL, NULL };
  xkb_context *context;
  xkb_keymap *key_map;


  if (!rules.layout) {
    rules.layout = getenv("XKB_DEFAULT_LAYOUT");
  }
  if (!rules.model) {
    rules.model = getenv("XKB_DEFAULT_MODEL");
  }
  if (!rules.options) {
    rules.options = getenv("XKB_DEFAULT_OPTIONS");
  }
  if (!rules.rules) {
    rules.rules = getenv("XKB_DEFAULT_RULES");
  }
  if (!rules.variant) {
    rules.variant = getenv("XKB_DEFAULT_VARIANT");
  }

  if (!(context = xkb_context_new(XKB_CONTEXT_NO_FLAGS))) {
    std::cerr << "Cannot create the xkb context" << std::endl;
    return ;
  }
  if (!(key_map = xkb_map_new_from_names(context, &rules, XKB_KEYMAP_COMPILE_NO_FLAGS))) {
    std::cerr << "Cannot configure keyboard: keymap not found" << std::endl;
    xkb_context_unref(context);
    return ;
  }

  xkb_keymap_unref(this->keymap);
  this->keymap = key_map;
  if (debug)
    std::cout << xkb_keymap_get_as_string(this->keymap, XKB_KEYMAP_USE_ORIGINAL_FORMAT) << std::endl;
  wlr_keyboard_set_keymap(device->keyboard, this->keymap);
  xkb_context_unref(context);
  configureKeyRepeat();
}

void Keyboard::setModifiersListener()
{
  SET_LISTENER(Keyboard, KeyboardListeners, modifiers, keyboard_handle_modifiers);
  wl_signal_add(&device->keyboard->events.modifiers, &modifiers);
}

void Keyboard::setKeyListener()
{
  SET_LISTENER(Keyboard, KeyboardListeners, key, keyboard_handle_key);
  wl_signal_add(&device->keyboard->events.key, &key);
}

void Keyboard::disarm_key_repeat() {
  repeatBinding.clear();
  if (wl_event_source_timer_update(key_repeat_source, 0) < 0) {
    std::cerr << "failed to disarm key repeat timer" << std::endl;
  }
}

int Keyboard::keyboard_handle_repeat(void *data)
{
  Keyboard *k = static_cast<Keyboard *>(data);
  if (k->repeatBinding.size() > 0) {
    if (k->device->keyboard->repeat_info.rate > 0 &&  wl_event_source_timer_update(k->key_repeat_source, 1000 / k->device->keyboard->repeat_info.rate) < 0) {
      std::cerr << "failed to update key repeat timer" << std::endl;
    }
    std::cout << k->repeatBinding << std::endl;
    k->shortcuts[k->repeatBinding].action(nullptr);
  }
  return 0;
}
