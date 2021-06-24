add_executable(retifd
    retif_daemon_main.c
    retif_config.c
    retif_daemon.c
    retif_plugin.c
    retif_scheduler.c
    retif_task.c
    retif_taskset.c
    retif_utils.c
    vector.c
)

# This will allow plugins to link against symbols provided by the daemon
# (without using multiple copies of the same code)
set_property(TARGET retifd
    PROPERTY ENABLE_EXPORTS 1
)

# find_package(PkgConfig REQUIRED)
# pkg_check_modules(YAML REQUIRED yaml)
# find_library(YAML yaml NAMES yaml)

target_include_directories(retifd
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(retifd
    PRIVATE
    ${CMAKE_DL_LIBS}
    retif_channel
    retif_common
    yaml
)

# ----------------------- Service ------------------------ #

# Substitute correct binary path in the initd script
set(INITDSCRIPT_IN ${CMAKE_CURRENT_SOURCE_DIR}/init.d/retif.in)
set(INITDSCRIPT_OUT ${CMAKE_CURRENT_BINARY_DIR}/init.d/retif)
configure_file(${INITDSCRIPT_IN} ${INITDSCRIPT_OUT} @ONLY)

# ------------------ Path Substitution ------------------- #

function(configure_header header_name)
    set(header_in ${CMAKE_CURRENT_SOURCE_DIR}/${header_name}.in)
    set(header_out ${CMAKE_CURRENT_BINARY_DIR}/${header_name})
    configure_file(${header_in} ${header_out} @ONLY)
endfunction(configure_header)

# Following headers need to be configured to substitute
# paths for installed files in the source code
configure_header(retif_config.h)
configure_header(retif_plugin.h)

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
