#include <windows.h>
#include <stdint.h>

#include "_Pool.h"

namespace cpin {
_Pool::_Pool(void* arr, int size, int ele_size)
    : max(size + 1), rear(size), ele_size(ele_size) {
  pool = new void*[size + 1];

  auto cs = new CRITICAL_SECTION;
  InitializeCriticalSection(cs);
  cs_lock = cs;

  auto addr = (uint64_t)arr;
  for (int i = 0; i < size; i++) {
    pool[i] = (void*)(addr + i * ele_size);
  }
}

_Pool::~_Pool() {
  if (pool) delete[] pool;

  if (cs_lock) {
    auto real_cs_lock = (CRITICAL_SECTION*)cs_lock;
    DeleteCriticalSection(real_cs_lock);
    delete real_cs_lock;
  }

  pool = nullptr;
  cs_lock = nullptr;
}

void* _Pool::get() {
  void* ret = nullptr;
  auto cs = (CRITICAL_SECTION*)cs_lock;

  EnterCriticalSection(cs);
  if (front != rear) {
    ret = pool[front];
    front = (front + 1) % max;
  }
  LeaveCriticalSection(cs);

  return ret;
}

void _Pool::put(void* item) {
  auto cs = (CRITICAL_SECTION*)cs_lock;

  EnterCriticalSection(cs);
  if ((rear + 1) % max != front) {
    pool[rear] = item;
    rear = (rear + 1) % max;
  }
  LeaveCriticalSection(cs);
}
}  // namespace cpin
