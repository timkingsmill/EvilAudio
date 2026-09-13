#[[
\file CMakeLists.txt
\brief Configures API documentation generation for EvilAudio.

\details
This CMake entry point orchestrates Doxygen and PlantUML setup for the
documentation toolchain. During configure, it:
- Resolves output directories for generated documentation artifacts.
- Configures Doxygen template variables used by doxygen/Doxyfile.in.
- Refreshes bundled C4-PlantUML include assets into the build tree.
- Optionally runs hpp2plantuml to generate header-derived .puml sources.
- Generates and invokes Doxygen, failing configure on non-zero exit.
]]

include("${CMAKE_CURRENT_SOURCE_DIR}/EvilAudioAPIDocumentation.cmake")

message(STATUS "============================================================")
message(STATUS "Configuring Doxygen and PlantUML documentation generation...")
increment_log_indent()

find_package(Doxygen)
if (NOT DOXYGEN_FOUND)
    message(FATAL_ERROR "Doxygen is required to generate documentation. Please install Doxygen and try again.")
else()
    message(STATUS "Doxygen found: ${DOXYGEN_EXECUTABLE}")
endif()

option(EVIL_ENABLE_HPP2PLANTUML "Enable hpp2plantuml header diagram generation workflow" OFF)
message(STATUS "hpp2plantuml workflow enabled: ${EVIL_ENABLE_HPP2PLANTUML}")

# =========================================================================================
#[[
\brief Resolves standard documentation output directories.

\details
The helper defines three locations used consistently across this file:
- docs root output directory.
- HTML output directory.
- PlantUML source output directory under HTML output.
]]
get_documentation_file_and_directories(
    _output_directory
    _html_output_directory
    _puml_output_directory

)

# =========================================================================================
#[[
\brief Sets core Doxygen project metadata.

\details
These variables are consumed by doxygen/Doxyfile.in when creating the final
Doxyfile in the binary directory.
]]
set(EVIL_PROJECT_NAME  "EvilAudio")
set(EVIL_PROJECT_BRIEF "EvilAudio is a C++ library for audio processing and synthesis, designed to be fast, flexible, and easy to use.")

# =========================================================================================
#[[
\brief Maps resolved directories to Doxygen output variables.

\details
Generated files are emitted only to the build tree to keep source directories
clean and reproducible across reconfigure runs.
]]
set(EVIL_OUTPUT_DIRECTORY ${_output_directory})
message(STATUS "Doxygen output directory set to: ${EVIL_OUTPUT_DIRECTORY}")

set(EVIL_DOXYGEN_HTML_OUTPUT_DIR ${_html_output_directory})
message(STATUS "Doxygen HTML output directory set to: ${EVIL_DOXYGEN_HTML_OUTPUT_DIR}")

# =========================================================================================
#[[
\brief Sets the Doxygen HTML layout file.

\details
This file customizes the appearance and structure of the generated HTML documentation.
]]
set(EVIL_HTML_LAYOUT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/doxygen/EvilAudioAPIDocsLayout.xml)
if (NOT EXISTS "${EVIL_HTML_LAYOUT_FILE}")
    message(FATAL_ERROR "Doxygen HTML layout file not found at: ${EVIL_HTML_LAYOUT_FILE}")
else()
    message(STATUS "Doxygen HTML layout file set to: ${EVIL_HTML_LAYOUT_FILE}")
endif()

set(EVIL_HTML_EXTRA_STYLESHEET
    "${CMAKE_CURRENT_SOURCE_DIR}/doxygen/css/header.css"
    "${CMAKE_CURRENT_SOURCE_DIR}/doxygen/css/evil-audio-overrides.css"
)
string(REPLACE ";" " " EVIL_HTML_EXTRA_STYLESHEET "${EVIL_HTML_EXTRA_STYLESHEET}")
message(STATUS "Doxygen HTML extra stylesheet file set to: ${EVIL_HTML_EXTRA_STYLESHEET}")

# =========================================================================================
#[[
\brief Defines the source tree roots scanned by Doxygen.
]]
set(EVIL_INPUT 
    "${CMAKE_SOURCE_DIR}/source/" 
    "${CMAKE_SOURCE_DIR}/docs/puml/"
)
string(REPLACE ";" " " EVIL_INPUT "${EVIL_INPUT}")
message(STATUS "Doxygen input directory set to: ${EVIL_INPUT}")

# =========================================================================================
#[[
\brief Declares Doxygen exclude paths and glob patterns.

\details
CMake stores lists as semicolon-delimited values, while Doxygen expects
space-delimited strings for EXCLUDE and EXCLUDE_PATTERNS.
]]
set(DOXYGEN_EXCLUDE_DIRS
    #${CMAKE_BINARY_DIR}    # Exclude the entire build tree to prevent generated files from being scanned.

    #${CMAKE_SOURCE_DIR}/docs/puml/themes
    ${CMAKE_SOURCE_DIR}/source/libs/asiosdk
    ${CMAKE_SOURCE_DIR}/source/testing
) 
string(REPLACE ";" " " EVIL_EXCLUDE "${DOXYGEN_EXCLUDE_DIRS}")


