add_library(retifd_common
    STATIC
    list.c
    usocket.c
)

target_include_directories(retifd_common
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
