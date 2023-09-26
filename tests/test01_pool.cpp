#include "Thread.h"
#include "Pool.hpp"

class Data {
 public:
  Data() = default;

  void reset(int data) { this->data = data; }
  int get_data() const { return data; }

 private:
  int data = 0;
};

class ThreadContext {
 public:
  ThreadContext() = default;

  void reset(int data, cpin::CSLock* lock, cpin::Event* main_event,
             cpin::Event* sub_event) {
    this->data = data;
    this->lock = lock;
    this->main_event = main_event;
    this->sub_event = sub_event;
  }

  void work(cpin::Pool<Data>* pool, bool* awake) {
    if (!lock || !main_event || !sub_event || !pool || !awake) return;

    // 尝试获取 Data 对象
    auto ret = pool->get();

    // 打印返回值状况
    if (ret) {
      printf("[%d]\tGet Result: %d\n", data, ret->get_data());
    } else {
      printf("[%d]\tGet Result: null\n", data);
    }

    // 如果出现 ret 为空的情况，通知主线程
    if (!ret) {
      lock->lock();
      if (!(*awake)) {
        main_event->signal();
        *awake = true;
      }
      lock->unlock();
      return;
    }

    // 等待主线程通知
    sub_event->wait();

    // 如果获取到 Data 对象，则重新放回到 pool 中
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
  bool awake = false;

  cpin::Thread thrs[thread_num];
  ThreadContext a_array[thread_num];
  Data t_array[thread_num / 2];

  cpin::CSLock lock;
  cpin::Event main_event;
  cpin::Event sub_event;

  cpin::Pool<Data> t_pool(t_array, thread_num / 2);

  for (int i = 0; i < thread_num / 2; i++) {
    t_array[i].reset(i + 1);
  }

  for (int i = 0; i < thread_num; i++) {
    a_array[i].reset(i + 1, &lock, &main_event, &sub_event);
    thrs[i].reset(std::bind(ThreadContext::work, &a_array[i], &t_pool, &awake));
    thrs[i].run();
  }

  // 获取到 pool 池为空的通知
  main_event.wait();

  // 等待 500 毫秒
  cpin::Thread::Sleep(500);

  printf("main awake...\n");

  // 通知获取到 Data 对象的线程
  for (int i = 0; i < thread_num / 2; i++) {
    sub_event.signal();
  }

  // 等待所有线程运行结束
  for (int i = 0; i < thread_num; i++) thrs[i].wait();
  printf("end...\n");
}

void test03() {
  cpin::TaskManager manager(4, 40);

  manager.run();

  auto task = [](int arg) {
    printf("[%d] ", arg);
    cpin::Thread::Sleep(500);
  };

  for (int i = 0; i < 40; i++) {
    manager.addTask(std::bind(task, i + 1));
  }

  for (int i = 0; i < 5; i++) {
    cpin::Thread::Sleep(1000);
    printf("\n[main]\t%ds\n", i + 1);
  }
}

int main(int argc, char const* argv[]) {
  printf("test01...\n");
  test01();
  printf("\ntest02...\n");
  test02();
  printf("\ntest03...\n");
  test03();
  return 0;
}