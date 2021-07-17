set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

find_package(XNetwork QUIET)
if(XNetwork_FOUND)
  message(STATUS "Found XNetwork: ${XNetwork_INCLUDE_DIR}")
else()
  # CPMAddPackage("gh:luk036/xn-cpp#1.0.5")
  CPMAddPackage(
    NAME XNetwork
    GIT_TAG 1.0.6
    GITHUB_REPOSITORY luk036/xn-cpp
  )
endif(XNetwork_FOUND)

CPMAddPackage("gh:microsoft/GSL@3.1.0")

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
