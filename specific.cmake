set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# find_package(Boost REQUIRED COMPONENTS container) if(Boost_FOUND) message(STATUS "Found boost:
# ${Boost_LIBRARIES}") # add_library(Boost::boost INTERFACE IMPORTED GLOBAL)
# target_include_directories(Boost::boost # SYSTEM INTERFACE ${Boost_INCLUDE_DIRS}) endif()

CPMAddPackage(
  NAME fmt
  GIT_TAG 10.2.1
  GITHUB_REPOSITORY fmtlib/fmt
  OPTIONS "FMT_INSTALL YES" # create an installable target
)

# cpmaddpackage("gh:ericniebler/range-v3#0.10.0")
# CPMAddPackage("gh:microsoft/GSL@4.0.0")

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # using GCC add_compile_options(-fcoroutines)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # using Visual Studio C++
  add_compile_options(/std:c++latest /await)
else() # (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  # using clang
  add_compile_options(-stdlib=libc++)
endif()

CPMAddPackage(
  NAME Py2Cpp
  GIT_TAG 1.4.9
  GITHUB_REPOSITORY luk036/py2cpp
  OPTIONS "INSTALL_ONLY ON" # create an installable target
)

CPMAddPackage(
  NAME XNetwork
  GIT_TAG 1.6.10
  GITHUB_REPOSITORY luk036/xnetwork-cpp
  OPTIONS "INSTALL_ONLY ON" # create an installable target
)

set(SPECIFIC_LIBS
    XNetwork::XNetwork Py2Cpp::Py2Cpp
    Threads::Threads fmt::fmt
)
