cmake_minimum_required(VERSION 3.18)
project(libarchivetests C)

include(FetchContent)

FetchContent_Declare(
  archive
  GIT_REPOSITORY https://github.com/libarchive/libarchive.git
  GIT_TAG v3.4.3)

FIND_PACKAGE(PkgConfig REQUIRED)
PKG_CHECK_MODULES(GLIB REQUIRED glib-2.0)
find_package(OpenSSL REQUIRED)

set(ENABLE_NETTLE OFF)
set(ENABLE_MBEDTLS OFF)
set(ENABLE_OPENSSL ON)
FetchContent_MakeAvailable(archive)

add_executable(libarchivetest libarchivetest.c)
target_include_directories(libarchivetest PRIVATE ${CMAKE_BINARY_DIR}/_deps/archive-src/libarchive ${GLIB_INCLUDE_DIRS})
target_link_libraries(libarchivetest archive ${GLIB_LIBRARIES})

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
