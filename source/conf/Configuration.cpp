#include "conf/Configuration.hpp"

#include <fstream>
#include <cassert>
#include <iostream>
#include <vector>

namespace conf
{
  void Configuration::ConfigDeleter::operator()(Albinos::Config *config) noexcept
  {
    if (config)
      Albinos::releaseConfig(config);
  }

  namespace
  {
    struct DestroyConfig
    {
      void operator()(Albinos::Config *config) noexcept
      {
	Albinos::destroyConfig(config);
      }
    };
  }

  Configuration::Configuration()
  {
    std::filesystem::path configKeyPath = "configKey.txt";
    std::filesystem::path configReadOnlyKeyPath = "configReadOnlyKey.txt";

    if (std::filesystem::exists(configKeyPath))
      {
	Albinos::Config *tmp = nullptr;
	std::ifstream file(configKeyPath, std::ios::in | std::ios::binary | std::ios::ate);

	auto size = file.tellg();
	file.seekg(0, std::ios::beg);
	std::vector<char> data(size);

        file.read(data.data(), size);
	Albinos::Key const key{
			       .data = data.data(),
			       .size = size_t(size),
			       .type = Albinos::READ_WRITE
	};
	if (Albinos::getConfig(key, &tmp) != Albinos::SUCCESS)
	  std::cerr << "WARNING: failed to load config" << std::endl;
	config.reset(tmp);
      }
    else
      {
	std::unique_ptr<Albinos::Config, DestroyConfig> tmp;
	{
	  Albinos::Config *tmp2 = nullptr;
	  if (Albinos::createConfig("feathers", &tmp2) != Albinos::SUCCESS)
	    {
	      std::cerr << "WARNING: failed to create config" << std::endl;
	      return ;
	    }
	  tmp.reset(tmp2);
	}

	Albinos::Key key;

	Albinos::getConfigKey(tmp.get(), &key);
	{
	  std::ofstream file(configKeyPath, std::ios::out);

	  file.write(key.data, key.size);
	}
	Albinos::getReadOnlyConfigKey(tmp.get(), &key);
	{
	  std::ofstream file(configReadOnlyKeyPath, std::ios::out);

	  file.write(key.data, key.size);
	}
	config.reset(tmp.release());
      }
  }

  char const *Configuration::get(char const *name)
  {
    try {
      return subscriptions.at(name).value.c_str();
    } catch (std::out_of_range const &) {
      auto &subscription(subscriptions[name]);

      Albinos::subscribeToSetting(config.get(), name, this,
				  [](Albinos::Subscription const *rawSubscription, Albinos::ModifType type)
				  {
				    auto *this_(static_cast<decltype(this)>(Albinos::getSupscriptionUserData(rawSubscription)));
				    auto name(Albinos::getSupscriptionSettingName(rawSubscription));
				    auto &subscription(this_->subscriptions.at(name));

				    subscription.hasChanged = true;
				    switch (type)
				      {
				      case Albinos::UPDATE:
					{
					  size_t size;

					  Albinos::getSettingSize(this_->config.get(), name, &size);
					  subscription.value.resize(size);
					  Albinos::getSettingValue(this_->config.get(), name, subscription.value.data(), size);
					}
					break;
				      case Albinos::DELETE:
					{
					  subscription.value = "";
					}
					break;
				      default:
					assert(!"Unknown update type");
				      }
				  }, &subscription.subscription);
      size_t size;

      if (Albinos::getSettingSize(config.get(), name, &size) != Albinos::SUCCESS)
	return (subscription.value = "").c_str();
      subscription.value.resize(size);
      if (Albinos::getSettingValue(config.get(), name, subscription.value.data(), size) != Albinos::SUCCESS)
	subscription.value = "";
      return subscription.value.c_str();
    }
  }

  std::string Configuration::getOnce(char const *name)
  {
    size_t size;
    std::string value;

    if (Albinos::getSettingSize(config.get(), name, &size) != Albinos::SUCCESS)
      return {};
    value.resize(size);
    if (Albinos::getSettingValue(config.get(), name, value.data(), size) != Albinos::SUCCESS)
      return {};
    return value;
  }

  bool Configuration::consumeChanged(char const *name) noexcept
  {
    try {
      bool result;
      if (result = subscriptions.at(name).hasChanged)
	subscriptions.at(name).hasChanged = false;
      return result;
    } catch (std::out_of_range const &) {
      return true;
    }
  }

  void Configuration::poll()
  {
    pollSubscriptions(config.get());
  }
}
