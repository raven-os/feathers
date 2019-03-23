#pragma once

namespace Listeners {

  #define SET_LISTENER(TYPE, SUBTYPE, NAME, TARGET)			\
    NAME.notify = [](wl_listener *listener, void *data)			\
  		{							\
  		  TYPE *that(static_cast<TYPE *>(wl_container_of(listener, static_cast<SUBTYPE *>(nullptr), NAME))); \
  		  that->TARGET(listener, data);				\
  		};							\

}
