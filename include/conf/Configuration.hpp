#pragma once

#include "Albinos.h"
#include <memory>
#include <filesystem>
#include <unordered_map>

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

    /// get a setting only once, optimal when called rarely
    std::string getOnce(char const *name);

    /// returns if a setting has changed and put it's hasChanged flag to false
    bool consumeChanged(char const *name) noexcept;

    /// poll for updates
    void poll();
  };
}
