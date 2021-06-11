add_library(retif_common
    STATIC
    list.c
)

target_include_directories(retif_common
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set_property(TARGET retif_common
    PROPERTY POSITION_INDEPENDENT_CODE ON)
