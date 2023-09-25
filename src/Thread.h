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
}  // namespace cpin