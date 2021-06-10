# cmake_minimum_required(VERSION 3.0)

set(LIBRARY_NAME retif)
set(LIBRARY_TYPE SHARED)
set(LIBRARY_VERSION 0.1.0)
set(LIBRARY_SOVERSION 0)

set(LIBRARY_INCLUDEDIR
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(LIBRARY_SRCFILES
    retif_lib.c
)

set(LIBRARY_DEPENDENCIES
    PRIVATE
    retif_channel
)

# -------------------------------------------------------- #

# Create a library with the given source files
add_library(${LIBRARY_NAME} ${LIBRARY_TYPE} ${LIBRARY_SRCFILES})

set_target_properties(${LIBRARY_NAME} PROPERTIES
    VERSION   ${LIBRARY_VERSION}
    SOVERSION ${LIBRARY_SOVERSION}
    INTERFACE_INCLUDE_DIRECTORIES ${LIBRARY_INCLUDEDIR}
)

# Set library dependencies
target_link_libraries(${LIBRARY_NAME} ${LIBRARY_DEPENDENCIES})

target_include_directories(${LIBRARY_NAME}
    # PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
    PUBLIC ${LIBRARY_INCLUDEDIR}
)

install(
    TARGETS ${LIBRARY_NAME}
    EXPORT ${LIBRARY_NAME}-config
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} # Windows specific
)

# The include directory must be named as the library
INSTALL (
    DIRECTORY ${LIBRARY_INCLUDEDIR}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h*"
)
