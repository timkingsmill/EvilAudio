#[[
================================================================================
EvilAudioStaticLibUtils.cmake

DESCRIPTION:
    CMake utility module for building JUCE audio framework modules as static 
    libraries. Provides functions to create static library targets from JUCE 
    modules with proper configuration, metadata extraction, and platform-specific 
    handling.

CONTENTS:
    - add_juce_module_static_library()
        Main function to create a static library target for a JUCE module
    
    - Helper functions:
        _add_static_library()              - Creates static library target
        _extract_metadata_block()          - Parses metadata from module headers
        _evil_link_libs_from_metadata()    - Links platform libraries from metadata
        _add_standard_defs()               - Applies standard JUCE definitions
        _strip_alias_prefix()              - Removes namespace prefixes
        _get_juce_module_path()            - Resolves the path to the to the modules directory target
        _evil_get_metadata()               - Fetches metadata properties
        get_juce_cmake_utils_dir()         - Locates CMake utils directory

DEPENDENCIES:
    - CMake 4.2.0 or higher
    - JUCE framework CMake integration
    - JUCE module targets must be pre-defined

FEATURES:
    - Automatic metadata extraction from JUCE module declarations
    - Platform-specific compilation and linking (Windows, BSD, Android)
    - C++ standard detection and enforcement from module metadata
    - Module dependency resolution and linking
    - Include path management
    - Special handling for juce_core module
    - Windows MSVC /bigobj flag for large modules

USAGE:
    include(EvilAudioStaticLibUtils.cmake)
    add_juce_module_static_library(juce_core)
    add_juce_module_static_library(juce_audio_processors)

================================================================================
]]#
include_guard(GLOBAL)

#===============================================================================================

cmake_minimum_required(VERSION 4.2.0)

#===============================================================================================
#[[
Function: add_juce_module_static_library
Description: Creates a static library target for a JUCE module with proper configuration,
             include paths, and compile definitions.

Arguments:
    source_module_name (required)
        The name of the JUCE module to create a static library for.
        Can include alias prefix which will be stripped before processing.

Behavior:
    - Validates that the module target exists
    - Locates the module source directory
    - Searches for module header file (.h or .hpp extension)
    - Collects all module source and header files
    - Creates a static library target named "<module_name>_lib"
    - Sets INTERFACE properties for module sources, headers, and path
    - Applies special configuration for juce_core module (system-specific linking)
    - Configures recommended JUCE compiler flags and warning levels
    - Sets compile definitions (JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED, JUCE_WEB_BROWSER, etc.)
    - Exports include directories as PUBLIC

Output Properties:
    - INTERFACE_JUCE_MODULE_SOURCES: List of module source files
    - INTERFACE_JUCE_MODULE_HEADERS: List of module header files
    - INTERFACE_JUCE_MODULE_PATH: Path to module directory

Dependencies:
    Helper functions:
    - _strip_alias_prefix()
    - _get_juce_module_path()
    - _add_static_library()
    - _add_standard_defs()

