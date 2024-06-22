// =====================================================================================
//
//       Filename:  l_thread.hpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-02-13 9:31:08 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#pragma once
#include "l_logger.hpp"
#include "l_sync.hpp"
#include "l_tuple.hpp"
#include "l_variant.hpp"
#include <thread>


namespace lofi {

  template<typename JobOp>
  class ThreadPool {
    using Task = JobOp;
    struct VTable {

      template<typename F>
      bool operator()(F&& f) {
        return f();
      }

      bool operator()(size_t i) {
        EVAL_PRINT_ULL(i);
        return false;
      }

    } v_table;
  public:

    ThreadPool(size_t num_threads = std::thread::hardware_concurrency()) : num_threads(MIN(num_threads, 4)) {}

    void run() {
      for(size_t i = 0; i < num_threads; i++) {
        _workers[i].init(this, i);
      }
    }


    bool terminate() {
      tasks.clear();
      for(size_t i = 0; i < num_threads; i++) { // may be unnecessary to push null tasks
        add_task((size_t)0);
      }
      for(size_t i = 0; i < NumThreads; i++) {
        _workers[i].halt();
      }
      DUMP_LOGS();
      return true;
    }

    template<typename Task>
    void add_task(Task&& t) {
      tasks.enqueue_back(FWD(t));
    }

  private:
    class Worker
    {
    public:
      Worker() {};
      void init(ThreadPool* pool, size_t _thread_id) { 
        pool_ptr = pool; 
        thread_id = _thread_id; 
        _thread = std::thread{&Worker::run_kernel, this};
      };

      void halt() {
        _thread.join();
      }

    private:
      bool get_task() {
        Arena<0, 256, 8, mem::StackAllocPolicy> scratch{};
        String str = str_cstring("Worker thread_id = %llu exited loop");
        String f_string = str_pushf(&scratch, (char*)str.str, thread_id);
        dispatcher _dispatcher{IdxV<NumThreads>
          , [&]<size_t ID>(IdxT<ID>) {
            return LOG((char*)f_string.str, Fatal);
          }
        };

        b8 to_break = false;
        while (true) {
          Task task;
          task = pool_ptr->tasks.pop_front();
          pool_ptr->sem.spin_take(); // acquire a token
          to_break = !task.visit(pool_ptr->v_table);
          pool_ptr->sem.give(); // release a token
          if(to_break) [[unlikely]]
            break;
        }
        _dispatcher(thread_id);
        return false;
      }

      void run_kernel() {
        if(!get_task())
          return;
      }

      ThreadPool *pool_ptr;
      std::thread _thread;
      size_t thread_id;
    };

    u32 num_threads{};

    semaphore<4> sem;
    SyncQueue<32, Task> tasks; // a queue to hold the tasks
    Worker _workers[4];
  };

}		// -----  end of namespace lofi  ----- 
