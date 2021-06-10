foreach(PLUGIN IN ITEMS
    "sched_EDF"
    "sched_RR"
    "sched_FP"
    "sched_RM"
)
    add_library(${PLUGIN}
        MODULE
        ${PLUGIN}.c
        # ${CMAKE_CURRENT_SOURCE_DIR}/../src/retif_task.c
        # ${CMAKE_CURRENT_SOURCE_DIR}/../src/retif_taskset.c
        # ${CMAKE_CURRENT_SOURCE_DIR}/../src/retif_utils.c
    )

    set_target_properties(${PLUGIN} PROPERTIES PREFIX "")

    target_include_directories(${PLUGIN}
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../src
        ${CMAKE_CURRENT_BINARY_DIR}/..
    )

    target_link_libraries(${PLUGIN}
        PRIVATE
        retif_common

        # Create a plugin that uses the API provided by the
        # daemon executable. Each plugin module "links" to
        # the executable to use the symbols. On Linux, this
        # is not strictly necessary and out-of-tree plugins
        # can be built without having access to the daemon
        # binary.
        retifd
    )

    # Install each plugin
    install(
        TARGETS ${PLUGIN}
        EXPORT retif-plugin-${PLUGIN}-config
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/retif
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/retif
    )
endforeach()
