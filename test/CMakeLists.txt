# TODO: add testing features around
include(CTest)
enable_testing()

add_executable(retif_client
    client.c
)

target_link_libraries(retif_client
    PRIVATE
    retif
)
