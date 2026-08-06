cmake_minimum_required(VERSION 3.14)
project(CkPttnInstalledTest LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find dependencies first - the installed config references fmt, Py2Cpp, XNetwork, NetlistX and
# MyWheel.
find_package(Threads REQUIRED)
find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)
find_package(Py2Cpp REQUIRED)
find_package(XNetwork REQUIRED)
find_package(NetlistX REQUIRED)
find_package(MyWheel REQUIRED)
find_package(CkPttn REQUIRED)

add_executable(ckpttn_installed_test main.cpp)
target_link_libraries(ckpttn_installed_test PRIVATE CkPttn::CkPttn)

enable_testing()
add_test(NAME ckpttn_installed_test COMMAND ckpttn_installed_test)
