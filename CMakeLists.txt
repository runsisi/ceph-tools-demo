cmake_minimum_required(VERSION 2.8.12)

project(utils)

if(CMAKE_CXX_COMPILER_ID STREQUAL GNU)
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7)
    message(FATAL_ERROR "GCC 7+ required due to C++17 requirements")
  endif()
endif()

if(CMAKE_VERSION VERSION_LESS "3.8")
  include(CheckCXXCompilerFlag)
  CHECK_CXX_COMPILER_FLAG("-std=c++17" COMPILER_SUPPORTS_CXX17)
  if(NOT COMPILER_SUPPORTS_CXX17)
    message(FATAL_ERROR
      "The compiler ${CMAKE_CXX_COMPILER} has no C++17 support.")
  endif()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
  include(CheckCCompilerFlag)
  CHECK_C_COMPILER_FLAG("-std=gnu99" COMPILER_SUPPORTS_GNU99)
  if(NOT COMPILER_SUPPORTS_GNU99)
    message(FATAL_ERROR
      "The compiler ${CMAKE_C_COMPILER} has no GNU C99 support.")
  endif()
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")
else()
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_EXTENSIONS OFF)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_C_STANDARD 99)
  set(CMAKE_C_EXTENSIONS ON)
  set(C_STANDARD_REQUIRED ON)
endif()

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "/usr" CACHE PATH "" FORCE)
endif()

include(GNUInstallDirs)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
if(NOT CMAKE_INSTALL_RPATH)
  set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_FULL_LIBDIR}/ceph")
endif()

include_directories(
  ${PROJECT_SOURCE_DIR}/../build/boost/include
  ${PROJECT_SOURCE_DIR}/../build/include
  ${PROJECT_SOURCE_DIR}/../build/src/include
  ${PROJECT_SOURCE_DIR}/../src
  ${PROJECT_SOURCE_DIR}/third
  ${PROJECT_SOURCE_DIR}
)
link_directories(
  ${PROJECT_SOURCE_DIR}/../build/boost/lib
  ${PROJECT_SOURCE_DIR}/../build/lib
)

add_subdirectory(common)
add_subdirectory(list_snaps)
