add_library(retif
    SHARED
    retif_lib.c
)

target_link_libraries(retif
    PRIVATE
    retif_channel
)

target_include_directories(retif
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
