#include <iostream>
#include <backtrace_saver.hpp>

void setup() {
  backtrace_saver::init();
}

int third(int b) {
  throw std::runtime_error("test");
  return b + 1;
}
int second(int a) {
  return third(a);
}
int first() {
  return second(1);
}

void loop() {
  sleep(1);  
  backtrace_saver::re_restart_debug_t current_debug_info = backtrace_saver::debugGet();
  std::cout << "Hello from backtrace_saver" << std::endl;
  std::cout << "backtrace: " << std::endl;
  #if CONFIG_RESTART_DEBUG_STACK_DEPTH > 0
    for (int i = 0; i < CONFIG_RESTART_DEBUG_STACK_DEPTH; i++) {
      std::cout << "0x" << std::hex << current_debug_info.backtracePC[i] << ":0x" << std::hex << current_debug_info.backtraceSP[i] << " ";
    }
    std::cout << std::endl;
  #endif

  std::cout << "heap_total: " << std::dec << current_debug_info.heap_total << std::endl;
  std::cout << "heap_free: " << std::dec << current_debug_info.heap_free << std::endl;
  std::cout << "heap_free_min: " << std::dec << current_debug_info.heap_free_min << std::endl;
  std::cout << "heap_min_time: " << std::dec << current_debug_info.heap_min_time << std::endl;
  
  // provoque the error
  int res = first();
}