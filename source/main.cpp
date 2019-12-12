#include "Server.hpp"
#include <getopt.h>

void help() {
  std::cout << "usage: feathers " 
  << "[--help | -h] "
  << "[--command | -c <command_name>]\n"
  << "\t\t[--open-type | -o <open value>] (0: tilling / 1: bellow / 2: right / 3: floating)" 
  << std::endl;
}

int main(int argc, char **argv)
{
  char *command;
  OpenType openType;

  if (argc > 1) {
    constexpr struct option long_options[] =
    {
      {"help", no_argument, 0, 'h'},
      {"command", no_argument, 0, 'c'},
      {"open-type", no_argument, 0, 'o'},
      {0, 0, 0, 0}
    };
  while (1) {
    int option_index = 0;
    int c = getopt_long (argc, argv, "hc:o:",
          long_options, &option_index);

    if (c == -1)
      break;
    switch (c)
    {
      case 'c':
        command = optarg;
        break;
      case 'h':
        help();
        return 0;
      case 'o':
        openType = static_cast<OpenType>(atoi(optarg));
        break;
      case '?':
        break ;
      default:
        return -1;
    }
  }
  }
  wlr_log_init(WLR_DEBUG, NULL);
  Server &server = Server::getInstance();
  server.openType = openType;
  server.run(command);
  return 0;
}