Special Cases:
    - juce_core: Adds atomic wrapper, execinfo on BSD, Android NDK sources on Android
]]
function (add_juce_module_static_library juce_module)  

    message(STATUS  "-------------------------------------------------------------------------")
    message (STATUS "Adding JUCE Module Static Library for module: ${juce_module}")
    increment_log_indent()
    # Check that the module target exists. If it doesn't, throw an error.
    if(NOT TARGET ${juce_module})
        message(FATAL_ERROR "Module ${juce_module} is not a valid or existing cmake target.")
    endif()
    # Strip any alias prefix from the module name.
    _strip_alias_prefix(${juce_module} temp)
    set(juce_module ${temp})

    # Get the path to the JUCE module parent directory
    # This is where the module source files are located.
    # e.g. C:\source\EvilAudio\libs\JUCE\install\include\JUCE-8.0.10\modules\
    _get_juce_module_path(${juce_module} module_parent_path)
    
    # Fail if the path to the module source directory does not exist
    if(NOT EXISTS "${module_parent_path}")
        message(FATAL_ERROR "${juce_module} source directory does not exist: ${module_parent_path}")
    endif()
    message(STATUS "Found parent directory for JUCE [${juce_module}] module : ${module_parent_path}")
    # Check that the module header file exists
    set(module_header_name "${juce_module}.h")
    
    # First check for .h extension, then .hpp
    if(NOT EXISTS "${module_parent_path}/${juce_module}/${module_header_name}")
        set(module_header_name "${module_header_name}pp")
    endif()

    # Fail if the module header file does not exist
    if(NOT EXISTS "${module_parent_path}/${juce_module}/${module_header_name}")
        message(FATAL_ERROR "${juce_module} Module header does not exist: ${module_parent_path}/${module_header_name}")
    endif()

    # Found the module header
    message(STATUS "Found module header: ${module_header_name}")

    # Copy the source module souces and headers from the JUCE module
    # to this static library target.
    get_target_property(module_sources ${juce_module} INTERFACE_JUCE_MODULE_SOURCES)
    foreach(filename ${module_sources})
        #message(STATUS "\t\tModule source: ${filename}")
    endforeach()
    get_target_property(module_headers ${juce_module} INTERFACE_JUCE_MODULE_HEADERS)
    foreach(filename ${module_headers})
        #message(STATUS "\t\tModule header: ${filename}")
    endforeach()

    # Create the static library target for the JUCE module.
    set(lib_target "${juce_module}_lib")
    _add_static_library(${lib_target} "${module_sources}")
    add_library(evil::${lib_target} ALIAS ${lib_target})
    message(STATUS "Created static library target: ${lib_target}")

    set_target_properties(${lib_target} PROPERTIES
        INTERFACE_JUCE_MODULE_SOURCES   "${module_sources}"
        INTERFACE_JUCE_MODULE_HEADERS   "${module_headers}"
        INTERFACE_JUCE_MODULE_PATH      "${module_parent_path}"
    )

    #if(JUCE_ENABLE_MODULE_SOURCE_GROUPS)
    #    message(STATUS "\tAdding module headers to: ${lib_target}")
    #    target_sources(${lib_target} INTERFACE ${module_headers})
    #endif()

    # Config special case juce_core  
    if(${juce_module} STREQUAL "juce_core")
        message(STATUS "SPECIAL CASE")

        _add_standard_defs(${lib_target})

        target_link_libraries(${lib_target} INTERFACE juce::juce_atomic_wrapper)

        if(CMAKE_SYSTEM_NAME MATCHES ".*BSD")
            target_link_libraries(${lib_target} INTERFACE execinfo)
        elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")
            target_sources(${lib_target} INTERFACE "${ANDROID_NDK}/sources/android/cpufeatures/cpu-features.c")
            target_include_directories(${lib_target} INTERFACE "${ANDROID_NDK}/sources/android/cpufeatures")
            target_link_libraries(${lib_target} INTERFACE android log)
        endif()
    endif()

    # Apply the juce module include directories to the static library target.
    message(STATUS "Looking for include directories for module: ${juce_module}")
    get_target_property(module_includes ${juce_module} INTERFACE_INCLUDE_DIRECTORIES)
    if(module_includes)
        message(STATUS "Found include directories for module: ${juce_module}")
        
        increment_log_indent()
        foreach(include_dir IN LISTS module_includes)
            message(STATUS "Include dir: ${include_dir}")
        endforeach()

        message(STATUS "Appending include directory to ${lib_target}")
        target_include_directories(${lib_target} 
            PUBLIC 
                ${module_includes}
        )
        decrement_log_indent()
    endif()

    # We also need to export the include directories for the modules
    # Must be public to add the include path to both this library
    # and its clients.
    #target_include_directories(${lib_target} 
    #    PUBLIC 
    #        #${module_parent_path}
    #)

    # Set compiler definitions for the static library target.
    # This must be done to ensure that the "JuceHeader.h" file
    # is correctly configured for each module.
    # The module and the static library target share the same definitions.
    target_compile_definitions(${lib_target} INTERFACE JUCE_MODULE_AVAILABLE_${juce_module}=1)  
    # Handle module metadata ---------------------------------------

    # Extract module metadata from the module header file
    _extract_metadata_block(JUCE_MODULE_DECLARATION "${module_parent_path}/${juce_module}/${module_header_name}" metadata_dict)
    if (TARGET ${metadata_dict})
        message(STATUS "\tExtracted metadata for module: ${juce_module} as target: ${metadata_dict}")
    endif()
    # Populate the metadata dictionary
    _evil_get_metadata("${metadata_dict}" minimumCppStandard module_cpp_standard)
    # Set the C++ standard if specified in the module metadata
    if(module_cpp_standard)
        message(STATUS "\tSetting C++ standard to ${module_cpp_standard} for ${lib_target}")
        target_compile_features(${lib_target} PUBLIC cxx_std_${module_cpp_standard})   
    else()
        target_compile_features(${lib_target} INTERFACE cxx_std_11)
    endif()

    #Handle platform-specific compile options and linking from module metadata
    #if(FALSE)
    #else
    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        if((CMAKE_CXX_COMPILER_ID STREQUAL "MSVC") OR (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC"))
            if(juce_module MATCHES "juce_gui_basics|juce_audio_processors|juce_core|juce_graphics")
                message(STATUS "\tAdding /bigobj compile option to ${lib_target} to avoid C1128 errors")
                target_compile_options(${lib_target} PUBLIC /bigobj)
            endif()

            # Link Windows-specific libraries from metadata extracted from the JUCE module header.
            #_add_evil_link_libs_from_metadata("${lib_target}" "${metadata_dict}" windowsLibs)
            _evil_link_libs_from_metadata("${juce_module}" "${metadata_dict}" windowsLibs)
        endif()
    endif()

    # Handle module dependencies extracted from metadata.
    #
    _evil_get_metadata("${metadata_dict}" dependencies module_dependencies)
    foreach(module IN LISTS module_dependencies)
        # Get the static library target name for the dependency.
        set(dependency_target "${module}_lib")
        message(STATUS "\tLinking dependency target: ${dependency_target} to ${lib_target}")
        target_link_libraries(${lib_target} PRIVATE ${dependency_target})
    endforeach()

    _evil_get_metadata("${metadata_dict}" searchpaths module_searchpaths)
    if(NOT module_searchpaths STREQUAL "")
        foreach(module_searchpath IN LISTS module_searchpaths)
            target_include_directories(
                ${lib_target}
                INTERFACE "${module_parent_path}/${module_searchpath}")
        endforeach()
    endif()

    _juce_add_module_staticlib_paths("${lib_target}" "${module_parent_path}")

    # Set standard JUCE compile definitions for the static library target.
    # These are required for proper JUCE module functionality.
    # We're linking the modules privately, but we need to export
    # their compile flags as PUBLIC.
    target_compile_definitions(${lib_target}
        PUBLIC
            JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=0
            JUCE_WEB_BROWSER=0
            JUCE_USE_CURL=0
            JUCE_INCLUDE_ZLIB_CODE=1
        INTERFACE
            $<TARGET_PROPERTY:${lib_target},COMPILE_DEFINITIONS>
    )
    decrement_log_indent()
    message(STATUS "-------------------------------------------------------------------------")
endfunction()


#===============================================================================================
#
# _add_static_library
# 
# Private function to create a static library target with the given sources.
#
# ARGUMENTS:
#   delim_str       - The delimiter string used to identify metadata blocks
#                     (e.g., "JUCE_MODULE_DECLARATION")
#   file_with_block - Path to the file containing the metadata block
#   out_dict        - Variable name to store the resulting metadata dictionary
#                     (an IMPORTED INTERFACE library target)
#
# DESCRIPTION:
#   This is an internal helper function used to add a static library target
#   to the CMake project. It should not be called directly from outside this module.
#
function(_add_static_library target sources)
    add_library(${target} STATIC)
    increment_log_indent()
    message(STATUS "Adding sources to static library target: ${target}")
    increment_log_indent()
    
    foreach(filename ${sources})
        message(STATUS "Adding source file to ${target}: ${filename}")
    endforeach()
    decrement_log_indent()
    decrement_log_indent()
    target_sources(${target} PRIVATE ${sources})
endfunction()

#===============================================================================================

#[[
    Extracts metadata from a delimited block within a file and stores it as target properties.
    
    This function parses a file for a metadata block delimited by BEGIN_<delim_str> and 
    END_<delim_str> markers. It extracts key-value pairs from the block and stores them
    as INTERFACE properties on an internally-created IMPORTED INTERFACE library target.
    
    The metadata format expected is:
    - Keys are alphanumeric identifiers followed by a colon
    - Values are comma, space, or tab-separated and converted to semicolon-separated lists
    - Multiple lines with the same key append to the same property
    
    @param delim_str    The delimiter string used in BEGIN_<delim_str> and END_<delim_str> markers
    @param file_with_block    Path to the file containing the metadata block
    @param out_dict    Output variable that receives the name of the created target dictionary
    
    @details
    - Creates a random-named IMPORTED INTERFACE library to store metadata as target properties
    - Properties are named INTERFACE_JUCE_<KEY> where <KEY> is extracted from the metadata
    - Throws FATAL_ERROR if the specified file does not exist
    - Skips leading whitespace in all pattern matching
    
    @example
    _extract_metadata_block("METADATA" "myfile.h" dict_var)
    # dict_var now contains target name with properties like INTERFACE_JUCE_VERSION, etc.
#]]
function(_extract_metadata_block delim_str file_with_block out_dict)
    string(RANDOM LENGTH 16 random_string)
    set(target_name "${random_string}_dict")
    set(${out_dict} "${target_name}" PARENT_SCOPE)
    add_library(${target_name} INTERFACE IMPORTED)

    if(NOT EXISTS ${file_with_block})
        message(FATAL_ERROR "Unable to find file ${file_with_block}")
    else()
        message(STATUS "Extracting metadata block ${delim_str} from file: ${file_with_block}")
    endif()

    # Read the file contents into a list of lines
    file(STRINGS ${file_with_block} module_header_contents)

    set(last_written_key)
    set(append NO)

    increment_log_indent()
    message(STATUS "Parsing metadata block: ${delim_str}")
    foreach(line IN LISTS module_header_contents)
        if(NOT append)
            if(line MATCHES "[\t ]*BEGIN_${delim_str}[\t ]*")
                set(append YES)
            endif()

            continue()
        endif()

        if(append AND (line MATCHES "[\t ]*END_${delim_str}[\t ]*"))
            break()
        endif()

        if(line MATCHES "^[\t ]*([a-zA-Z]+):")
            set(last_written_key "${CMAKE_MATCH_1}")
        endif()

        string(REGEX REPLACE "^[\t ]*${last_written_key}:[\t ]*" "" line "${line}")
        string(REGEX REPLACE "[\t ,]+" ";" line "${line}")
        
        # Skip empty lines
        if (line STREQUAL "")
            continue()
        endif()

        set(property_key "INTERFACE_JUCE_${last_written_key}")
        
        increment_log_indent()
        message(STATUS "Extracted metadata: ${property_key} = ${line}")
        decrement_log_indent()
        
        set_property(
            TARGET ${target_name} 
            APPEND PROPERTY
                "${property_key}" "${line}"
        )
    endforeach()
    decrement_log_indent()
endfunction()

#===============================================================================================

function(_evil_link_libs_from_metadata module_name dict key)
    _evil_get_metadata("${dict}" "${key}" libs)
    if(libs)
        foreach(lib IN LISTS libs)
            message(STATUS "\tLinking ${lib} to ${module_name}_lib from metadata")
            target_link_libraries("${module_name}_lib" INTERFACE "${lib}")
        endforeach()
    endif()
endfunction()

#===============================================================================================

function(_add_standard_defs lib_target)
    _juce_get_debug_config_genex(debug_config)

    target_compile_definitions(${lib_target} INTERFACE
        JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1
        $<IF:${debug_config},DEBUG=1 _DEBUG=1,NDEBUG=1 _NDEBUG=1>
        $<$<PLATFORM_ID:Android>:JUCE_ANDROID=1>)
endfunction()

#===============================================================================================
# Function to strip alias prefix from module name
# Usage: _strip_alias_prefix(<module_name> <return_var>)
function (_strip_alias_prefix module_name return_var)
    string(REGEX REPLACE "^[a-zA-Z0-9]+::" "" stripped_name "${module_name}")
    set(${return_var} "${stripped_name}" PARENT_SCOPE)
endfunction()

#===============================================================================================
# Function to get the path directory for a given module
# Usage: _get_juce_module_path(<module_name> <return_var>)
# Example: _get_juce_module_path(JUCE::juce_core JUCE_CORE_PATH)
function(_get_juce_module_path module_name return_var)
    get_target_property(module_path ${module_name} INTERFACE_JUCE_MODULE_PATH)
    if(NOT module_path STREQUAL "module_path-NOTFOUND")
        set(${return_var} "${module_path}" PARENT_SCOPE)
    else()
        message(FATAL_ERROR "Module ${module_name} does not have a valid INTERFACE_JUCE_MODULE_PATH property.")
    endif()
endfunction()

#===============================================================================================
# Fetches properties attached to a metadata target.
function(_evil_get_metadata target key out_var)
    message(STATUS "\tFetching metadata for key [${key}] from target [${target}]")

    # ToDo: Fix this function 
    # Do I have the correct target?

    get_target_property(content "${target}" "INTERFACE_JUCE_${key}")

    if("${content}" STREQUAL "content-NOTFOUND")
        set(${out_var} PARENT_SCOPE)
        message(STATUS "\t\tMetadata not found.")
    else()
        set(${out_var} "${content}" PARENT_SCOPE)
    endif()
endfunction()

#===============================================================================================

# Function to get JUCE_CMAKE_UTILS_DIR
# If JUCE_CMAKE_UTILS_DIR is not defined, try to get it from environment variable
# If still not defined, throw an error
function (get_juce_cmake_utils_dir return_var)
    if(DEFINED JUCE_CMAKE_UTILS_DIR)
        set(${return_var} "${JUCE_CMAKE_UTILS_DIR}" PARENT_SCOPE)
    else()
        if(DEFINED ENV{JUCE_CMAKE_UTILS_DIR})
            set(JUCE_CMAKE_UTILS_DIR "$ENV{JUCE_CMAKE_UTILS_DIR}")
            set(${return_var} "${JUCE_CMAKE_UTILS_DIR}" PARENT_SCOPE)
        else()
            message(FATAL_ERROR "JUCE_CMAKE_UTILS_DIR is not defined. Please set this variable to point to the JUCE CMake Utils directory.")
        endif()
    endif()
endfunction()

# ================================================================================================    