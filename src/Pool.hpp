#pragma once
#include "_Pool.h"

namespace cpin {

template <typename T>
class Pool {
 public:
  Pool(int size) : Pool(new T[size], size) { array_need_delete = true; }

  Pool(T* array, int size)
      : pool(new _Pool(array, size, sizeof(T))), array(array) {}

  ~Pool() {
    if (pool) delete pool;
    if (array_need_delete && array) delete[] array;

    pool = nullptr;
    array = nullptr;
  }

  T* get() { return (T*)(pool->get()); }

  void put(T* item) { pool->put(item); }

 private:
  bool array_need_delete = false;

  T* array = nullptr;
  _Pool* pool = nullptr;
};

}  // namespace cpin