#pragma once

namespace cpin {

class _Pool {
 private:
  int front = 0;
  int rear = 0;
  int max = 0;
  int ele_size = 0;

  void** pool = nullptr;
  void* cs_lock = nullptr;

 public:
  _Pool(void* arr, int size, int ele_size);
  ~_Pool();

  void* get();
  void put(void* item);
};

}  // namespace cpin