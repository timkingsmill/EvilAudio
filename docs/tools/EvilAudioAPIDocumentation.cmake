include_guard(GLOBAL)

##\file evil_doxygen.cmake
##\brief Helper utilities for configuring Doxygen and PlantUML assets.
##
##\details
##Provides reusable CMake functions used by the documentation toolchain to:
##- Materialize a configured \c Doxyfile in the build tree.
##- Locate the most recent PlantUML JAR.
##- Normalize PlantUML paths for use by downstream Doxygen settings.

##\brief Imports shared logging wrappers used by this module.
##
##\details
##The included module centralizes project logging behavior (indentation and
##message formatting) so diagnostics emitted here remain consistent with the
##rest of the build system.
include("${CMAKE_SOURCE_DIR}/extras/cmake/EvilAudioLogging.cmake")

function(get_documentation_file_and_directories 
            output_directory
            html_output_directory
            puml_output_directory
)
    set(${output_directory}         "${CMAKE_BINARY_DIR}/docs" PARENT_SCOPE)
    set(${html_output_directory}    "${CMAKE_BINARY_DIR}/docs/html" PARENT_SCOPE)
    set(${puml_output_directory}    "${CMAKE_BINARY_DIR}/docs/html/puml" PARENT_SCOPE)

endfunction()

##\name Doxygen Configuration Functions
##\brief Functions responsible for generating and preparing Doxygen inputs.
##@{

##\brief Creates a Doxygen configuration file in the current binary directory.
##
##\details If a \c Doxyfile.in template exists in the current source directory, it is
##processed with CMake variable substitution (\c @ONLY mode) to produce the output
##\c Doxyfile. Otherwise, an existing \c Doxyfile is copied verbatim to the binary
##directory.
##
##\param[out] DOXYFILE Variable name to store the path to the generated Doxyfile.
##This variable is set in the parent scope.
##
function(create_doxygen_config_file DOXYFILE)
    set(_doxyfile_config_input "${CMAKE_CURRENT_SOURCE_DIR}/doxygen/Doxyfile.in")
    set(_doxyfile_config_output "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile")

    if(EXISTS "${_doxyfile_config_input}")
        configure_file("${_doxyfile_config_input}" "${_doxyfile_config_output}" @ONLY)
    else()
        configure_file("${CMAKE_CURRENT_SOURCE_DIR}/doxygen/Doxyfile" "${_doxyfile_config_output}" COPYONLY)
    endif()
    set(${DOXYFILE} "${_doxyfile_config_output}" PARENT_SCOPE)
endfunction()

##\brief Configures the PlantUML integration for Doxygen documentation.
##
##\details Locates the newest PlantUML JAR file in the external directory and
##copies it alongside the source JAR using the stable filename \c plantuml.jar
##so Doxygen can reference a predictable path during documentation generation.
##
function(create_plantuml_config jar_file plantuml_config_file)
    # Find the newest PlantUML JAR file.
    _find_newest_plantuml_jar_file(_source_plantuml_jar_file)
    if (NOT EXISTS "${_source_plantuml_jar_file}")
        message(FATAL_ERROR "PlantUML JAR file not found. Expected to find at least one JAR in the external/plantUML directory.")
    endif()

    # Copy the JAR to the binary directory with a stable name so Doxygen can reference it predictably.
    set(_destination_jar_file_name "${CMAKE_CURRENT_BINARY_DIR}/plantuml.jar")
    file(COPY_FILE ${_source_plantuml_jar_file} ${_destination_jar_file_name})
    # Set the output variable to the path of the copied JAR file for use in Doxygen configuration.
    set(${jar_file} "${_destination_jar_file_name}" PARENT_SCOPE)

    # Attempt to locate a PlantUML configuration file in the same directory as the JAR. 
    # If it doesn't exist, log a warning and proceed with default settings.
    set(_plantuml_source_config_file "${CMAKE_CURRENT_SOURCE_DIR}/plantUML/plantuml.cfg")
    set(_plantuml_destination_config_file "${CMAKE_CURRENT_BINARY_DIR}/plantuml.cfg")
    if(EXISTS "${_plantuml_source_config_file}")
        file(COPY_FILE ${_plantuml_source_config_file} ${_plantuml_destination_config_file})
        set(${plantuml_config_file} "${_plantuml_destination_config_file}" PARENT_SCOPE)
    else()
        message(WARNING "PlantUML config file not found: ${_plantuml_source_config_file}. Default settings will be used.")
    endif()
