#pragma once

// From https://kotyara12.ru/iot/remote_esp32_backtrace

// Depth to save the processor stack on restart. 0 - do not save
#ifndef CONFIG_RESTART_DEBUG_STACK_DEPTH
    #define CONFIG_RESTART_DEBUG_STACK_DEPTH 28
#endif

#include "esp_types.h"
#include "esp_attr.h"
#include <esp_system.h>
#include <utility>

namespace backtrace_saver {
    void IRAM_ATTR debugHeapUpdate();
    void IRAM_ATTR debugBacktraceUpdate();
    void IRAM_ATTR debugUpdate();
    typedef struct {
        size_t heap_total;
        size_t heap_free;
        size_t heap_free_min;
        time_t heap_min_time;
        #if CONFIG_RESTART_DEBUG_STACK_DEPTH > 0
        std::pair<uint32_t, uint32_t> backtrace[CONFIG_RESTART_DEBUG_STACK_DEPTH]; // PC, SP
        #endif // CONFIG_RESTART_DEBUG_STACK_DEPTH
    } re_restart_debug_t;
    re_restart_debug_t debugGet();
    __NOINIT_ATTR static re_restart_debug_t _debug_info;

    void init();
}