set(DOXYGEN_EXCLUDE_PATTERNS
    ${CMAKE_SOURCE_DIR}/build/**/*
    ${CMAKE_SOURCE_DIR}/docs/**/*
    ${CMAKE_SOURCE_DIR}/.venv/**/*
    ${CMAKE_SOURCE_DIR}/.git/**/*
    ${CMAKE_SOURCE_DIR}/.github/**/*
    ${CMAKE_SOURCE_DIR}/.vscode/**/*
    ${CMAKE_SOURCE_DIR}/extras/cmake/**/*
    ${CMAKE_SOURCE_DIR}/CMakeLists.txt)

message(STATUS "Excluding files and directories from doxygen input:")
increment_log_indent()
foreach(exclude_item ${DOXYGEN_EXCLUDE_DIRS})
    message(STATUS "  - ${exclude_item}")
endforeach()
decrement_log_indent()

# Doxygen expects the EXCLUDE variable to be a space-separated list, 
# so we replace the semicolons with spaces.
string(REPLACE ";" " " EVIL_EXCLUDE_PATTERNS "${DOXYGEN_EXCLUDE_PATTERNS}")

# ===============================================================================================
#[[
\brief Refreshes C4-PlantUML include assets into the build tree.

\details
The destination is recreated each configure run to prevent stale include files
from previous builds from affecting rendered diagrams.
]]
set(_C4_PLANTUML_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/C4-PlantUML")
set(_C4_PLANTUML_DESTINATION_ROOT "${CMAKE_BINARY_DIR}/docs/tools")
set(_C4_PLANTUML_DESTINATION_DIR "${_C4_PLANTUML_DESTINATION_ROOT}/C4-PlantUML")

if(EXISTS "${_C4_PLANTUML_SOURCE_DIR}")
    message(STATUS "C4-PlantUML directory found: ${_C4_PLANTUML_SOURCE_DIR}")

    # Always refresh to avoid stale assets in incremental configure runs.
    if(EXISTS "${_C4_PLANTUML_DESTINATION_DIR}")
        file(REMOVE_RECURSE "${_C4_PLANTUML_DESTINATION_DIR}")
    endif()

    file(MAKE_DIRECTORY "${_C4_PLANTUML_DESTINATION_ROOT}")
    file(COPY "${_C4_PLANTUML_SOURCE_DIR}" DESTINATION "${_C4_PLANTUML_DESTINATION_ROOT}")

    # Validate copy result using a well-known file from C4-PlantUML.
    if(NOT EXISTS "${_C4_PLANTUML_DESTINATION_DIR}/C4_Context.puml")
        message(FATAL_ERROR
            "Failed to refresh C4-PlantUML assets at: ${_C4_PLANTUML_DESTINATION_DIR}")
    endif()

    message(STATUS "Copied C4-PlantUML directory to: ${_C4_PLANTUML_DESTINATION_ROOT}")
else()
    message(FATAL_ERROR
        "C4-PlantUML directory not found at: ${_C4_PLANTUML_SOURCE_DIR}. "
        "Please ensure the directory exists and try again.")
endif()

message(STATUS "C4-PlantUML directory set to: ${_C4_PLANTUML_SOURCE_DIR}")
    
# =========================================================================================
#[[
\brief Configures PlantUML integration variables for Doxygen.

\details
This block resolves the PlantUML JAR/config and defines include/file search
paths used by \c PLANTUML_INCLUDE_PATH and \c PLANTUMLFILE_DIRS.
]]
message(STATUS "Setting up PlantUML integration with Doxygen...")
increment_log_indent()
create_plantuml_config(jar_file plantuml_config_file)

message(STATUS "PlantUML jar file found: ${jar_file}")
message(STATUS "PlantUML configuration file found: ${plantuml_config_file}")

# Update the Doxygen configuration variable for the PlantUML JAR file path.
set(EVIL_PLANTUML_JAR_PATH "${jar_file}") 
set(EVIL_PLANTUML_CFG_FILE "${plantuml_config_file}")


# Todo - Consider adding more configuration parameters here, such as:
# - EVIL_ENABLE_PLANTUML: Whether to enable PlantUML diagrams in Doxygen.
# - EVIL_DOXYGEN_PROJECT_LOGO: Path to a logo image to include in the Doxygen documentation.
# - EVIL_DOXYGEN_GENERATE_TREEVIEW: Whether to enable the tree view in the HTML output.
# - EVIL_DOXYGEN_GENERATE_LATEX: Whether to generate LaTeX output in addition to HTML.  

# Set the directories to search for PlantUML files. 
# Doxygen will look for .puml files in these directories.
set(EVIL_PLANTUML_FILE_DIRS
    "${CMAKE_SOURCE_DIR}/docs/puml/diagrams"
    "${CMAKE_SOURCE_DIR}/docs/puml/themes"
)
string(REPLACE ";" " " EVIL_PLANTUML_FILE_DIRS "${EVIL_PLANTUML_FILE_DIRS}")

