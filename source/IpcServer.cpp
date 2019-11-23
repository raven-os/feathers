#include "IpcServer.hpp"
#include "Server.hpp"
#include "Output.hpp"

#include <iostream>
#include <unistd.h>

IpcServer::IpcServer(std::string const& socket, Server *server)
  : ipcServer(socket), socket(socket), server(server), thread(&IpcServer::accept, this)
{
}

void IpcServer::accept()
{
  try {
    while (1)
      {
        std::unique_ptr<libsocket::unix_stream_client> client = ipcServer.accept2();

        int size = server->outputManager.workspaceCount;
        int activeWorkspaceId = findActiveWorkspaceId();
        sendData(client.get(), size, activeWorkspaceId);

        while (1)
          {
            usleep(100);
            int newSize = server->outputManager.workspaceCount;
            int newActiveWorkspaceId = findActiveWorkspaceId();
            if (newSize != size || activeWorkspaceId != newActiveWorkspaceId)
              {
                size = newSize;
                activeWorkspaceId = newActiveWorkspaceId;
                sendData(client.get(), size, activeWorkspaceId);
              }
          }
      }
  } catch (const libsocket::socket_exception& e) {
    std::cerr << e.mesg;
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

int IpcServer::findActiveWorkspaceId() const
{
  int i = 0;
  for (auto const& w : server->outputManager.getOutputs().at(0)->getWorkspaces())
    {
      if (w.get() == server->outputManager.getActiveWorkspace())
        {
          break;
        }
      ++i;
    }
  return i + 1; // from index to actual number of the workspace
}
