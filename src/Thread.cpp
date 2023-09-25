#include <windows.h>
#include <process.h>

#include "Thread.h"

namespace cpin {
Thread::Thread(std::function<void()> f) : f(f) {}

void Thread::run() {
  if (thread_handle) return;
  thread_handle =
      (void*)_beginthreadex(nullptr, 0, ThreadRunFunc, this, 0, nullptr);
}

void Thread::wait() {
  if (!thread_handle) return;
  WaitForSingleObject(thread_handle, INFINITE);
}

void Thread::reset(std::function<void()> f) { this->f = f; }

void Thread::Sleep(int ms) { ::Sleep(ms); }

unsigned int Thread::ThreadRunFunc(void* arg) {
  auto ptr = (Thread*)arg;
  if (ptr && ptr->f) ptr->f();
  return 0;
}

CSLock::CSLock() {
  auto cs = new CRITICAL_SECTION;
  InitializeCriticalSection(cs);
  cs_ptr = cs;
}

CSLock::~CSLock() {
  if (!cs_ptr) return;
  DeleteCriticalSection((CRITICAL_SECTION*)cs_ptr);
  cs_ptr = nullptr;
}

void CSLock::lock() {
  if (cs_ptr) EnterCriticalSection((CRITICAL_SECTION*)cs_ptr);
}

void CSLock::unlock() {
  if (cs_ptr) LeaveCriticalSection((CRITICAL_SECTION*)cs_ptr);
}

Event::Event() { event_handle = CreateEvent(nullptr, true, false, nullptr); }

Event::~Event() {
  if (!event_handle) return;
  CloseHandle(event_handle);
  event_handle = nullptr;
}

void Event::wait() {
  if (event_handle) WaitForSingleObject(event_handle, INFINITE);
}

void Event::signal() {
  if (event_handle) SetEvent(event_handle);
}
}  // namespace cpin