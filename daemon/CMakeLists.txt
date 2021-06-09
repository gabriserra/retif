add_subdirectory(plugins)

add_executable(retifd
    daemon.c
    src/retif_config.c
    src/retif_daemon.c
    src/retif_plugin.c
    src/retif_scheduler.c
    src/retif_task.c
    src/retif_taskset.c
    src/retif_utils.c
)

target_include_directories(retifd
    PRIVATE
    src
)

target_link_libraries(retifd
    PRIVATE
    ${CMAKE_DL_LIBS}
    retif_channel
    retif_common
)

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

install(
    TARGETS retifd
    EXPORT retifd-config
    RUNTIME DESTINATION ${CMAKE_INSTALL_SBINDIR}
)

# Substitute correct binary path in the initd script
set(INITDSCRIPT_IN ${CMAKE_CURRENT_SOURCE_DIR}/retif.in)
set(INITDSCRIPT_OUT ${CMAKE_CURRENT_BINARY_DIR}/init.d/retif)
configure_file(${INITDSCRIPT_IN} ${INITDSCRIPT_OUT} @ONLY)

# TODO: this is statically like this, it should not be
# installed like this during debug!
install(
    PROGRAMS ${INITDSCRIPT_OUT}
    DESTINATION ${CMAKE_INSTALL_INITDIR}
)
