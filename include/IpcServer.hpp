#pragma once

#include <vector>
#include <memory>
#include <thread>
#include <libsocket/unixserverstream.hpp>
#include <libsocket/unixclientstream.hpp>
#include <libsocket/exception.hpp>
#include "Workspace.hpp"

class Server;

class IpcServer
{
public:
  IpcServer(std::string const& socket, Server *server);
  ~IpcServer() = default;

private:
  void accept();
  void sendData(libsocket::unix_stream_client *client, int size, int activeWorkspaceId);
  int findActiveWorkspaceId() const;

private:
  libsocket::unix_stream_server ipcServer;
  std::string socket;
  Server *server;
  std::thread thread;
};
