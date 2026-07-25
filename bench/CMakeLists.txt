# ---- Dependencies ----

# nanobench is a header-only library for microbenchmarking
CPMAddPackage(
  NAME nanobench
  GITHUB_REPOSITORY martinus/nanobench
  GIT_TAG v4.3.11
)

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
    ${TARGET_NAME} nanobench ${PROJECT_NAME}::${PROJECT_NAME} ${SPECIFIC_LIBS}
  )

  add_test(NAME ${ONE_BENCH_EXEC} COMMAND ${TARGET_NAME})
endforeach()

# enable compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  target_compile_options(CkPttn PUBLIC -Wall -Wpedantic -Wextra -Werror)
elseif(MSVC)
  target_compile_options(CkPttn PUBLIC /W4 /WX /wd4996)
  # target_compile_definitions(CkPttnTests PUBLIC DOCTEST_CONFIG_USE_STD_HEADERS)
endif()
