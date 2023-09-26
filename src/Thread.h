#pragma once
#include <functional>

namespace cpin {
class Thread {
 public:
  Thread() = default;
  Thread(std::function<void()> f);

  void reset(std::function<void()> f);

  void run();
  void wait();

  static void Sleep(int ms);

 private:
  void* thread_handle = nullptr;
  std::function<void()> f;

  static unsigned int ThreadRunFunc(void* arg);
};

class CSLock {
 public:
  CSLock();
  ~CSLock();

  void lock();
  void unlock();

 private:
  void* cs_ptr = nullptr;
};

class Event {
 public:
  Event();
  ~Event();

  void wait();
  void signal();

 private:
  void* event_handle = nullptr;
};

class TaskManager {
 public:
  using TaskType = std::function<void()>;

  TaskManager(int tNum, int qMaxSize);
  ~TaskManager();

  // static TaskManager* Create(int tNum, int qMaxSize);

  void run();
  void stop();
  void addTask(TaskType task);

 private:
  TaskType* taskQueue = nullptr;

  static unsigned long __stdcall work(void* arg);

  int state = 0;

  int qFront = 0;
  int qRear = 0;
  int qMaxSize = 0;

  int tNum = 0;

  void* hMutex = nullptr;
  void* hEvent = nullptr;
  void* hFull = nullptr;

  void** threads = nullptr;
};
}  // namespace cpin