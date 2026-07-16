cmake_minimum_required(VERSION 3.16)

project(logger_project VERSION 1.0.0 LANGUAGES CXX)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR
          "In-source builds are not supported. Use: cmake -S . -B build")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Threads REQUIRED)

function(logger_enable_project_warnings target)
  target_compile_options(
    ${target}
    PRIVATE -Wall
            -Wextra
            -Wpedantic
            -Wconversion
            -Wshadow
            -Wswitch-enum
            -Werror)
endfunction()

add_library(
  logger_objects OBJECT
  src/file_sink.cpp
  src/log_level.cpp
  src/logger.cpp
  src/record_formatter.cpp
  src/serialization.cpp
  src/statistics.cpp
  src/udp_sink.cpp)
target_include_directories(logger_objects PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(logger_objects PUBLIC cxx_std_17)
set_target_properties(logger_objects PROPERTIES POSITION_INDEPENDENT_CODE ON)
logger_enable_project_warnings(logger_objects)

add_library(logger_static STATIC $<TARGET_OBJECTS:logger_objects>)
add_library(logger_shared SHARED $<TARGET_OBJECTS:logger_objects>)

set_target_properties(logger_static PROPERTIES OUTPUT_NAME logger)
set_target_properties(logger_shared PROPERTIES OUTPUT_NAME logger)

target_include_directories(logger_static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(logger_shared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(logger_shared PRIVATE Threads::Threads)

add_library(logger_demo_support STATIC src/demo_support.cpp)
target_include_directories(
  logger_demo_support
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(logger_demo_support PUBLIC logger_static Threads::Threads)
logger_enable_project_warnings(logger_demo_support)

add_executable(logger_demo src/logger_demo.cpp)
target_link_libraries(logger_demo PRIVATE logger_demo_support)
logger_enable_project_warnings(logger_demo)

add_executable(udp_logger_demo src/udp_logger_demo.cpp)
target_link_libraries(udp_logger_demo PRIVATE logger_demo_support)
logger_enable_project_warnings(udp_logger_demo)

add_executable(statistics_receiver src/statistics_receiver.cpp)
target_link_libraries(statistics_receiver PRIVATE logger_static Threads::Threads)
logger_enable_project_warnings(statistics_receiver)

enable_testing()

add_executable(
  logger_tests
  tests/file_sink_tests.cpp
  tests/expected_tests.cpp
  tests/logger_demo_tests.cpp
  tests/logger_tests.cpp
  tests/queue_tests.cpp
  tests/serialization_tests.cpp
  tests/statistics_tests.cpp
  tests/test_main.cpp
  tests/udp_sink_tests.cpp)
target_include_directories(
  logger_tests
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
          ${CMAKE_CURRENT_SOURCE_DIR}/src
          ${CMAKE_CURRENT_SOURCE_DIR}/tests)
target_link_libraries(logger_tests PRIVATE logger_demo_support Threads::Threads)
logger_enable_project_warnings(logger_tests)

add_test(NAME logger_tests COMMAND logger_tests)
