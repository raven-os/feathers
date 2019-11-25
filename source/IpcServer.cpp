#include "IpcServer.hpp"
#include "Server.hpp"
#include "Output.hpp"

#include <iostream>
#include <unistd.h>
#include <signal.h>

IpcServer::IpcServer(std::string const& socket, Server *server)
  : ipcServer(socket)
  , socket(socket)
  , server(server)
  , clients()
  , acceptThread(&IpcServer::acceptClients, this)
  , processThread(&IpcServer::processClients, this)
  , mutex()
{
  // Ignore broken pipe
  signal(SIGPIPE, SIG_IGN);
}

void IpcServer::acceptClients()
{
  while (1)
    {
      try {
        std::unique_ptr<libsocket::unix_stream_client> client = ipcServer.accept2();
        std::cout << "New ipc client" << std::endl;
        mutex.lock();
        clients.emplace_back(std::move(client));
        mutex.unlock();

      } catch (const libsocket::socket_exception& e) {
        std::cerr << e.mesg;
      }
    }
}

void IpcServer::processClients()
{
  int size = -1;
  int activeWorkspaceId = -1;
  unsigned int clientSize = -1;
  while (1)
    {
      usleep(100);
      if (clients.empty())
        continue;
      int newSize = server->outputManager.workspaceCount;
      int newActiveWorkspaceId = findActiveWorkspaceId();
      unsigned int newClientSize = clients.size();
      if (newSize != size || activeWorkspaceId != newActiveWorkspaceId || clientSize != newClientSize)
        {
          size = newSize;
          activeWorkspaceId = newActiveWorkspaceId;
          clientSize = newClientSize;
          mutex.lock();
          for (auto it = clients.begin(); it != clients.end();)
            {
              try {
                sendData((*it).get(), size, activeWorkspaceId);
                ++it;
              } catch (const libsocket::socket_exception& e) {
                // remove closed client
                std::cerr << e.mesg;
                it = clients.erase(it);
              }
            }
          mutex.unlock();
        }
    }
}

void IpcServer::sendData(libsocket::unix_stream_client *client, int size, int activeWorkspaceId)
{
  for (int i = 1; i < size + 1; ++i)
    {
      if (i != 1)
        {
          *client << " |";
        }
      if (i == activeWorkspaceId)
        {
          *client << " [" << std::to_string(i) << "]";
        }
      else
        {
          *client << " " << std::to_string(i);
        }
    }
  *client << "\n";
}

int IpcServer::findActiveWorkspaceId() const noexcept
{
  int i = 0;
  for (auto const& w : server->outputManager.getOutputs().at(0)->getWorkspaces())
    {
      if (w.get() == server->outputManager.getActiveWorkspace())
        {
          return i + 1; // from index to actual number of the workspace
        }
      ++i;
    }
  std::cerr << "No active workspace found" << std::endl;
  return -1;
}