endfunction()

##\brief Resolves a Python interpreter for the documentation toolchain.
##
##\details
##Prefers an explicitly provided interpreter, then a provided virtual
##environment, then the active shell virtual environment, then
##\c ${CMAKE_SOURCE_DIR}/.venv, and finally falls back to
##\c find_package(Python3).
##
##\param[out] out_python Variable name that receives the resolved interpreter path.
##
##\param[out] out_python_source Variable name that receives a description of how
##the interpreter was chosen.
##
function(_evil_doxygen_resolve_python_interpreter out_python out_python_source)
    set(options)
    set(oneValueArgs PYTHON_EXECUTABLE VENV_PATH CALLER)
    cmake_parse_arguments(EVIL_DOXY_PY "${options}" "${oneValueArgs}" "" ${ARGN})

    set(_python "${EVIL_DOXY_PY_PYTHON_EXECUTABLE}")
    set(_python_source "PYTHON_EXECUTABLE argument")

    if(NOT _python)
        set(_venv_root "")
        if(EVIL_DOXY_PY_VENV_PATH)
            set(_venv_root "${EVIL_DOXY_PY_VENV_PATH}")
        elseif(DEFINED ENV{VIRTUAL_ENV} AND NOT "$ENV{VIRTUAL_ENV}" STREQUAL "")
            set(_venv_root "$ENV{VIRTUAL_ENV}")
        elseif(EXISTS "${CMAKE_SOURCE_DIR}/.venv")
            set(_venv_root "${CMAKE_SOURCE_DIR}/.venv")
        endif()

        if(_venv_root)
            if(WIN32)
                if(EXISTS "${_venv_root}/Scripts/python.exe")
                    set(_python "${_venv_root}/Scripts/python.exe")
                endif()
            else()
                if(EXISTS "${_venv_root}/bin/python3")
                    set(_python "${_venv_root}/bin/python3")
                elseif(EXISTS "${_venv_root}/bin/python")
                    set(_python "${_venv_root}/bin/python")
                endif()
            endif()

            if(_python)
                set(_python_source "virtual environment")
            endif()
        endif()
    endif()

    if(NOT _python)
        find_package(Python3 COMPONENTS Interpreter QUIET)
        if(Python3_Interpreter_FOUND)
            set(_python "${Python3_EXECUTABLE}")
            set(_python_source "find_package(Python3)")
        endif()
    endif()

    if(NOT _python)
        set(_caller "_evil_doxygen_resolve_python_interpreter")
        if(EVIL_DOXY_PY_CALLER)
            set(_caller "${EVIL_DOXY_PY_CALLER}")
        endif()

        message(FATAL_ERROR
            "${_caller}(): Python interpreter not found. "
            "Install Python 3, activate a venv, or pass PYTHON_EXECUTABLE.")
    endif()

    set(${out_python} "${_python}" PARENT_SCOPE)
    set(${out_python_source} "${_python_source}" PARENT_SCOPE)
endfunction()

