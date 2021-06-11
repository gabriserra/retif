add_library(retif_channel
    STATIC
    usocket.c
    retif_channel.c
)

target_link_libraries(retif_channel
    PUBLIC
    retif_common
)

target_include_directories(retif_channel
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set_property(TARGET retif_channel
    PROPERTY POSITION_INDEPENDENT_CODE ON)
