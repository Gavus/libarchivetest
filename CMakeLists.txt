cmake_minimum_required(VERSION 3.18)
project(libarchivetests C)

option(BUILD_LIBARCHIVE "Download and build libarchive" OFF)
if(${BUILD_LIBARCHIVE})
  include(FetchContent)

  FetchContent_Declare(
    archive
    GIT_REPOSITORY https://github.com/libarchive/libarchive.git
    GIT_TAG v3.4.3)

  find_package(OpenSSL REQUIRED)
  set(ENABLE_NETTLE OFF)
  set(ENABLE_MBEDTLS OFF)
  set(ENABLE_OPENSSL ON)

  FetchContent_MakeAvailable(archive)
endif()

FIND_PACKAGE(PkgConfig REQUIRED)
PKG_CHECK_MODULES(GLIB REQUIRED glib-2.0)

add_executable(encryption-speed-test libarchivetest.c)
target_include_directories(encryption-speed-test PRIVATE ${CMAKE_BINARY_DIR}/_deps/archive-src/libarchive ${GLIB_INCLUDE_DIRS})
target_link_libraries(encryption-speed-test archive ${GLIB_LIBRARIES})

add_executable(password-length-test pass.c)
target_include_directories(password-length-test PRIVATE ${CMAKE_BINARY_DIR}/_deps/archive-src/libarchive ${GLIB_INCLUDE_DIRS})
target_link_libraries(password-length-test archive ${GLIB_LIBRARIES})

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)
