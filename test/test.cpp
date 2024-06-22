// =====================================================================================
//
//       Filename:  test.cpp
//
//    Description:  
//
//        Version:  1.0
//        Created:  2024-02-02 11:23:33 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  Roxi Graves (rg), nada
//   Organization:  Roxi Psychotronics
//
// =====================================================================================
#include <chrono>
#include <iostream>
#include <stdio.h>
#include <thread>
#include <vector>
#define LOFI_DEFAULT_BUCKETS_COUNT 4
#include "../core/include/l_tuple.hpp"
#include "../core/include/l_variant.hpp"
#include "../core/include/l_memory.hpp"
#include "../core/include/l_thread.hpp"

static constexpr u64 NumThreads = 4;

static constexpr u64 NumFibers = 128;

static u64 values[256];


DEFINE_JOB_SUCCESS(standard_job_success) {
  //PRINT("entering standard_job_success counter ptr = %llu\n", PTR2INT(counter));
  lofi::atomic_counter<>* a_counter = (lofi::atomic_counter<>*) counter;
  ++(*a_counter);
  //PRINT("success counter = %llu\n", (*a_counter).get_count());
}

DEFINE_JOB_FAILURE(standard_job_failure) {
  //PRINT_S("print job failed!\n");
}


DEFINE_JOB_FAILURE(print_job_failure) {
  //PRINT_S("print job failed!\n");
}

DEFINE_JOB(print_index) {
  using pool_t = lofi::ThreadPool<NumThreads, NumFibers>;
  using worker_t = typename pool_t::Worker;

  pool_t* thread_pool = (pool_t*)param;
  worker_t* worker = thread_pool->get_host_worker();

  //PRINT("thread_id: %llu entering print_index job\n", worker->get_thread_id());
  if(start == end)
    return false;
  for(size_t print_job_index = start; print_job_index < end; print_job_index++) {
    values[print_job_index] = print_job_index;
    EVAL_PRINT_ULL(print_job_index);
  }
  //PRINT("thread_id: %llu exiting print_index job\n", worker->get_thread_id());
  return true;
}

DEFINE_JOB(subinit_job) {
  using pool_t = lofi::ThreadPool<NumThreads, NumFibers>;
  using worker_t = lofi::ThreadPool<NumThreads, NumFibers>::Worker;
  pool_t* thread_pool  = (pool_t*)param;
  worker_t* worker = thread_pool->get_host_worker();
  size_t thread_id = worker->get_thread_id();
  //PRINT("thread %llu entering init_job\n", thread_id);
  lofi::atomic_counter<> counter(0);
  //PRINT("init job counter ptr = %llu\n", PTR2INT(&counter));
  const size_t job_size = 8;
  const size_t num_jobs = 8;
  const size_t start_index = start * num_jobs * job_size;
  for(size_t i = 0; i < num_jobs; i++) {
    lofi::Job new_job;
    new_job.set_table( 
        { print_index
        , standard_job_success
        , print_job_failure
        , i * job_size + start_index
        , (i * job_size) + job_size + start_index
        , &counter
        });
        
    //PRINT("job entry point = %llu\n", PTR2INT((void*)new_job.entry_point));
    worker->kick_high_priority_job(new_job);
  }
  worker->fiber_wait(&counter, num_jobs);
  //worker = thread_pool->get_host_worker();
  //thread_id = worker->get_thread_id();
  //PRINT("thread %llu exiting after waking in init job\n", thread_id);
  return true;
}

