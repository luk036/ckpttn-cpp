set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# find_package(Boost REQUIRED COMPONENTS container) if(Boost_FOUND) message(STATUS "Found boost:
# ${Boost_LIBRARIES}") # add_library(Boost::boost INTERFACE IMPORTED GLOBAL)
# target_include_directories(Boost::boost # SYSTEM INTERFACE ${Boost_INCLUDE_DIRS}) endif()

# Add spdlog for logging functionality - use bundled fmt to avoid compatibility issues
CPMAddPackage(
  NAME spdlog
  GIT_TAG v1.15.0
  GITHUB_REPOSITORY gabime/spdlog
  OPTIONS "SPDLOG_INSTALL YES"
)

# cpmaddpackage("gh:ericniebler/range-v3#0.10.0") CPMAddPackage("gh:microsoft/GSL@4.0.0")

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

CPMAddPackage(
  NAME NetlistX
  GIT_TAG 1.0
  GITHUB_REPOSITORY luk036/netlistx-cpp
  OPTIONS "INSTALL_ONLY ON" # create an installable target
)

CPMAddPackage(
  NAME MyWheel
  GIT_TAG 1.0.3
  GITHUB_REPOSITORY luk036/mywheel-cpp
  OPTIONS "INSTALL_ONLY ON" # create an installable target
)

set(SPECIFIC_LIBS MyWheel::MyWheel NetlistX::NetlistX XNetwork::XNetwork Py2Cpp::Py2Cpp
                  Threads::Threads
)

# Only add fmt::fmt to SPECIFIC_LIBS if we're using external fmt
if(TARGET fmt::fmt)
  list(APPEND SPECIFIC_LIBS fmt::fmt)
endif()

# Add spdlog::spdlog to SPECIFIC_LIBS for logging functionality
if(TARGET spdlog::spdlog)
  list(APPEND SPECIFIC_LIBS spdlog::spdlog)
endif()
