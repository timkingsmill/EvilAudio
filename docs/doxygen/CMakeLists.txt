# Require CMake version
cmake_minimum_required(VERSION 3.12)
project(EvilAudioDoxygen VERSION 1.0)

# Find Doxygen package
find_package(Doxygen)

# Check if Doxygen was found before proceeding
if (DOXYGEN_FOUND)
    message(STATUS "Doxygen found, version: ${DOXYGEN_VERSION}")

    # (Optional) Customize Doxygen configuration parameters
    # Any Doxygen config tag can be set by a CMake variable prefixed with DOXYGEN_
    set(DOXYGEN_PROJECT_NAME "${PROJECT_NAME} Documentation")
    set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/docs")
    
    set(DOXYGEN_EXTRACT_ALL          YES)
    set(DOXYGEN_HIDE_UNDOC_RELATIONS NO)

    set(DOXYGEN_HAVE_DOT             YES)
    set(DOXYGEN_DOT_IMAGE_FORMAT     svg)
    set(DOXYGEN_INTERACTIVE_SVG      YES)
    
    set(DOXYGEN_CLASS_GRAPH          YES)
    
    set(DOXYGEN_COLLABORATION_GRAPH  NO)
    set(DOXYGEN_UML_LOOK             YES)
    set(DOXYGEN_UML_LIMIT_NUM_FIELDS 50)
    set(DOXYGEN_TEMPLATE_RELATIONS   YES)
    set(DOXYGEN_DOT_GRAPH_MAX_NODES  100)
    set(DOXYGEN_MAX_DOT_GRAPH_DEPTH  10)
    set(DOXYGEN_DOT_TRANSPARENT      YES)
    set(DOXYGEN_CALL_GRAPH           YES)
    set(DOXYGEN_CALLER_GRAPH         YES)
    set(DOXYGEN_GRAPHICAL_HIERARCHY  YES)
    set(DOXYGEN_TREESCALE            0.2)

    set(DOXYGEN_GENERATE_LATEX       NO)
    set(DOXYGEN_GENERATE_RTF         NO)
    set(DOXYGEN_GENERATE_XML         NO)
    set(DOXYGEN_GENERATE_DOCSET      NO)
    set(DOXYGEN_GENERATE_HTML        YES)
    set(DOXYGEN_GENERATE_MAN         NO)
    set(DOXYGEN_GENERATE_TREEVIEW    YES) 


    # Add the documentation target
    # doxygen_add_docs(target_name source_files/dirs [ALL])
    # The 'ALL' option ensures the docs are built with the main application build
     # Add the documentation target
    # The 'doc' target will scan the current source directory and create a 'doc' build target
    doxygen_add_docs(
        doc                  # Name of the custom target (e.g., you can run `cmake --build . --target doc`)
        "${CMAKE_SOURCE_DIR}/source"  # Input directory to scan for source files
        ALL                  # Adds the target to the default build, so it runs with `cmake --build .`
        COMMENT "Generating API documentation with Doxygen" # Comment for the custom command
    )
else()
    message(WARNING "Doxygen not found. Documentation target will not be generated.")
endif()
