# backtrace-saver-esp32

A library to easily retrieve last crash's backtrace on ESP32.

## Credits

The code is mainly from KOTYARA12's article: https://kotyara12.ru/iot/remote_esp32_backtrace (thank you K12 for giving me the permission to post this module!).

The module has been made for the [Paxo](https://paxo.fr) project which uses an ESP32.

## Installation

1. Open platformio.ini, a project configuration file located in the root of PlatformIO project.
2. Add the following line to the `lib_deps` option of `[env:YourESP32BuildTarget]`:
`anto/backtrace-saver-esp32`.
3. Add `-Wl,--wrap=esp_panic_handler` in the `build_flags` of the project.
4. Build the project, PlatformIO will automatically install dependencies. 

## Activation

To activate the capture of the backtrace, you need to set up the shutdown handler of the ESP32 by calling `backtrace_saver::init()` at the beginning of your program (in `app_main` or `setup` for example).

## Options

You can opt to change the size of the saved backtrace by overriding the constant `CONFIG_RESTART_DEBUG_STACK_DEPTH` by adding `-DCONFIG_RESTART_DEBUG_STACK_DEPTH=x` to the `build_flags` of the project.

You can also manually capture backtrace and heap information by calling `backtbacktrace_saver::debugHeapUpdate()`, `backtbacktrace_saver::debugBacktraceUpdate()` or `backtbacktrace_saver::debugUpdate()` for both of them. To get the saved backtrace and heap information, call `backtbacktrace_saver::debugGet()`.

The i-th PC in `backtbacktrace_saver::re_restart_debug_t.backtracePC` corresponds to the i-th SP of `backtbacktrace_saver::re_restart_debug_t.backtracePC`.

## Backtrace usage

To use the backtrace information (e.g. `400d1ba7:0x3ffda0a0 400d575f:0x3ffda0a0 40082e1d:0x3ffda0a0 400846c8:0x3ffda0a0 400829c5:0x3ffda0a0 400891ad:0x3ffda0a0 4008dda5:0x3ffda0a0 400dffcb:0x3ffda0a0 400e0012:0x3ffda0a0 400de263:0x3ffda0a0 400d1beb:0x3ffda0a0 400d1c05:0x3ffda0a0 400d1c4c:0x3ffda0a0 400d1c7d:0x3ffda0a0`), use the `addr2line -pfiaC -e .pio/build/esp32/firmware.elf 400d1ba7 400d575f 40082e1d 400846c8 400829c5 400891ad 4008dda5 400dffcb 400e0012 400de263 400d1beb 400d1c05 400d1c4c 400d1c7d` command as KOTYARA12 explained [here](https://kotyara12.ru/pubs/iot/platformio-addr2name) (no need to include the sp).
