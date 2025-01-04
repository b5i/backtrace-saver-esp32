#pragma once

// From https://kotyara12.ru/iot/remote_esp32_backtrace

// Depth to save the processor stack on restart. 0 - do not save
#ifndef CONFIG_RESTART_DEBUG_STACK_DEPTH
    #define CONFIG_RESTART_DEBUG_STACK_DEPTH 28
#endif

#include "esp_types.h"
#include "esp_attr.h"
#include <esp_system.h>
#include <ctime>

#if ESP_IDF_VERSION_MAJOR > 4
    // panic handler must be set using set_arduino_panic_handler

    #include "esp32-hal.h"
    #define BT_SAVER_NEW_API
#endif

namespace backtrace_saver {
    void IRAM_ATTR debugHeapUpdate();

    void IRAM_ATTR debugBacktraceUpdate();

    #ifdef BT_SAVER_NEW_API
    void IRAM_ATTR debugUpdateWithInfo(arduino_panic_info_t *info, void*);
    void IRAM_ATTR debugBacktraceUpdateWithInfo(arduino_panic_info_t *info);
    #endif

    typedef struct {
        size_t heap_total;
        size_t heap_free;
        size_t heap_free_min;
        time_t heap_min_time;
        #if CONFIG_RESTART_DEBUG_STACK_DEPTH > 0
        uint32_t backtraceSP[CONFIG_RESTART_DEBUG_STACK_DEPTH];
        uint32_t backtracePC[CONFIG_RESTART_DEBUG_STACK_DEPTH];
        #endif // CONFIG_RESTART_DEBUG_STACK_DEPTH
    } re_restart_debug_t;
    __NOINIT_ATTR static re_restart_debug_t _debug_info;

    re_restart_debug_t debugGet();

    void init();
}