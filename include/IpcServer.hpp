#pragma once

#include <vector>
#include <memory>
#include <thread>
#include <mutex>
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
  void acceptClients();
  void processClients();
  void sendData(libsocket::unix_stream_client *client, int size, int activeWorkspaceId);
  int findActiveWorkspaceId() const noexcept;

private:
  libsocket::unix_stream_server ipcServer;
  std::string socket;
  Server *server;
  std::vector<std::unique_ptr<libsocket::unix_stream_client>> clients;
  std::thread acceptThread;
  std::thread processThread;
  std::mutex mutex;
};
