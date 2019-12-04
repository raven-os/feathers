#pragma once

#include "Albinos.h"
#include <memory>
#include <filesystem>
#include <unordered_map>
#include <charconv>
#include <cstring>

namespace conf
{
  class Configuration
  {
    struct ConfigDeleter
    {
      void operator()(Albinos::Config *) noexcept;
    };

    std::unique_ptr<Albinos::Config, ConfigDeleter> config;
    struct Subscription
    {
      Albinos::Subscription *subscription;
      std::string value;
      bool hasChanged;
    };

    std::unordered_map<std::string, Subscription> subscriptions;
    
  public:
    Configuration();

    /// get a setting, optimal called periodicly
    char const *get(char const *name);

    /// wrapper over get to check a boolean setting
    bool getBool(char const *name);

    /// wrapper over get to check an integer setting
    template<class Type>
    int getTyped(char const *name, Type defaultValue)
    {
      static_assert(!std::is_same_v<Type, bool> || !"Use getBool for boolean values!");
      static_assert(!std::is_same_v<Type, char const *> || !"Use get for string settings!");
      if (char const *value = get(name))
	{
	  Type result = defaultValue;
	  auto [ptr, err] = std::from_chars(value, value + std::strlen(value), result);

	  if (err == std::errc())
	    return result;
	  return defaultValue;
	}
      return defaultValue;
    }

    /// get a setting only once, optimal when called rarely
    std::string getOnce(char const *name);

    /// returns if a setting has changed and put it's hasChanged flag to false
    bool consumeChanged(char const *name) noexcept;

    /// poll for updates
    void poll();
  };
}
