set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

CPMAddPackage(
  NAME fmt
  GIT_TAG 7.1.3
  GITHUB_REPOSITORY fmtlib/fmt
  OPTIONS "FMT_INSTALL YES" # create an installable target
)

CPMAddPackage("gh:microsoft/GSL@3.1.0")
CPMAddPackage("gh:ericniebler/range-v3#0.10.0")

CPMAddPackage(
  NAME XNetwork
  GIT_TAG 1.0.9
  GITHUB_REPOSITORY luk036/xnetwork-cpp
  OPTIONS "INSTALL_ONLY ON" # create an installable target
)

CPMAddPackage(
  NAME cppcoro
  GIT_TAG 1.0
  GITHUB_REPOSITORY luk036/cppcoro
)
# print_target_properties(cppcoro)
if(cppcoro_ADDED)
  message(STATUS "Found cppcoro: ${cppcoro_SOURCE_DIR}")
  add_library(cppcoro::cppcoro INTERFACE IMPORTED GLOBAL)
  target_include_directories(cppcoro::cppcoro SYSTEM INTERFACE ${cppcoro_SOURCE_DIR}/include)
endif(cppcoro_ADDED)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # using GCC
  add_compile_options(-fcoroutines)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  # using clang
  add_compile_options(-fcoroutines-ts -stdlib=libc++)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # using Visual Studio C++
  add_compile_options(/std:c++latest /await)
endif()

set(SPECIFIC_LIBS XNetwork::XNetwork cppcoro::cppcoro Threads::Threads GSL fmt::fmt range-v3)
