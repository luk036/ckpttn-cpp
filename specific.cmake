set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# find_package(Boost REQUIRED COMPONENTS container) if(Boost_FOUND) message(STATUS "Found boost:
# ${Boost_LIBRARIES}") # add_library(Boost::boost INTERFACE IMPORTED GLOBAL)
# target_include_directories(Boost::boost # SYSTEM INTERFACE ${Boost_INCLUDE_DIRS}) endif()

# Try system-installed fmt first (Ubuntu: libfmt-dev, macOS: brew install fmt, Termux: fmt)
find_package(fmt CONFIG QUIET)

if(fmt_FOUND)
  message(STATUS "Found system fmt: ${fmt_DIR}")
else()
  # Add fmt explicitly as a dependency. This must come BEFORE XNetwork and NetlistX so that fmt::fmt
  # target is always available when their CMakeLists.txt process. Both XNetwork and NetlistX also add
  # fmt internally, but CPM deduplication ensures it's only added once.
  CPMAddPackage(
    NAME fmt
    GIT_TAG 12.1.0
    GITHUB_REPOSITORY fmtlib/fmt
    OPTIONS "FMT_INSTALL YES" # create an installable target
  )
endif()

# Try system-installed spdlog first (Ubuntu: libspdlog-dev, macOS: brew install spdlog, Termux: spdlog)
find_package(spdlog CONFIG QUIET)

if(spdlog_FOUND)
  message(STATUS "Found system spdlog: ${spdlog_DIR}")
else()
  # Add spdlog for logging functionality - use external fmt (added above) to avoid duplicate symbol
  # errors when both spdlog's bundled fmt and our separate fmt.lib are linked.
  CPMAddPackage(
    NAME spdlog
    GIT_TAG v1.17.0
    GITHUB_REPOSITORY gabime/spdlog
    OPTIONS "SPDLOG_INSTALL YES" "SPDLOG_FMT_EXTERNAL ON" # create an installable target, use external
                                                          # fmt
  )
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # using GCC add_compile_options(-fcoroutines)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # using Visual Studio C++
  add_compile_options(/std:c++latest)
  add_compile_definitions(_SILENCE_EXPERIMENTAL_COROUTINE_DEPRECATION_WARNINGS)
else() # (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  # using clang
  add_compile_options(-stdlib=libc++)
endif()

CPMAddPackage(
  NAME Py2Cpp
  GIT_TAG v1.6.3
  GITHUB_REPOSITORY luk036/py2cpp
  OPTIONS "INSTALL_ONLY ON" # create an installable target
)

CPMAddPackage(
  NAME XNetwork
  GIT_TAG v1.7.6
  GITHUB_REPOSITORY luk036/xnetwork-cpp
  OPTIONS "INSTALL_ONLY ON" # create an installable target
)

CPMAddPackage(
  NAME NetlistX
  GIT_TAG v1.1.6
  GITHUB_REPOSITORY luk036/netlistx-cpp
  OPTIONS "INSTALL_ONLY ON" # create an installable target
)

CPMAddPackage(
  NAME MyWheel
  GIT_TAG v1.1.5
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
