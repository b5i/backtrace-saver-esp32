// From https://kotyara12.ru/iot/remote_esp32_backtrace

#include "backtrace_saver.hpp"

#include "esp_err.h"
#include "esp_debug_helpers.h"
#include "soc/cpu.h"
#include "soc/soc_memory_layout.h"
#include <string.h>
#include <esp_heap_caps.h>
#include <time.h>

namespace backtrace_saver {
    void IRAM_ATTR debugHeapUpdate()
    {
        _debug_info.heap_total = heap_caps_get_total_size(MALLOC_CAP_DEFAULT);
        _debug_info.heap_free = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
        size_t _new_free_min = heap_caps_get_minimum_free_size(MALLOC_CAP_DEFAULT);
        if ((_debug_info.heap_free_min == 0) || (_new_free_min < _debug_info.heap_free_min)) {
            _debug_info.heap_free_min = _new_free_min;
            _debug_info.heap_min_time = time(nullptr);
        };
    }
    #if CONFIG_RESTART_DEBUG_STACK_DEPTH > 0
    void IRAM_ATTR debugBacktraceUpdate()
    {
        esp_backtrace_frame_t stk_frame;
        esp_backtrace_get_start(&(stk_frame.pc), &(stk_frame.sp), &(stk_frame.next_pc)); 
        _debug_info.backtrace[0] = std::make_pair(esp_cpu_process_stack_pc(stk_frame.pc), stk_frame.sp);
        bool corrupted = (esp_stack_ptr_is_sane(stk_frame.sp) &&
                            esp_ptr_executable((void*)esp_cpu_process_stack_pc(stk_frame.pc))) ?
                            false : true; 
        #if CONFIG_RESTART_DEBUG_STACK_DEPTH > 1
        uint8_t i = CONFIG_RESTART_DEBUG_STACK_DEPTH;
        while (i-- > 0 && stk_frame.next_pc != 0 && !corrupted) {
            if (!esp_backtrace_get_next_frame(&stk_frame)) {
                corrupted = true;
            };
            _debug_info.backtrace[CONFIG_RESTART_DEBUG_STACK_DEPTH - i] = std::make_pair(esp_cpu_process_stack_pc(stk_frame.pc), stk_frame.sp);
        };
        #endif // CONFIG_RESTART_DEBUG_STACK_DEPTH > 1
    }
    #endif // CONFIG_RESTART_DEBUG_STACK_DEPTH
    void IRAM_ATTR debugUpdate()
    {
        debugHeapUpdate();
        #if CONFIG_RESTART_DEBUG_STACK_DEPTH > 0
        debugBacktraceUpdate();
        #endif // CONFIG_RESTART_DEBUG_STACK_DEPTH
    }

    re_restart_debug_t debugGet()
    {
        re_restart_debug_t ret;
        memset(&ret, 0, sizeof(re_restart_debug_t));
        esp_reset_reason_t esp_reason = esp_reset_reason();
        if ((esp_reason != ESP_RST_UNKNOWN) && (esp_reason != ESP_RST_POWERON)) {
            ret = _debug_info;
            if (_debug_info.heap_total > heap_caps_get_total_size(MALLOC_CAP_DEFAULT)) {
            memset(&ret, 0, sizeof(re_restart_debug_t));
            };
        };
        memset(&_debug_info, 0, sizeof(re_restart_debug_t));
        return ret;
    }

    extern "C" void __real_esp_panic_handler(void* info);

    extern "C" void __wrap_esp_panic_handler(void* info) 
    {
        #if CONFIG_RESTART_DEBUG_STACK_DEPTH > 0
            debugBacktraceUpdate();
        #endif // CONFIG_RESTART_DEBUG_STACK_DEPTH
            
        // Call the original panic handler function to finish processing this error (creating a core dump for example...)
        __real_esp_panic_handler(info);
    }

    void init()
    {
        esp_register_shutdown_handler(debugUpdate);
    }
} // namespace backtrace_saver