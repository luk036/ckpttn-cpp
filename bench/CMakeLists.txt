# ---- Dependencies ----

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

# ---- Create binary ----

file(GLOB_RECURSE ALL_BENCH_CPP *.cpp)

enable_testing()

foreach(ONE_BENCH_CPP ${ALL_BENCH_CPP})

  get_filename_component(ONE_BENCH_EXEC ${ONE_BENCH_CPP} NAME_WE)

  # Avoid name collision
  set(TARGET_NAME Bench_${ONE_BENCH_EXEC})

  add_executable(${TARGET_NAME} ${ONE_BENCH_CPP})
  set_target_properties(${TARGET_NAME} PROPERTIES OUTPUT_NAME ${ONE_BENCH_EXEC})
  set_target_properties(${TARGET_NAME} PROPERTIES CXX_STANDARD 20)

  target_link_libraries(
    ${TARGET_NAME} doctest::doctest benchmark::benchmark ${PROJECT_NAME}::${PROJECT_NAME} ${SPECIFIC_LIBS}
  )

  # add_test(${TARGET_NAME} ${ONE_BENCH_EXEC})
  add_test(NAME ${ONE_BENCH_EXEC} COMMAND ${TARGET_NAME})
endforeach()

# enable compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  target_compile_options(CkPttn PUBLIC -Wall -Wpedantic -Wextra -Werror)
elseif(MSVC)
  target_compile_options(CkPttn PUBLIC /W4 /WX)
  # target_compile_definitions(CkPttnTests PUBLIC DOCTEST_CONFIG_USE_STD_HEADERS)
endif()
