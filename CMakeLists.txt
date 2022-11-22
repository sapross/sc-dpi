cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(SC-DPI)

find_package(verilator HINTS $ENV(VERILATOR_ROOT))

## define targets

add_library(serial_console STATIC
  src/serial_console.hpp
  src/serial_console.cpp
  src/SimUART.cpp
  )
target_include_directories(serial_console PUBLIC src)
verilate(serial_console SOURCES src/SimUART.sv)
add_executable(UART_DPI
  test/main.cpp
  )
target_link_libraries(UART_DPI PUBLIC serial_console PRIVATE)
verilate(UART_DPI
  SOURCES test/main.sv
  INCLUDE_DIRS src
  TOP_MODULE main
  TRACE_FST)

add_executable(UART_TEST
  test/echo_test.cpp
  )
target_link_libraries(UART_TEST PUBLIC serial_console PRIVATE)
