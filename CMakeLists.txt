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

# -------------------- Subdirectories -------------------- #

add_subdirectory(channel)
add_subdirectory(common)
add_subdirectory(daemon)
add_subdirectory(lib)

add_subdirectory(docs)

# ----------------------- Testing ------------------------ #

include(CTest)
enable_testing()
# TODO: add testing features around

# ---------------------- Packaging ----------------------- #

# TODO: all packages are not signed as of now, check this
# page to add signature capabilities
# https://cmake.org/cmake/help/latest/cpack_gen/productbuild.html

# Minimum variables required to build a `binary` package
set(CPACK_PACKAGE_VENDOR "Retis Lab@Scuola Superiore Sant'Anna")
set(CPACK_PACKAGE_CONTACT "gabriele.serra@santannapisa.it")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

# RPM Specific options
set(CPACK_RPM_PACKAGE_LICENSE "GPLv2+") # TODO: check if the + is ok

# The list of CPack generators to use
set(CPACK_GENERATOR
    TGZ
    ZIP
    DEB
    RPM # Note: on RPM systems requires rpm to be installed!
)

# The list of CPack generators to use for `source` package
set(CPACK_SOURCE_GENERATOR
    TGZ
    ZIP
)

# Directories to ignore when building a `source` package
set(CPACK_SOURCE_IGNORE_FILES
    /.git
    /dist
    /.*build.*
    /\\\\.DS_Store
)

# This module will then handle everything
include(CPack)
