#include "Server.hpp"

int main()
{
  wlr_log_init(WLR_DEBUG, NULL);
  Server server;

  server.run();
  return 0;
}