# Set the include path for PlantUML, which allows us to use !include in our .puml 
# files to include common definitions and themes.
set(EVIL_PLANTUML_INCLUDE_PATH
    "${CMAKE_SOURCE_DIR}/docs/puml/themes"
    "${CMAKE_CURRENT_SOURCE_DIR}/C4-PlantUML"
    "${CMAKE_CURRENT_SOURCE_DIR}/C4-PlantUML/themes"
)
string(REPLACE ";" " " EVIL_PLANTUML_INCLUDE_PATH "${EVIL_PLANTUML_INCLUDE_PATH}")

decrement_log_indent()

#[[
\brief Optionally generates PlantUML source diagrams from project headers.

\details
When EVIL_ENABLE_HPP2PLANTUML is ON, hpp2plantuml scans header globs and emits
generated .puml files into the PlantUML output directory consumed by Doxygen.
]]
set(_output_dir "${_puml_output_directory}")
if(EVIL_ENABLE_HPP2PLANTUML)
    if (NOT EXISTS "${_output_dir}")
        file(MAKE_DIRECTORY "${_output_dir}")
        message(STATUS "Created PlantUML output directory: ${_output_dir}")
    else()
        message(STATUS "PlantUML output directory already exists: ${_output_dir}")
    endif()

    # Generate PlantUML source diagrams from all headers in the source directory.
    run_hpp2plantuml_header_globs_for_doxygen(
        OUTPUT_FILE_H "${_output_dir}/headers.puml"
        OUTPUT_FILE_HPP "${_output_dir}/headers_hpp.puml"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        VENV_PATH "${CMAKE_SOURCE_DIR}/.venv"
        SUPPRESS_PARSE_WARNINGS
    )
else()
    message(STATUS "Skipping hpp2plantuml generation workflow (EVIL_ENABLE_HPP2PLANTUML=OFF)")
endif()

# =========================================================================================
#[[
\brief Materializes Doxygen config from the template.

\details
create_doxygen_config_file() substitutes the variables configured above into
doxygen/Doxyfile.in and writes the resulting Doxyfile to the binary tree.
]]
set(_doxygen_config_file "")
create_doxygen_config_file(_doxygen_config_file)
message(STATUS "Doxygen configuration file generated at: ${_doxygen_config_file}")
set(_doxygen_working_directory "${CMAKE_CURRENT_BINARY_DIR}")
message(STATUS "Doxygen working directory set to: ${_doxygen_working_directory}")

#[[
\brief Defines a standalone target to generate only header-derived PlantUML.

\details
The custom target writes and runs a generated script so the same helper logic
can be reused without executing the full Doxygen pass.
]]
if(EVIL_ENABLE_HPP2PLANTUML)
    set(_plantuml_only_script "${CMAKE_CURRENT_BINARY_DIR}/GeneratePlantUMLOnly.cmake")
    set(_plantuml_only_script_template [=[
        set(CMAKE_SOURCE_DIR "@CMAKE_SOURCE_DIR@")
        set(CMAKE_BINARY_DIR "@CMAKE_BINARY_DIR@")

        include("@CMAKE_CURRENT_SOURCE_DIR@/EvilAudioAPIDocumentation.cmake")

        run_hpp2plantuml_header_globs_for_doxygen(
            OUTPUT_FILE_H "@_output_dir@/plantuml_all_headers_h.puml"
            OUTPUT_FILE_HPP "@_output_dir@/plantuml_all_headers_hpp.puml"
            WORKING_DIRECTORY "@CMAKE_SOURCE_DIR@"
            VENV_PATH "@CMAKE_SOURCE_DIR@/.venv"
            QUIET
            SUPPRESS_PARSE_WARNINGS
        )
    ]=])

    string(CONFIGURE "${_plantuml_only_script_template}" _plantuml_only_script_content @ONLY)
    file(WRITE "${_plantuml_only_script}" "${_plantuml_only_script_content}")

    add_custom_target(generate_plantuml_only
        COMMAND ${CMAKE_COMMAND} -E echo "Generating PlantUML source diagrams from all headers in ${CMAKE_SOURCE_DIR}/source"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${_output_dir}"
        COMMAND ${CMAKE_COMMAND} -P "${_plantuml_only_script}"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        COMMENT "Generating PlantUML diagrams without running full Doxygen"
        USES_TERMINAL
        VERBATIM
    )
endif()

# ==================================================================================
#[[
\brief Runs Doxygen and fails configuration on non-zero exit code.
]]
message(STATUS "=============================================")
message(STATUS "Invoking Doxygen to generate documentation...")
increment_log_indent()
execute_process(
    COMMAND ${DOXYGEN_EXECUTABLE} "${_doxygen_config_file}"
    WORKING_DIRECTORY "${_doxygen_working_directory}"
    RESULT_VARIABLE _doxygen_result
)
if(NOT _doxygen_result EQUAL 0)
    message(FATAL_ERROR "Doxygen failed with exit code ${_doxygen_result}")
endif()
decrement_log_indent()
message(STATUS "Doxygen documentation generation completed.")

# ==================================================================================
# ==================================================================================

# 
#####run_doxygen_build_py()

# ==================================================================================
# ==================================================================================

