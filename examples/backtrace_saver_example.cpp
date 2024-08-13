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
  std::cout << "backtrace: " << std::endl;
  #if CONFIG_RESTART_DEBUG_STACK_DEPTH > 0
    for (int i = 0; i < CONFIG_RESTART_DEBUG_STACK_DEPTH; i++) {
      std::cout << "0x" << std::hex << current_debug_info.backtrace[i].first << ":0x" << std::hex << current_debug_info.backtrace[i].second << " ";
    }
    std::cout << std::endl;
  #endif

  // provoque the error
  int res = first();
}