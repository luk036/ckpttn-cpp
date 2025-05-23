# ---- Options ----

option(ENABLE_TEST_COVERAGE "Enable test coverage" OFF)
# option(TEST_INSTALLED_VERSION "Test the version found by find_package" OFF)

# ---- Dependencies ----

CPMAddPackage("gh:doctest/doctest#v2.4.11")
CPMAddPackage("gh:TheLartians/Format.cmake@1.7.3")

# if(TEST_INSTALLED_VERSION) find_package(${PROJECT_NAME} REQUIRED) else() CPMAddPackage(NAME
# ${PROJECT_NAME} SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/..) endif()

# ---- Create binary ----

file(GLOB sources CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)
add_executable(${PROJECT_NAME}Tests ${sources})
target_link_libraries(
  ${PROJECT_NAME}Tests doctest::doctest ${PROJECT_NAME}::${PROJECT_NAME} ${SPECIFIC_LIBS}
)
set_target_properties(${PROJECT_NAME}Tests PROPERTIES CXX_STANDARD 20)

# enable compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  # target_compile_options(${PROJECT_NAME} PUBLIC -Wall -Wpedantic -Wextra -Werror)
  target_compile_options(${PROJECT_NAME} PUBLIC -Wall -Wextra -Wno-unqualified-std-cast-call)
elseif(MSVC)
  target_compile_options(${PROJECT_NAME} PUBLIC /W4 /WX /wd4146 /wd4127 /wd4996)
  target_compile_definitions(${PROJECT_NAME}Tests PUBLIC DOCTEST_CONFIG_USE_STD_HEADERS)
endif()

# ---- Add ${PROJECT_NAME}Tests ----

enable_testing()

# Note: doctest and similar testing frameworks can automatically configure CMake tests. For other
# testing frameworks add the tests target instead: add_test(NAME ecgenTests COMMAND
# ${PROJECT_NAME}Tests)

string(TOLOWER ${PROJECT_NAME} PROJECT_FILE_NAME)
add_test(NAME ${PROJECT_FILE_NAME}Tests COMMAND ${PROJECT_NAME}Tests)

# ---- code coverage ----

if(ENABLE_TEST_COVERAGE)
  target_compile_options(${PROJECT_NAME} PUBLIC -O0 -g -fprofile-arcs -ftest-coverage)
  target_link_options(${PROJECT_NAME} PUBLIC -fprofile-arcs -ftest-coverage)
endif()
