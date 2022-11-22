cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(SC-DPI)

find_package(verilator HINTS $ENV(VERILATOR_ROOT))

## define targets

add_library(serial_console STATIC
  lib/serial_console.hpp
  lib/serial_console.cpp
  lib/SimUART.cpp
  )
target_include_directories(serial_console PUBLIC lib)
verilate(serial_console SOURCES lib/SimUART.sv)
add_executable(UART_DPI
  src/main.cpp
  )
target_link_libraries(UART_DPI PUBLIC serial_console PRIVATE)
verilate(UART_DPI
  SOURCES src/main.sv
  INCLUDE_DIRS lib
  TOP_MODULE main
  TRACE_FST)

add_executable(UART_TEST
  src/echo_test.cpp
  )
target_link_libraries(UART_TEST PUBLIC serial_console PRIVATE)
