add_subdirectory(lib)
add_subdirectory(plugins)

add_executable(retifd
    daemon.c

    src/rts_channel.c
    src/rts_config.c
    src/rts_daemon.c
    src/rts_plugin.c
    src/rts_scheduler.c
    src/rts_task.c
    src/rts_taskset.c
    src/rts_utils.c

)

target_include_directories(retifd
    PRIVATE
    src
)

target_link_libraries(retifd
    PUBLIC
    ${CMAKE_DL_LIBS}

    PRIVATE
    retifd_common
)
