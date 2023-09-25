#include "Thread.h"
#include "Pool.hpp"

#include <windows.h>

class T {
 public:
  T() = default;

  void reset(int data) { this->data = data; }
  int get_data() const { return data; }

 private:
  int data = 0;
};

class A {
 public:
  A(int data = 0) : data(data) {}

  void reset(int data, cpin::CSLock* lock, cpin::Event* main_event,
             cpin::Event* sub_event) {
    this->data = data;
    this->lock = lock;
    this->main_event = main_event;
    this->sub_event = sub_event;
  }

  void work(cpin::Pool<T>* pool, bool* awake) {
    if (!lock || !main_event || !sub_event || !pool || !awake) return;

    auto ret = pool->get();

    if (ret) {
      printf("[%d]\tGet Result: %d\n", data, ret->get_data());
    } else {
      printf("[%d]\tGet Result: null\n", data);
    }

    if (!ret) {
      lock->lock();
      if (!(*awake)) {
        main_event->signal();
        *awake = true;
      }
      lock->unlock();
      return;
    }

    sub_event->wait();

    printf("[%d]\t Put %d\n", data, ret->get_data());
    pool->put(ret);
  }

 private:
  int data;

  cpin::CSLock* lock = nullptr;
  cpin::Event* main_event = nullptr;
  cpin::Event* sub_event = nullptr;
};

void test01() {
  constexpr int array_size = 10;
  int ele_size = sizeof(int);

  int array[array_size];
  void* ptr_array[array_size];

  for (int i = 0; i < array_size; i++) {
    array[i] = i + 1;
  }

  auto addr = (uint64_t)array;
  for (int i = 0; i < array_size; i++) {
    ptr_array[i] = (void*)(addr + i * ele_size);
  }

  int* iptr = nullptr;
  for (int i = 0; i < array_size; i++) {
    iptr = (int*)ptr_array[i];
    printf("[%d]\t%p %p\n", *iptr, &array[i], ptr_array[i]);
  }
}

void test02() {
  constexpr int thread_num = 10;
  cpin::Thread thrs[thread_num];
  A a_array[thread_num];

  T t_array[thread_num / 2];
  for (int i = 0; i < thread_num / 2; i++) {
    t_array[i].reset(i + 1);
  }

  cpin::CSLock lock;
  cpin::Event main_event;
  cpin::Event sub_event;

  cpin::Pool<T> t_pool(t_array, thread_num / 2);

  bool awake = false;

  for (int i = 0; i < thread_num; i++) {
    a_array[i].reset(i + 1, &lock, &main_event, &sub_event);
    thrs[i].reset(std::bind(A::work, &a_array[i], &t_pool, &awake));
    thrs[i].run();
  }

  main_event.wait();

  cpin::Thread::Sleep(500);

  printf("main awake...\n");

  for (int i = 0; i < thread_num / 2; i++) {
    sub_event.signal();
  }

  for (int i = 0; i < thread_num; i++) thrs[i].wait();
  printf("end...\n");
}

int main(int argc, char const* argv[]) {
  // test01();
  test02();
  system("pause");
  // printf("end...\n");
  return 0;
}