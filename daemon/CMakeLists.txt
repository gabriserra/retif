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

# Set where the daemon binary should be installed
set(CMAKE_INSTALL_DAEMONDIR ${CMAKE_INSTALL_SBIN})

# ----------------------- Service ------------------------ #

# Substitute correct binary path in the initd script
set(INITDSCRIPT_IN ${CMAKE_CURRENT_SOURCE_DIR}/retif.in)
set(INITDSCRIPT_OUT ${CMAKE_CURRENT_BINARY_DIR}/init.d/retif)
configure_file(${INITDSCRIPT_IN} ${INITDSCRIPT_OUT} @ONLY)

# -------------------- Configuration --------------------- #

set(RETIF_CONF_SRC ${CMAKE_CURRENT_SOURCE_DIR}/retif.conf)

# ----------------------- Install ------------------------ #

# Install the daemon
install(
    TARGETS retifd
    EXPORT retifd-config
    RUNTIME DESTINATION ${CMAKE_INSTALL_DAEMONDIR}
)

# Install the init.d script
install(
    PROGRAMS ${INITDSCRIPT_OUT}
    DESTINATION ${CMAKE_INSTALL_INITDIR}
)

# Install the configuration file
install(
    FILES ${RETIF_CONF_SRC}
    TYPE SYSCONF
)
