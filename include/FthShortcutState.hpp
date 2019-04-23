#pragma once

#include <vector>
#include <algorithm>
# include "Wlroots.hpp"

#include <iostream>


#define FTH_LIMIT_KEY_PRESSED 32

struct FthShortcutState {
  //keyId , Keycode
  std::vector<std::pair<uint32_t, uint32_t>> pressed_key;
  //std::pair<uint32_t, uint32_t> pressed_key[FTH_LIMIT_KEY_PRESSED];
  size_t npressed = 0;
  uint32_t last_raw_modifiers = 0;
  uint32_t last_keycode = 0;
	uint32_t current_key = 0;

  void update_state(struct wlr_event_keyboard_key *event, uint32_t new_key,	uint32_t raw_modifiers) {
    bool last_was_modifier = raw_modifiers != last_raw_modifiers;
	  last_raw_modifiers = raw_modifiers;

  	if (last_was_modifier && last_keycode) {
  		// Last pressed key before this one was a modifier
      std::cout << "LAST WAS MODIFIER" << std::endl;
  		remove_key(last_keycode);
  	}

    if (event->state == WLR_KEY_PRESSED) {
      // Add current key to set; there may be duplicates
      add_key(event->keycode, new_key);
      last_keycode = event->keycode;
    } else {
       remove_key(event->keycode);
    }
    display_vector();
  }


private:
  void add_key(uint32_t keycode, uint32_t key_id) {
    auto begin = pressed_key.begin();

    if (npressed < FTH_LIMIT_KEY_PRESSED) {
      size_t i = static_cast<int>(std::distance(begin, std::find_if(begin, pressed_key.end(),
                [key_id](const std::pair<uint32_t, uint32_t> &key) {
                  return key.first == key_id;
                })));
      std::cout << i << " " << npressed << std::endl;
      if (i < npressed)
         pressed_key[i] = {key_id, keycode};
      else {
         pressed_key.insert(begin + i, std::make_pair(key_id, keycode));
      // size_t i = 0;
    	// while (i < npressed && pressed_key[i].first < key_id) {
    	// 	++i;
    	// }
    	// size_t j = npressed;
    	// while (j > i) {
    	// 	pressed_key[j] = pressed_key[j - 1];
    	// 	--j;
    	// }
    	// pressed_key[i] = {key_id, keycode};
    	 ++npressed;
      }
      current_key = key_id;
    }
  }

  void remove_key(uint32_t keycode) {
    auto begin = pressed_key.begin();
    //
    size_t i = static_cast<size_t>(std::distance(begin, std::find_if(begin, pressed_key.end(),
              [keycode](const std::pair<uint32_t, uint32_t> &key) {
                return key.second == keycode;
              })));
    while (npressed > i) {
      --npressed;
      pressed_key.erase(begin + npressed);
    }
    // size_t j = 0;
  	// for (size_t i = 0; i < npressed; ++i) {
  	// 	if (i > j) {
  	// 		pressed_key[j] = pressed_key[i];
  	// 	}
  	// 	if (pressed_key[i].second != keycode) {
  	// 		++j;
  	// 	}
  	// }
  	// while(npressed > j) {
  	// 	--npressed;
  	// 	pressed_key[npressed] = {0, 0};
  	// }
    current_key = 0;
  }

  void display_vector() {
    for (size_t i = 0; i < npressed; ++i) {
      std::cout << "KEYID: " << pressed_key[i].first << " KEYCODE: " << pressed_key[i].second << std::endl;
    }
    std::cout << std::endl;
  }

};
