#pragma once

namespace cpin {
// 前置声明, 减少依赖
template <typename T>
class Pool;
class TaskManager;
class Event;

using SocketType = unsigned long long;

class Conn {
 public:
  SocketType client_socket;
  void* client_address = nullptr;

  Conn();
  ~Conn();
};

class Buffer {};

class Server {
 public:
  Server(unsigned short port = 8080);

  int start();

 private:
  int process_num = 0;
  SocketType server_socket = 0;

  void* iocp_port = nullptr;
  void* server_address = nullptr;

  Pool<Buffer>* buffer_pool = nullptr;
  Pool<Conn>* conn_pool = nullptr;
  TaskManager* task_manager = nullptr;
  Event* cp_not_empty = nullptr;

  void** threads = nullptr;

 private:
  static unsigned WorkThread(void* arg);
};
}  // namespace cpin