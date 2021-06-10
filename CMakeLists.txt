# TODO: increase maximum tested CMake version
cmake_minimum_required(VERSION 3.16)

project(
    ReTiF
    VERSION 0.1.0
    DESCRIPTION "Userspace RT computing on Unix systems"
    LANGUAGES C
)

# --------------------- Build types ---------------------- #

set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
      STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# ------------------- Paths Management ------------------- #

# Install directory changes depending on build mode
if (CMAKE_BUILD_TYPE MATCHES "^[Dd]ebug")
    # During debug, the library will be installed into a local directory
    set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/_install
        CACHE PATH "" FORCE)
    set(CMAKE_INSTALL_INITDIR ${CMAKE_BINARY_DIR}/_install/init.d
        CACHE PATH "" FORCE)
else ()
    # This will install in /usr/lib and /usr/include
    set(CMAKE_INSTALL_PREFIX /usr CACHE PATH "" FORCE)
    set(CMAKE_INSTALL_INITDIR /etc/init.d CACHE PATH "" FORCE)
endif ()

# Specifies what to export when installing (using the
# directories provided by GNUInstallDirs)

# This module will export a set of variables containing
# paths in which stuff can be installed to. REQUIRED!
include(GNUInstallDirs)

# -------------------- Subdirectories -------------------- #

add_subdirectory(channel)
add_subdirectory(common)
add_subdirectory(daemon)
add_subdirectory(lib)

add_subdirectory(docs)
add_subdirectory(test)

# Packaging
include(${CMAKE_CURRENT_LIST_DIR}/packaging/CMakeLists.txt)
