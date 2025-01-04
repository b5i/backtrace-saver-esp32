// From https://kotyara12.ru/iot/remote_esp32_backtrace

#include "backtrace_saver.hpp"

#include "esp_err.h"
#include "esp_debug_helpers.h"

#if __has_include("soc/cpu.h")
    #include "soc/cpu.h"
#else
    #include "esp_cpu.h" 
    #include "esp_cpu_utils.h"
#endif

#include "soc/soc_memory_layout.h"
#include <string.h>
#include <esp_heap_caps.h>

namespace backtrace_saver {
    
    void debugHeapUpdate()
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

    #ifdef BT_SAVER_NEW_API
    void debugBacktraceUpdateWithInfo(arduino_panic_info_t *info)
    {
        for (unsigned int i = 0; i < CONFIG_RESTART_DEBUG_STACK_DEPTH; i++) {
            if (i > info->backtrace_len) {
                _debug_info.backtracePC[i] = 0;
                _debug_info.backtraceSP[i] = 0;
            } else {
                _debug_info.backtraceSP[i] = esp_cpu_process_stack_pc(info->backtrace[i]);
                _debug_info.backtracePC[i] = info->backtrace[i];
            }
        }
    }

    void debugUpdateWithInfo(arduino_panic_info_t *info, void*)
    {
        debugHeapUpdate();
        #if CONFIG_RESTART_DEBUG_STACK_DEPTH > 0
        debugBacktraceUpdateWithInfo(info);
        #endif // CONFIG_RESTART_DEBUG_STACK_DEPTH
    }

    #endif

    void debugBacktraceUpdate()
    {
        esp_backtrace_frame_t stk_frame;
        esp_backtrace_get_start(&(stk_frame.pc), &(stk_frame.sp), &(stk_frame.next_pc)); 
        _debug_info.backtracePC[0] = esp_cpu_process_stack_pc(stk_frame.pc);
        _debug_info.backtraceSP[0] = stk_frame.sp;
        bool corrupted = (esp_stack_ptr_is_sane(stk_frame.sp) &&
                            esp_ptr_executable((void*)esp_cpu_process_stack_pc(stk_frame.pc))) ?
                            false : true; 
        #if CONFIG_RESTART_DEBUG_STACK_DEPTH > 1
        uint8_t i = CONFIG_RESTART_DEBUG_STACK_DEPTH;
        while (i-- > 0 && stk_frame.next_pc != 0 && !corrupted) {
            if (!esp_backtrace_get_next_frame(&stk_frame)) {
                corrupted = true;
            };
            _debug_info.backtracePC[CONFIG_RESTART_DEBUG_STACK_DEPTH - i] = esp_cpu_process_stack_pc(stk_frame.pc);
            _debug_info.backtraceSP[CONFIG_RESTART_DEBUG_STACK_DEPTH - i] = stk_frame.sp;
        };
        #endif // CONFIG_RESTART_DEBUG_STACK_DEPTH > 1
    }

    #endif // CONFIG_RESTART_DEBUG_STACK_DEPTH

    void debugUpdate()
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

    #ifndef BT_SAVER_NEW_API

    extern "C" void __real_esp_panic_handler(void* info);

    extern "C" void __wrap_esp_panic_handler(void* info) 
    {
        debugUpdate();
            
        // Call the original panic handler function to finish processing this error (creating a core dump for example...)
        __real_esp_panic_handler(info);
    }

    #endif

    void init()
    {
        esp_register_shutdown_handler(debugUpdate);

        #ifdef BT_SAVER_NEW_API
        set_arduino_panic_handler(debugUpdateWithInfo, nullptr);
        #endif
    }

} // namespace backtrace_saver
