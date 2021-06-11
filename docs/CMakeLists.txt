# Option to disable generating documentation (default ON)
option(BUILD_DOC "Build documentation" OFF)

if (BUILD_DOC)
    # Check if Doxygen components are installed
    find_package(Doxygen
        REQUIRED
        dot
        OPTIONAL_COMPONENTS
        mscgen
        dia
    )

    if (DOXYGEN_FOUND)
        # The Doxyfile used to generate the docs is also
        # generated starting from an input file using the
        # configure option of CMake
        set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/CMakeDoxyfile.in)
        set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

        # If you enable the option ALL, the docs are built
        # together with the other targets
        add_custom_target( docs # ALL
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM
        )
    else (DOXYGEN_FOUND)
        message("Doxygen need to be installed to generate the doxygen documentation")
    endif (DOXYGEN_FOUND)
endif (BUILD_DOC)

# --------- Install and Packaging Documentation ---------- #

# TODO
