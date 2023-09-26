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

TaskManager::TaskManager(int tNum, int qMaxSize)
    : tNum(tNum), qMaxSize(qMaxSize) {
  taskQueue = new TaskType[qMaxSize];
  threads = new HANDLE[tNum];

  hMutex = CreateMutex(nullptr, false, nullptr);
  hEvent = CreateEvent(nullptr, true, false, nullptr);
  hFull = CreateEvent(nullptr, true, false, nullptr);
}

unsigned long __stdcall TaskManager::work(void* arg) {
  auto mgr = static_cast<TaskManager*>(arg);
  TaskType task;

  while (true) {
    WaitForSingleObject(mgr->hEvent, INFINITE);
    if (!mgr->state) break;
    WaitForSingleObject(mgr->hMutex, INFINITE);

    if (mgr->qFront != mgr->qRear) {
      task = mgr->taskQueue[mgr->qFront];
      mgr->qFront = (mgr->qFront + 1) % mgr->qMaxSize;

      SetEvent(mgr->hFull);
      ReleaseMutex(mgr->hMutex);
      if (task) task();

    } else {
      ReleaseMutex(mgr->hMutex);
    }
  }

  return 0;
}

void TaskManager::run() {
  if (state) return;

  state = 1;
  for (int i = 0; i < tNum; i++) {
    threads[i] = CreateThread(nullptr, 0, work, this, 0, nullptr);
  }
}

void TaskManager::addTask(TaskType task) {
  if (!state) return;

  WaitForSingleObject(hMutex, INFINITE);
  while ((qRear + 1) % qMaxSize == qFront) {
    ReleaseMutex(hMutex);
    WaitForSingleObject(hFull, INFINITE);
    WaitForSingleObject(hMutex, INFINITE);
  }

  taskQueue[qRear] = task;
  qRear = (qRear + 1) % qMaxSize;

  ReleaseMutex(hMutex);
  SetEvent(hEvent);
}

void TaskManager::stop() {
  if (!state) return;

  state = 0;
  for (int i = 0; i < tNum; i++) {
    SetEvent(hEvent);
  }

  WaitForMultipleObjects(tNum, threads, true, INFINITE);
}

TaskManager::~TaskManager() {
  stop();
  if (taskQueue) delete[] taskQueue;
  if (threads) delete[] threads;

  taskQueue = nullptr;
  threads = nullptr;

  CloseHandle(hMutex);
  CloseHandle(hEvent);
  CloseHandle(hFull);
}

}  // namespace cpin