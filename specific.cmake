set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# find_package(Boost REQUIRED COMPONENTS container) if(Boost_FOUND) message(STATUS "Found boost:
# ${Boost_LIBRARIES}") # add_library(Boost::boost INTERFACE IMPORTED GLOBAL)
# target_include_directories(Boost::boost # SYSTEM INTERFACE ${Boost_INCLUDE_DIRS}) endif()

# Add fmt explicitly as a dependency. This must come BEFORE XNetwork and NetlistX
# so that fmt::fmt target is always available when their CMakeLists.txt process.
# Both XNetwork and NetlistX also add fmt internally, but CPM deduplication
# ensures it's only added once.
CPMAddPackage(
  NAME fmt
  GIT_TAG 12.1.0
  GITHUB_REPOSITORY fmtlib/fmt
  OPTIONS "FMT_INSTALL YES" # create an installable target
)

# Add spdlog for logging functionality - use bundled fmt to avoid compatibility issues
CPMAddPackage(
  NAME spdlog
  GIT_TAG v1.17.0
  GITHUB_REPOSITORY gabime/spdlog
  OPTIONS "SPDLOG_INSTALL YES" # create an installable target
)

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
  GIT_TAG 1.6.0
  GITHUB_REPOSITORY luk036/py2cpp
  OPTIONS "INSTALL_ONLY ON" # create an installable target
)

CPMAddPackage(
  NAME XNetwork
  GIT_TAG 1.7.3
  GITHUB_REPOSITORY luk036/xnetwork-cpp
  OPTIONS "INSTALL_ONLY ON" # create an installable target
)

CPMAddPackage(
  NAME NetlistX
  GIT_TAG 1.1.4
  GITHUB_REPOSITORY luk036/netlistx-cpp
  OPTIONS "INSTALL_ONLY ON" # create an installable target
)

CPMAddPackage(
  NAME MyWheel
  GIT_TAG 1.1.3
  GITHUB_REPOSITORY luk036/mywheel-cpp
  OPTIONS "INSTALL_ONLY ON" # create an installable target
)

set(SPECIFIC_LIBS MyWheel::MyWheel NetlistX::NetlistX XNetwork::XNetwork Py2Cpp::Py2Cpp
                  Threads::Threads fmt::fmt
)

# Add spdlog::spdlog to SPECIFIC_LIBS for logging functionality
if(TARGET spdlog::spdlog)
  list(APPEND SPECIFIC_LIBS spdlog::spdlog)
endif()
