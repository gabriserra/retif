foreach(PLUGIN IN ITEMS
    "sched_EDF"
    "sched_RR"
    "sched_FP"
    "sched_RM"
)
    add_library(${PLUGIN}
        MODULE
        ${PLUGIN}.c
    )

    set_target_properties(${PLUGIN} PROPERTIES PREFIX "")

    target_include_directories(${PLUGIN}
        PRIVATE
        ../src
    )

    target_link_libraries(${PLUGIN}
        PRIVATE
        retif_common
    )

    # Install each plugin
    install(
        TARGETS ${PLUGIN}
        EXPORT retif-plugin-${PLUGIN}-config
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/retif
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/retif
    )
endforeach()
