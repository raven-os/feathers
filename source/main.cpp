#include "Server.hpp"

int main()
{
  wlr_log_init(WLR_DEBUG, NULL);
  Server &server = Server::getInstance();

  server.run();
  return 0;
}
