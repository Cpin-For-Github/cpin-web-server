
#include "Server.h"

#include <windows.h>
#include <winsock2.h>

#include "Pool.hpp"
#include "Thread.h"

namespace cpin {

Conn::Conn() : client_socket(INVALID_SOCKET) {
  auto addr = new SOCKADDR_IN;
  memset(addr, 0, sizeof(SOCKADDR_IN));
  client_address = addr;
}

Conn::~Conn() {
  auto real_client_addr = (SOCKADDR_IN*)client_address;
  if (real_client_addr) delete real_client_addr;

  client_address = nullptr;
  client_socket = INVALID_SOCKET;
}

Server::Server(unsigned short port) {
  SYSTEM_INFO si;
  GetSystemInfo(&si);
  process_num = si.dwNumberOfProcessors;

  threads = new HANDLE[process_num];

  auto addr = new SOCKADDR_IN;
  memset(&addr, 0, sizeof(SOCKADDR_IN));
  addr->sin_family = AF_INET;
  addr->sin_addr.s_addr = htonl(INADDR_ANY);
  addr->sin_port = port;

  server_address = addr;
  buffer_pool = new Pool<Buffer>(process_num * 2);
  conn_pool = new Pool<Conn>(process_num * 2);
  task_manager = new TaskManager(process_num, process_num * 2);
  cp_not_empty = new Event;
}

int Server::start() {
  WSADATA data;
  WSAStartup(MAKEWORD(2, 2), &data);

  iocp_port = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
  if (!iocp_port) {
    // TODO: Error Log
    printf("create iocp failed\n");
    return 1;
  }

  server_socket =
      WSASocket(AF_INET, SOCK_STREAM, 0, nullptr, 0, WSA_FLAG_OVERLAPPED);
  if (server_socket == INVALID_SOCKET) {
    // TODO: Error Log
    printf("get invalid socket\n");
    return 2;
  }

  int ret = 0;
  ret = bind(server_socket, (SOCKADDR*)server_address, sizeof(SOCKADDR));
  if (ret == SOCKET_ERROR) {
    // TODO: Error Log
    printf("bind server error\n");
    return 3;
  }

  ret = listen(server_socket, SOMAXCONN);
  if (ret == SOCKET_ERROR) {
    // TODO: Error Log
    printf("listen server error\n");
    return 4;
  }

  for (int i = 0; i < process_num; i++) {
    threads[i] =
        (HANDLE)_beginthreadex(nullptr, 0, WorkThread, this, 0, nullptr);
  }

  // auto real_conn_pool = (Pool<Conn>*)conn_pool;
  // auto real_buffer_pool = (Pool<Buffer>*)buffer_pool;

  Conn* conn = nullptr;
  int addrLen = sizeof(SOCKADDR);

  while (true) {
    while (nullptr == (conn = conn_pool->get())) {
      cp_not_empty->wait();
    }

    conn->client_socket =
        accept(server_socket, (SOCKADDR*)conn->client_address, &addrLen);
    CreateIoCompletionPort((HANDLE)(conn->client_address), iocp_port, 0, 0);
  }

  WaitForMultipleObjects(process_num, threads, true, INFINITE);
  return 0;
}

unsigned Server::WorkThread(void* arg) {
  if (!arg) return;
  auto serv = (Server*)arg;
}

}  // namespace cpin
