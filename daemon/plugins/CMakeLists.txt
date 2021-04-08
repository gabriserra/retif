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

    target_include_directories(${PLUGIN}
        PRIVATE
        ../src
    )

    target_link_libraries(${PLUGIN}
        PRIVATE
        retifd_common
    )
endforeach()
