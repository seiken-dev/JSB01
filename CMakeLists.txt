cmake_minimum_required(VERSION 3.12)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
project(stargazer C CXX ASM)
pico_sdk_init()

add_executable(stargazer src/main.cpp)
target_link_libraries(stargazer pico_stdlib)
pico_enable_stdio_usb(stargazer 1)
pico_enable_stdio_uart(stargazer 0)

pico_add_extra_outputs(stargazer)