DEFINE_JOB(init_job) {
  using pool_t = lofi::ThreadPool<NumThreads, NumFibers>;
  using worker_t = lofi::ThreadPool<NumThreads, NumFibers>::Worker;
  pool_t* thread_pool  = (pool_t*)param;
  worker_t* worker = thread_pool->get_host_worker();
  size_t thread_id = worker->get_thread_id();
  //PRINT("thread %llu entering init_job\n", thread_id);
  lofi::atomic_counter<> counter(0);
  //PRINT("init job counter ptr = %llu\n", PTR2INT(&counter));
  const size_t num_jobs = 4;
  for(size_t i = 0; i < num_jobs; i++) {
    lofi::Job new_job;
    new_job.set_table( 
        { subinit_job
        , standard_job_success
        , standard_job_failure
        , i
        , i + 1
        , &counter
        });
    //PRINT("job entry point = %llu\n", PTR2INT((void*)new_job.entry_point));
    worker->kick_high_priority_job(new_job);
  }
  worker->fiber_wait(&counter, num_jobs);
  //worker = thread_pool->get_host_worker();
  //thread_id = worker->get_thread_id();
  //PRINT("thread %llu exiting after waking in init job\n", thread_id);
  return true;
}

int main(int argc, char** argv) {
//--------------------------------------------------------------------------------------------
  PRINT_TITLE("OS AND COMPILER");
//--------------------------------------------------------------------------------------------
  if(OS_WINDOWS) {
    PRINT_S("WINDOWS\n");
  } else if (OS_LINUX) {
    PRINT_S("LINUX\n");
  } else if (OS_MAC) {
    PRINT_S("MAC\n");
  } else {
    PRINT_S("OS NOT FOUND\n");
  }
  if(COMPILER_CL) {
    PRINT_S("MSVC\n");
  } else if(COMPILER_CLANG) {
    PRINT_S("CLANG\n");
  } else if(COMPILER_GCC) {
    PRINT_S("GCC\n");
  } else {
    PRINT_S("COMPILER NOT FOUND\n");
  }

//--------------------------------------------------------------------------------------------
  PRINT_TITLE("TUPLES");
//--------------------------------------------------------------------------------------------

  lofi::tuple<int, float, double, int> t0;
  lofi::tuple<float, double, int> t1;

  lofi::meta::get<0>(t0).elem = 4;
  lofi::meta::get<1>(t0).elem = 4.20f;
  lofi::meta::get<2>(t0).elem = 4.24;
  lofi::meta::get<3>(t0).elem = 420;

  PRINT("t0[0] = %d\n", lofi::meta::get<0>(t0).elem);
  PRINT("t0[1] = %f\n", lofi::meta::get<1>(t0).elem);
  PRINT("t0[2] = %f\n", lofi::meta::get<2>(t0).elem);
  PRINT("t0[3] = %d\n", lofi::meta::get<3>(t0).elem);

  t0.get<0>() = 69;
  t0.get<1>() = 6.9;
  t0.get<2>() = 6.969;
  t0.get<3>() = 6969;

  PRINT("t0[0] = %d\n", t0.get<0>());
  PRINT("t0[1] = %f\n", t0.get<1>());
  PRINT("t0[2] = %f\n", t0.get<2>());
  PRINT("t0[3] = %d\n", t0.get<3>());
  
  lofi::meta::get<0>(t1).elem = 44.20f;
  lofi::meta::get<1>(t1).elem = 4.20f;
  lofi::meta::get<2>(t1).elem = 424;

  PRINT("t1[0] = %f\n", lofi::meta::get<0>(t1).elem);
  PRINT("t1[1] = %f\n", lofi::meta::get<1>(t1).elem);
  PRINT("t1[2] = %d\n", lofi::meta::get<2>(t1).elem);

  t1.get<0>() = 69.69;
  t1.get<1>() = 6.9;
  t1.get<2>() = 6969;

  PRINT("t1[0] = %f\n", t1.get<0>());
  PRINT("t1[1] = %f\n", t1.get<1>());
  PRINT("t1[2] = %d\n", t1.get<2>());

  using Var0 = lofi::variant<f32, i32, u8, u16>;

  struct custom_type0 {
    u8 data[64];
    custom_type0() {
      MEM_ZERO_ARRAY(data);
    }

    custom_type0(const custom_type0& other) {
      MEM_COPY_ARRAY(data, other.data);
    }

    custom_type0(custom_type0&& other) {}

    //~custom_type0() {
    //  PRINT_S("destructor called for custom_type");
    //}
  };


  struct custom_type1 {};

  using Var1 = lofi::variant<f32, custom_type0, i32>;
  using Var2 = lofi::variant<f32, custom_type1, i32>;


  Var0 var00(2.5f);
  Var0 var01((i32)420);
  Var0 var02((u16)420);
  Var0 var03((u8)69);

  Var1 var10((f32)420.420f);
  Var1 var11((i32)6969);
  Var1 var12(custom_type0{});

  Var2 var20((i32)6969);
  Var2 var21(custom_type1{});
  Var2 var22((f32)6969.6969f);

//--------------------------------------------------------------------------------------------
  PRINT_TITLE("VARIANTS");
//--------------------------------------------------------------------------------------------

  struct Visitor {
    using result_t = void;
    void operator()(f32&& f) {
      PRINT_S("dispatching visitor f32: ");
      EVAL_PRINT_F(f);
    }
    void operator()(i32&& i) {
      PRINT_S("dispatching visitor i32: ");
      EVAL_PRINT_I(i);
    }
    void operator()(u8&& i) {
      PRINT_S("dispatching visitor u8: ");
      EVAL_PRINT_I(i);
    }
    void operator()(u16&& i) {
      PRINT_S("dispatching visitor u16: ");
      EVAL_PRINT_I(i);
    }
    void operator()(custom_type0&& i) {
      PRINT_S("dispatching visitor ct0: ");
      PRINT("%s\n", "custom_type0{}");
    }
    void operator()(custom_type1&& i) {
      PRINT_S("dispatching visitor ct1: ");
      PRINT("%s\n", "custom_type1{}");
    }
  } vis;

  var00.visit(vis);
  var01.visit(vis);
  var02.visit(vis);
  var03.visit(vis);

  var10.visit(vis);
  var11.visit(vis);
  var12.visit(vis);

  var20.visit(vis);
  var21.visit(vis);
  var22.visit(vis);

//--------------------------------------------------------------------------------------------
  PRINT_TITLE("MEMORY POOL");
//--------------------------------------------------------------------------------------------

  struct DataStruct0 {
    u8 data[64];
    u64 data2[64];
  };

  struct DataStruct1 {
    u64 data[64];
    u64 data2[64];
    u64 data3[64];
  };

  size_t DataSize0 = sizeof(DataStruct0);
  size_t DataSize1 = sizeof(DataStruct1);

  EVAL_PRINT_ULL(DataSize0);
  EVAL_PRINT_ULL(DataSize1);

  PRINT_S("Allocating with static allocator\n");
  DataStruct0* static_pointers0[64];
  DataStruct0* runtime_pointers0[64];
  DataStruct1* static_pointers1[64];
  DataStruct1* runtime_pointers1[64];
  for(size_t i = 0; i < 64; i++) {
    static_pointers0[i] = lofi::StaticAllocator::allocate<0, DataStruct0>();
    new((void*)static_pointers0[i]) DataStruct0{};
    static_pointers1[i] = lofi::StaticAllocator::allocate<0, DataStruct1>();
    new((void*)static_pointers1[i]) DataStruct1{};
    EVAL_PRINT_ULL(static_pointers1[i]);
    EVAL_PRINT_ULL(i);
  }

  PRINT_S("Allocating with runtime allocator\n"); 
  for(size_t i = 0; i < 64; i++) {
    runtime_pointers0[i] = (DataStruct0*)lofi::RuntimeAllocator<0>::allocate(sizeof(DataStruct0), 8);
    new((void*)runtime_pointers0[i]) DataStruct0{};
    runtime_pointers1[i] = (DataStruct1*)lofi::RuntimeAllocator<0>::allocate(sizeof(DataStruct1), 8);
    new((void*)runtime_pointers1[i]) DataStruct1{};
    EVAL_PRINT_ULL(runtime_pointers1[i]);
    EVAL_PRINT_ULL(i);
  }

  PRINT_S("Deallocating with static allocator\n"); 
  for(size_t i = 0; i < 64; i++) {
    lofi::StaticAllocator::deallocate<0>(static_pointers0[i]);
    lofi::StaticAllocator::deallocate<0>(static_pointers1[i]);
    EVAL_PRINT_ULL(i);
  }
  PRINT_S("Deallocating with runtime allocator\n"); 
  for(size_t i = 0; i < 64; i++) {
    lofi::RuntimeAllocator<0>::free(runtime_pointers0[i]);
    lofi::RuntimeAllocator<0>::free(runtime_pointers1[i]);
    EVAL_PRINT_ULL(i);
  }

//--------------------------------------------------------------------------------------------
  PRINT_TITLE("LOGGING");
//--------------------------------------------------------------------------------------------

  if(lofi::get_log_system<4>().enable_file_output("log")) {
    lofi::Logger<0>::log<lofi::LogPriority::Fatal>("this is a log");
    lofi::Logger<1>::log<lofi::LogPriority::Fatal>("this is another log");
    lofi::Logger<2>::log<lofi::LogPriority::Fatal>("this is yet another log");
    PRINT_S("log system successfully initialized\n");
    lofi::get_log_system<4>().dump_logs();
  } else {
    PRINT_S("log system failed to initialize\n");
  }
//
////--------------------------------------------------------------------------------------------
//  PRINT_TITLE("SYNC");
////--------------------------------------------------------------------------------------------
//
//  lofi::lock_free_queue<size_t, 128> g_queue;
//  using node = lofi::lock_free_queue<size_t, 128>::node;
//  node nodes[128];
//  std::vector<std::thread> worker_pool;
//
//  for(size_t i = 0; i < NumThreads; i++) {
//    PRINT("constructing push thread %llu\n", i);
//    worker_pool.emplace_back(std::thread{[&](){
//      for(size_t j = i * 16; j < ((i * 16) + 16); j++) {
//        nodes[j].data = j;
//        PRINT("thread %llu pushing %llu\n", i, j);
//        g_queue.push(&nodes[j]);  
//      }
//    }});
//  }
//
//  for(size_t i = 0; i < NumThreads; i++) {
//    worker_pool[i].join();
//  }
//
//  for(size_t i = 0; i < NumThreads; i++) {
//    PRINT("constructing pop thread %llu\n", i);
//    worker_pool.emplace_back(std::thread{[&](){
//        size_t index = MAX_u64;
//      while(g_queue.pop(&index)) {
//        EVAL_PRINT_ULL(index);
//      }
//    }});
//  }
//
//  for(size_t i = 0; i < NumThreads; i++) {
//    worker_pool[i + NumThreads].join();
//  }

//--------------------------------------------------------------------------------------------
  PRINT_TITLE("THREAD POOL");
//--------------------------------------------------------------------------------------------

  using thread_pool_t = lofi::ThreadPool<NumThreads, NumFibers>;
  thread_pool_t thread_pool;

  thread_pool_t::job_node_t node;
  lofi::atomic_counter<> terminate_gate{0};

  lofi::Job job;
  job.set_table(
      { init_job
      , standard_job_success
      , standard_job_failure
      , 0
      , 1
      , &terminate_gate }
      );
  node.data = job;

  thread_pool.push_high_priority_job(MAX_u64, &node);
  thread_pool.run();
  using namespace std::chrono_literals;
  while(terminate_gate.get_count() < 1) { /* PRINT_S("sleeping 5ms, current count\n");*/ std::this_thread::sleep_for(5ms); }

  //PRINT_TITLE("TERMINATING");
  thread_pool.terminate();

  for(size_t i = 0; i < 256; i++) {
    PRINT("values[%llu] = %llu\n", i, values[i]);
  }

  PRINT_TITLE("EXITING TESTS");
  return 0;
}
