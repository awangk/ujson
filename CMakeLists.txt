# ujson

cmake_minimum_required(VERSION 2.8)

project(ujson)
enable_testing()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# enable c++11 on xcode
if (CMAKE_GENERATOR STREQUAL Xcode)
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
endif ()

if (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

# fetch re2c
include(ExternalProject)

set(RE2C_URL http://downloads.sourceforge.net/project/re2c/re2c)

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

  externalproject_add(re2c_external_ujson
    URL ${RE2C_URL}/0.13.5/re2c-0.13.5-bin.zip
    URL_MD5 7bb567b054d5cdacc7c5864ab25d2de4
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND "")

  add_executable(re2c IMPORTED)
  externalproject_get_property(re2c_external_ujson SOURCE_DIR)
  set_target_properties(re2c PROPERTIES IMPORTED_LOCATION ${SOURCE_DIR}/re2c)
  add_dependencies(re2c re2c_external_ujson)

elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

  externalproject_add(re2c_external_ujson
    URL ${RE2C_URL}/0.13.5/re2c-0.13.5.tar.gz
    URL_MD5 4a97d8f77ed6d2c76c8bd840a43f5633
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=<INSTALL_DIR>
    BUILD_COMMAND make
    INSTALL_COMMAND make install)

  add_executable(re2c IMPORTED)
  externalproject_get_property(re2c_external_ujson INSTALL_DIR)
  set_target_properties(re2c PROPERTIES
    IMPORTED_LOCATION ${INSTALL_DIR}/bin/re2c)
  add_dependencies(re2c re2c_external_ujson)

endif()

set_target_properties(re2c_external_ujson PROPERTIES FOLDER "ujson")

add_subdirectory(ujson)
add_subdirectory(test)
add_subdirectory(examples)

install(EXPORT ujson-export FILE ujson-targets.cmake DESTINATION cmake)
set(INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/include")

include(CMakePackageConfigHelpers)
configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/ujson-config.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/ujson-config.cmake"
  INSTALL_DESTINATION "${CMAKE_INSTALL_PREFIX}/cmake"
  PATH_VARS INCLUDE_INSTALL_DIR
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/ujson-config.cmake"
  DESTINATION "${CMAKE_INSTALL_PREFIX}/cmake"
)
