cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(CkPttnBench LANGUAGES CXX)

# --- Import tools ----

include(../cmake/tools.cmake)

# ---- Dependencies ----

include(../cmake/CPM.cmake)

CPMAddPackage("gh:onqtam/doctest#2.4.5")
CPMAddPackage("gh:TheLartians/Format.cmake@1.7.0")

include(../specific.cmake)

if(TEST_INSTALLED_VERSION)
  find_package(CkPttn REQUIRED)
else()
  CPMAddPackage(NAME CkPttn SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/..)
endif()

# ---- Create binary ----

CPMAddPackage(
  NAME benchmark
  GITHUB_REPOSITORY google/benchmark
  VERSION 1.5.2
  OPTIONS "BENCHMARK_ENABLE_TESTING Off"
)

if(benchmark_ADDED)
  # enable c++11 to avoid compilation errors
  set_target_properties(benchmark PROPERTIES CXX_STANDARD 11)
endif()

# ~~~

file(GLOB_RECURSE ALL_BENCH_CPP *.cpp)

enable_testing()

foreach(ONE_BENCH_CPP ${ALL_BENCH_CPP})

  get_filename_component(ONE_BENCH_EXEC ${ONE_BENCH_CPP} NAME_WE)

  # Avoid name collision
  set(TARGET_NAME Bench_${ONE_BENCH_EXEC})

  add_executable(${TARGET_NAME} ${ONE_BENCH_CPP})
  set_target_properties(${TARGET_NAME} PROPERTIES OUTPUT_NAME ${ONE_BENCH_EXEC})
  set_target_properties(${TARGET_NAME} PROPERTIES CXX_STANDARD 11)

  target_link_libraries(
    ${TARGET_NAME}
    doctest::doctest
    benchmark::benchmark
    CkPttn::CkPttn
    XNetwork::XNetwork
    cppcoro::cppcoro
    Threads::Threads
    GSL
    range-v3
  )
  set_target_properties(${TARGET_NAME} PROPERTIES CXX_STANDARD 20)

  # add_test(${TARGET_NAME} ${ONE_BENCH_EXEC})
  add_test(NAME ${ONE_BENCH_EXEC} COMMAND ${TARGET_NAME})
endforeach()

# enable compiler warnings
if(NOT TEST_INSTALLED_VERSION)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(CkPttn PUBLIC -Wall -Wpedantic -Wextra -Werror)
  elseif(MSVC)
    target_compile_options(CkPttn PUBLIC /W4 /WX)
    # target_compile_definitions(CkPttnTests PUBLIC DOCTEST_CONFIG_USE_STD_HEADERS)
  endif()
endif()
