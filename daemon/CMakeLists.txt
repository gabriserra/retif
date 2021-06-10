# -------------------- Configuration --------------------- #

set(RETIF_CONF_FNAME retif.conf)
set(RETIF_CONF_SRC  ${CMAKE_CURRENT_SOURCE_DIR}/${RETIF_CONF_FNAME})

# Set all destination paths for various components that will be installed
set(RETIF_CONF_PATH ${CMAKE_INSTALL_FULL_SYSCONFDIR}/${RETIF_CONF_FNAME})
set(RETIF_PLUGINS_PATH ${CMAKE_INSTALL_FULL_LIBDIR}/retif)
set(RETIF_DAEMON_PATH ${CMAKE_INSTALL_FULL_SBINDIR})

message(RETIF_CONF_PATH: ${RETIF_CONF_PATH})
message(RETIF_PLUGINS_PATH: ${RETIF_PLUGINS_PATH})
message(RETIF_DAEMON_PATH: ${RETIF_DAEMON_PATH})

# ----------------------- Plugins ------------------------ #

add_subdirectory(plugins)

# ------------------------ Daemon ------------------------ #

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

# This will allow plugins to link against symbols provided by the daemon
# (without using multiple copies of the same code)
set_property(TARGET retifd
    PROPERTY ENABLE_EXPORTS 1
)

target_include_directories(retifd
    PRIVATE
    src
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(retifd
    PRIVATE
    ${CMAKE_DL_LIBS}
    retif_channel
    retif_common
)

# ----------------------- Service ------------------------ #

# Substitute correct binary path in the initd script
set(INITDSCRIPT_IN ${CMAKE_CURRENT_SOURCE_DIR}/retif.in)
set(INITDSCRIPT_OUT ${CMAKE_CURRENT_BINARY_DIR}/init.d/retif)
configure_file(${INITDSCRIPT_IN} ${INITDSCRIPT_OUT} @ONLY)

# ------------------ Path Substitution ------------------- #

# Substitute correct path of the configuration in the corresponding header file
set(RETIF_CONF_H retif_config.h)
set(RETIF_CONF_H_IN ${CMAKE_CURRENT_SOURCE_DIR}/src/${RETIF_CONF_H}.in)
set(RETIF_CONF_H_OUT ${CMAKE_CURRENT_BINARY_DIR}/${RETIF_CONF_H})
configure_file(${RETIF_CONF_H_IN} ${RETIF_CONF_H_OUT} @ONLY)

# Substitute correct path of the plugin DLLs in the corresponding header file
set(RETIF_PLUGIN_H retif_plugin.h)
set(RETIF_PLUGIN_H_IN ${CMAKE_CURRENT_SOURCE_DIR}/src/${RETIF_PLUGIN_H}.in)
set(RETIF_PLUGIN_H_OUT ${CMAKE_CURRENT_BINARY_DIR}/${RETIF_PLUGIN_H})
configure_file(${RETIF_PLUGIN_H_IN} ${RETIF_PLUGIN_H_OUT} @ONLY)

# ----------------------- Install ------------------------ #

# Install the daemon
install(
    TARGETS retifd
    EXPORT retifd-config
    RUNTIME DESTINATION ${CMAKE_INSTALL_SBINDIR}
)

# Install the init.d script
install(
    PROGRAMS ${INITDSCRIPT_OUT}
    DESTINATION ${CMAKE_INSTALL_INITDIR}
)

# Install the configuration file
install(
    FILES ${RETIF_CONF_SRC}
    DESTINATION ${CMAKE_INSTALL_FULL_SYSCONFDIR}
)