##\brief Ensures the Python \c hpp2plantuml package is installed for docs generation.
##
##\details
##Uses the resolved interpreter to check whether \c hpp2plantuml is installed.
##If it is missing, the function installs it with \c pip before docs generation
##continues.
##
function(_evil_doxygen_ensure_hpp2plantuml_installed)
    set(options QUIET)
    set(oneValueArgs PYTHON_EXECUTABLE VENV_PATH)
    cmake_parse_arguments(EVIL_DOXY_H2P "${options}" "${oneValueArgs}" "" ${ARGN})

    _evil_doxygen_resolve_python_interpreter(
        _python
        _python_source
        PYTHON_EXECUTABLE "${EVIL_DOXY_H2P_PYTHON_EXECUTABLE}"
        VENV_PATH "${EVIL_DOXY_H2P_VENV_PATH}"
        CALLER "_evil_doxygen_ensure_hpp2plantuml_installed"
    )

    execute_process(
        COMMAND "${_python}" -m pip show hpp2plantuml
        RESULT_VARIABLE _hpp2plantuml_installed
        OUTPUT_QUIET
        ERROR_QUIET
    )

    if(_hpp2plantuml_installed EQUAL 0)
        if(NOT EVIL_DOXY_H2P_QUIET)
            message(STATUS "hpp2plantuml is already installed")
        endif()
        return()
    endif()

    if(NOT EVIL_DOXY_H2P_QUIET)
        message(STATUS "Installing hpp2plantuml with ${_python} -m pip (${_python_source})")
    endif()

    execute_process(
        COMMAND "${_python}" -m pip install hpp2plantuml
        RESULT_VARIABLE _install_result
        OUTPUT_VARIABLE _install_stdout
        ERROR_VARIABLE _install_stderr
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_STRIP_TRAILING_WHITESPACE
    )

    if(NOT _install_result EQUAL 0)
        message(FATAL_ERROR
            "Failed to install hpp2plantuml using interpreter: ${_python}\n"
            "pip install output:\n${_install_stdout}\n${_install_stderr}")
    endif()

    if(NOT EVIL_DOXY_H2P_QUIET)
        message(STATUS "hpp2plantuml installed successfully")
    endif()
endfunction()

##\brief Executes the docs/tools/doxygen/build.py script.
##
##\details
##Resolves a Python interpreter using the same precedence as the rest of the
##documentation toolchain, then runs \c build.py from the doxygen script
##directory. Additional arguments can be passed through to the script.
##
##\param[in] PYTHON_EXECUTABLE Optional Python interpreter override.
##
##\param[in] VENV_PATH Optional virtual environment root.
##
##\param[in] WORKING_DIRECTORY Optional working directory for script execution.
##Defaults to the directory containing \c build.py.
##
##\param[in] DOXYFILE Optional path to the Doxygen configuration file.
##If omitted, defaults to \c ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile when present.
##
##\param[in] EXTRA_ARGS Additional arguments forwarded directly to
##\c docs/tools/doxygen/build.py.
##
##\param[in] QUIET Suppress status logging for script output.
##
##\param[in] FAIL_ON_ERROR Treat script failure as a configure-time fatal error.
##
function(run_doxygen_build_py)
    set(options QUIET FAIL_ON_ERROR)
    set(oneValueArgs PYTHON_EXECUTABLE VENV_PATH WORKING_DIRECTORY DOXYFILE)
    set(multiValueArgs EXTRA_ARGS)
    cmake_parse_arguments(EVIL_DOXY_BUILD "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    _evil_doxygen_resolve_python_interpreter(
        _python
        _python_source
        PYTHON_EXECUTABLE "${EVIL_DOXY_BUILD_PYTHON_EXECUTABLE}"
        VENV_PATH "${EVIL_DOXY_BUILD_VENV_PATH}"
        CALLER "run_doxygen_build_py"
    )

    set(_build_script "${CMAKE_CURRENT_LIST_DIR}/doxygen/build.py")
    if(NOT EXISTS "${_build_script}")
        message(FATAL_ERROR
            "run_doxygen_build_py(): build.py not found at: ${_build_script}")
    endif()

    set(_build_working_directory "${EVIL_DOXY_BUILD_WORKING_DIRECTORY}")
    if(NOT _build_working_directory)
        get_filename_component(_build_working_directory "${_build_script}" DIRECTORY)
    endif()

    set(_doxyfile_path "${EVIL_DOXY_BUILD_DOXYFILE}")
    if(NOT _doxyfile_path)
        set(_default_doxyfile "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile")
        if(EXISTS "${_default_doxyfile}")
            set(_doxyfile_path "${_default_doxyfile}")
        endif()
    endif()

    set(_build_py_command
        "${_python}"
        "${_build_script}"
    )

    if(_doxyfile_path)
        list(APPEND _build_py_command --doxyfile "${_doxyfile_path}")
    endif()

    if(EVIL_DOXY_BUILD_EXTRA_ARGS)
        list(APPEND _build_py_command ${EVIL_DOXY_BUILD_EXTRA_ARGS})
    endif()

    if(NOT EVIL_DOXY_BUILD_QUIET)
        message(STATUS "Running doxygen build script with ${_python} (${_python_source})")
        message(STATUS "build.py path: ${_build_script}")
        if(_doxyfile_path)
            message(STATUS "build.py Doxyfile path: ${_doxyfile_path}")
        else()
            message(STATUS "build.py Doxyfile path: <not specified, build.py will use default lookup>")
        endif()
    endif()

    execute_process(
        COMMAND ${_build_py_command}
        WORKING_DIRECTORY "${_build_working_directory}"
        RESULT_VARIABLE _build_py_result
        OUTPUT_VARIABLE _build_py_stdout
        ERROR_VARIABLE _build_py_stderr
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_STRIP_TRAILING_WHITESPACE
    )

    if(_build_py_stdout AND NOT EVIL_DOXY_BUILD_QUIET)
        message(STATUS "build.py stdout:\n${_build_py_stdout}")
    endif()
    if(_build_py_stderr AND NOT EVIL_DOXY_BUILD_QUIET)
        message(STATUS "build.py stderr:\n${_build_py_stderr}")
    endif()

    if(NOT _build_py_result EQUAL 0)
        set(_build_py_error
            "run_doxygen_build_py(): build.py failed with exit code ${_build_py_result}")
        if(EVIL_DOXY_BUILD_FAIL_ON_ERROR)
            message(FATAL_ERROR
                "${_build_py_error}\n"
                "stdout:\n${_build_py_stdout}\n"
                "stderr:\n${_build_py_stderr}")
        else()
            message(WARNING "${_build_py_error}")
        endif()
    endif()
