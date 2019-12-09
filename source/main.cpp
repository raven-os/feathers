#include "Server.hpp"
#include <getopt.h>

int main(int argc, char **argv)
{
  if (argc > 1) {
    constexpr struct option long_options[] =
    {
      {"help", no_argument, 0, 'h'},
      {"command", no_argument, 0, 'c'},
      {0, 0, 0, 0}
    };
    int option_index = 0;
    int c = getopt_long (argc, argv, "hc:",
          long_options, &option_index);

    if (c == -1)
      return -1;
    switch (c)
    {
      case 'c':
        std::cout << optarg << std::endl;
        break;
      case 'h':
        std::cout << "help" << std::endl;
        break;
      case '?':
        std::cout << "HERE" << std::endl;
        return -1;
      default:
        return -1;
    }
  }
  wlr_log_init(WLR_DEBUG, NULL);
  Server &server = Server::getInstance();
  char *command = optarg;

  server.run(command);
  return 0;
}
