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