endfunction()

##\brief Runs the Python \c hpp2plantuml package to generate PlantUML output.
##
##\details
##Executes \c hpp2plantuml using the resolved Python interpreter after ensuring
##the package is installed.
##
##\param[in] INPUT_GLOB Glob pattern forwarded to \c hpp2plantuml.
##
##\param[in] OUTPUT_FILE Output location hint for generated PlantUML files.
##If this value ends with \c .puml, its directory is used as the destination
##folder and its basename is used as a filename prefix. Otherwise, the value is
##treated as a destination directory.
##
##\param[in] WORKING_DIRECTORY Optional working directory used for invocation.
##
##\param[in] PYTHON_EXECUTABLE Optional Python interpreter override.
##
##\param[in] VENV_PATH Optional virtual environment root.
##
##\param[in] EXTRA_ARGS Additional arguments forwarded directly to
##\c hpp2plantuml.
##
##\param[in] FAIL_ON_ERROR Treat runtime failure as a configure-time fatal error.
##
##\param[in] QUIET Suppress status logging.
##
##\par Example
##\code{.cmake}
##run_hpp2plantuml_for_doxygen(
##    INPUT_GLOB "${CMAKE_SOURCE_DIR}/source/modules/**/*.hpp"
##    OUTPUT_FILE "${CMAKE_BINARY_DIR}/docs/generated/modules"
##    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
##    VENV_PATH "${CMAKE_SOURCE_DIR}/.venv"
##)
##\endcode
function(run_hpp2plantuml_for_doxygen)
    set(options FAIL_ON_ERROR QUIET SUPPRESS_PARSE_WARNINGS ALLOW_EMPTY_INPUT)
    set(oneValueArgs INPUT_GLOB OUTPUT_FILE WORKING_DIRECTORY PYTHON_EXECUTABLE VENV_PATH)
    set(multiValueArgs EXTRA_ARGS)
    cmake_parse_arguments(EVIL_DOXY_RUN "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT EVIL_DOXY_RUN_INPUT_GLOB)
        message(FATAL_ERROR "run_hpp2plantuml_for_doxygen(): INPUT_GLOB is required")
    endif()

    if(NOT EVIL_DOXY_RUN_OUTPUT_FILE)
        message(FATAL_ERROR "run_hpp2plantuml_for_doxygen(): OUTPUT_FILE is required")
    endif()

    set(_working_directory "${EVIL_DOXY_RUN_WORKING_DIRECTORY}")
    if(NOT _working_directory)
        set(_working_directory "${CMAKE_SOURCE_DIR}")
    endif()

    _evil_doxygen_resolve_python_interpreter(
        _python
        _python_source
        PYTHON_EXECUTABLE "${EVIL_DOXY_RUN_PYTHON_EXECUTABLE}"
        VENV_PATH "${EVIL_DOXY_RUN_VENV_PATH}"
        CALLER "run_hpp2plantuml_for_doxygen"
    )

    _evil_doxygen_ensure_hpp2plantuml_installed(
        PYTHON_EXECUTABLE "${_python}"
        VENV_PATH "${EVIL_DOXY_RUN_VENV_PATH}"
    )

    # Expand the glob using CMake so Python's glob limitation with ** is bypassed.
    file(GLOB_RECURSE _input_files LIST_DIRECTORIES false "${EVIL_DOXY_RUN_INPUT_GLOB}")
    if(NOT _input_files)
        if(NOT EVIL_DOXY_RUN_QUIET AND NOT EVIL_DOXY_RUN_ALLOW_EMPTY_INPUT)
            message(WARNING "run_hpp2plantuml_for_doxygen(): no files matched INPUT_GLOB: ${EVIL_DOXY_RUN_INPUT_GLOB}")
        endif()
        return()
    endif()

    # Resolve output directory and optional filename prefix.
    set(_output_file_hint "${EVIL_DOXY_RUN_OUTPUT_FILE}")
    get_filename_component(_output_file_ext "${_output_file_hint}" EXT)
    if(_output_file_ext STREQUAL ".puml")
        get_filename_component(_output_directory "${_output_file_hint}" DIRECTORY)
        get_filename_component(_output_prefix "${_output_file_hint}" NAME_WE)
    else()
        set(_output_directory "${_output_file_hint}")
        set(_output_prefix "")
    endif()

    if(NOT _output_directory)
        set(_output_directory "${CMAKE_CURRENT_BINARY_DIR}")
    endif()

    file(MAKE_DIRECTORY "${_output_directory}")

    if(NOT EVIL_DOXY_RUN_QUIET)
        list(LENGTH _input_files _input_count)
        message(STATUS "Running hpp2plantuml with ${_python} (${_python_source}) for ${_input_count} input file(s)")
    endif()

    set(_generated_count 0)
    increment_log_indent()
    foreach(_f IN LISTS _input_files)
        file(RELATIVE_PATH _relative_input_file "${_working_directory}" "${_f}")
        if(_relative_input_file MATCHES "^\\.\\.")
            # Try source-root-relative path first
            file(RELATIVE_PATH _relative_input_file "${CMAKE_SOURCE_DIR}" "${_f}")
            if(_relative_input_file MATCHES "^\\.\\.")
                # File is outside source tree, use hash-based unique identifier
                string(MD5 _path_hash "${_f}")
                string(SUBSTRING "${_path_hash}" 0 10 _path_hash)
                get_filename_component(_relative_input_file "${_f}" NAME)
                set(_relative_input_file "${_relative_input_file}.${_path_hash}")
            endif()
        endif()

        string(REPLACE "\\" "/" _relative_input_file "${_relative_input_file}")
        string(REGEX REPLACE "\\.[^.]+$" "" _relative_stem "${_relative_input_file}")
        string(REPLACE "/" "_" _relative_stem "${_relative_stem}")

        if(_output_prefix)
            set(_output_file "${_output_directory}/${_output_prefix}_${_relative_stem}.puml")
        else()
            set(_output_file "${_output_directory}/${_relative_stem}.puml")
        endif()

        execute_process(
            COMMAND ${CMAKE_COMMAND} -E env PYTHONUTF8=1 PYTHONIOENCODING=UTF-8 "${_python}" -m hpp2plantuml.hpp2plantuml -i "${_f}" -o "${_output_file}" ${EVIL_DOXY_RUN_EXTRA_ARGS}
            WORKING_DIRECTORY "${_working_directory}"
            RESULT_VARIABLE _hpp2plantuml_result
            OUTPUT_VARIABLE _hpp2plantuml_stdout
            ERROR_VARIABLE _hpp2plantuml_stderr
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_STRIP_TRAILING_WHITESPACE
        )

        if(_hpp2plantuml_stdout AND NOT EVIL_DOXY_RUN_QUIET)
            message(STATUS "hpp2plantuml stdout (${_f}):\n${_hpp2plantuml_stdout}")
        endif()
        if(_hpp2plantuml_stderr AND NOT EVIL_DOXY_RUN_QUIET)
            message(STATUS "hpp2plantuml stderr (${_f}):\n${_hpp2plantuml_stderr}")
        endif()

        if(NOT _hpp2plantuml_result EQUAL 0)
            set(_hpp2plantuml_error "hpp2plantuml execution failed for '${_f}' (exit ${_hpp2plantuml_result}) using interpreter: ${_python}")
            if(EVIL_DOXY_RUN_FAIL_ON_ERROR)
                message(FATAL_ERROR "${_hpp2plantuml_error}")
            elseif(EVIL_DOXY_RUN_QUIET OR EVIL_DOXY_RUN_SUPPRESS_PARSE_WARNINGS)
                continue()
            else()
                message(WARNING "${_hpp2plantuml_error}")
                continue()
            endif()
        endif()

        math(EXPR _generated_count "${_generated_count} + 1")
        if(NOT EVIL_DOXY_RUN_QUIET)
            message(STATUS "Generated PlantUML: ${_output_file}")
        endif()
    endforeach()
    decrement_log_indent()

    if(NOT EVIL_DOXY_RUN_QUIET)
        message(STATUS "Generated ${_generated_count} PlantUML file(s) into: ${_output_directory}")
    endif()
endfunction()

##\brief Generates PlantUML for both .h and .hpp headers under source/.
##
##\details
##Provides a single, shared entry point used by both configure-time docs
##generation and the PlantUML-only script to avoid duplicate call blocks.
##
##\param[in] OUTPUT_FILE_H Required output hint for .h generation.
##\param[in] OUTPUT_FILE_HPP Required output hint for .hpp generation.
##\param[in] WORKING_DIRECTORY Optional working directory for tool execution.
##\param[in] PYTHON_EXECUTABLE Optional interpreter override.
##\param[in] VENV_PATH Optional virtual environment root.
##\param[in] QUIET Suppress verbose tool output.
##\param[in] SUPPRESS_PARSE_WARNINGS Skip parser warnings/failures non-fatally.
##
function(run_hpp2plantuml_header_globs_for_doxygen)
    set(options QUIET SUPPRESS_PARSE_WARNINGS)
    set(oneValueArgs OUTPUT_FILE_H OUTPUT_FILE_HPP WORKING_DIRECTORY PYTHON_EXECUTABLE VENV_PATH)
    cmake_parse_arguments(EVIL_DOXY_HDR "${options}" "${oneValueArgs}" "" ${ARGN})

    if(NOT EVIL_DOXY_HDR_OUTPUT_FILE_H)
        message(FATAL_ERROR "run_hpp2plantuml_header_globs_for_doxygen(): OUTPUT_FILE_H is required")
    endif()

    if(NOT EVIL_DOXY_HDR_OUTPUT_FILE_HPP)
        message(FATAL_ERROR "run_hpp2plantuml_header_globs_for_doxygen(): OUTPUT_FILE_HPP is required")
    endif()

    set(_common_args
        WORKING_DIRECTORY "${EVIL_DOXY_HDR_WORKING_DIRECTORY}"
        PYTHON_EXECUTABLE "${EVIL_DOXY_HDR_PYTHON_EXECUTABLE}"
        VENV_PATH "${EVIL_DOXY_HDR_VENV_PATH}"
    )

    if(EVIL_DOXY_HDR_QUIET)
        list(APPEND _common_args QUIET)
    endif()

    if(EVIL_DOXY_HDR_SUPPRESS_PARSE_WARNINGS)
        list(APPEND _common_args SUPPRESS_PARSE_WARNINGS)
    endif()

    run_hpp2plantuml_for_doxygen(
        INPUT_GLOB "${CMAKE_SOURCE_DIR}/source/**/*.h"
        OUTPUT_FILE "${EVIL_DOXY_HDR_OUTPUT_FILE_H}"
        ${_common_args}
    )

    run_hpp2plantuml_for_doxygen(
        INPUT_GLOB "${CMAKE_SOURCE_DIR}/source/**/*.hpp"
        OUTPUT_FILE "${EVIL_DOXY_HDR_OUTPUT_FILE_HPP}"
        ${_common_args}
        ALLOW_EMPTY_INPUT
    )
endfunction()

##\brief Resolves the PlantUML directory under external.
##
##\details Supports both \c external/plantUML (current) and \c external/plantuml
##directory naming, so documentation generation remains robust across case-sensitive
##and case-insensitive file systems.
##
##\param[out] plantuml_dir Variable name to store the resolved PlantUML directory.
##This variable is set in the parent scope.
##
function(_resolve_plantuml_dir plantuml_dir)
    set(_plantuml_candidates
        "${CMAKE_SOURCE_DIR}/docs/tools/plantUML"
        "${CMAKE_SOURCE_DIR}/docs/tools/plantuml"
    )

    foreach(_candidate IN LISTS _plantuml_candidates)
        if(EXISTS "${_candidate}")
            set(${plantuml_dir} "${_candidate}" PARENT_SCOPE)
            return()
        endif()
    endforeach()

    message(FATAL_ERROR
        "PlantUML directory not found. Checked: ${_plantuml_candidates}")
endfunction()

##\brief Finds the newest PlantUML JAR file in the external directory.
##
##\details Searches the \c external/plantUML directory for all JAR files,
##determines which one is the most recently modified based on file timestamp,
##and returns the path to that file. If no JAR files are found, an error is
##issued and the CMake configuration fails.
##
##\param[out] plantuml_jar_file Variable name to store the path to the newest PlantUML JAR file.
##This variable is set in the parent scope.
##
##\exception FATAL_ERROR Thrown if no PlantUML JAR files are found in the external/plantUML directory.
##
##\note This is an internal function (prefixed with underscore) and should not be called directly
##from external CMake code.
##
function(_find_newest_plantuml_jar_file plantuml_jar_file)
    _resolve_plantuml_dir(JAR_DIR)

    ##\details
    ##Removes an existing \c plantuml.jar alias before scanning so the newest
    ##versioned JAR remains the only candidate selected by timestamp.
    set(_existing_jar_file "${JAR_DIR}/plantuml.jar")
    if(EXISTS "${_existing_jar_file}")
        file(REMOVE "${_existing_jar_file}")
    endif()
    
    file(GLOB JAR_FILES "${JAR_DIR}/*.jar")
    set(NEWEST_JAR "")
    set(NEWEST_TIME 0)
    foreach(JAR_FILE ${JAR_FILES})
        file(TIMESTAMP "${JAR_FILE}" JAR_TIME "%s" UTC)
        if(JAR_TIME GREATER NEWEST_TIME)
            set(NEWEST_TIME ${JAR_TIME})
            set(NEWEST_JAR ${JAR_FILE})
        endif()
    endforeach()

    if(NOT EXISTS "${NEWEST_JAR}")
        message(FATAL_ERROR "No PlantUML jar found in ${JAR_DIR}")
    else()
        set(${plantuml_jar_file} "${NEWEST_JAR}" PARENT_SCOPE)
    endif()
endfunction()

##@}
