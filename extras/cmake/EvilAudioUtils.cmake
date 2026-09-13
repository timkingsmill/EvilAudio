include_guard(GLOBAL)


# =================================================================================
# Utility functions for creating Evil Audio application targets.
include(${CMAKE_SOURCE_DIR}/extras/cmake/EvilAudioApplicationUtils.cmake)

# =================================================================================
# Utility functions for linking Evil Audio static libraries to an application.
# DEPRICATED: This is no longer needed since we can link JUCE modules directly to the application target.
###include(cmake/EvilAudioStaticLibUtils.cmake)

# =================================================================================
# =================================================================================
#
# Links one or more existing STATIC_LIBRARY targets to the given target.
# Verifies each argument is a defined target of type STATIC_LIBRARY, links it
# privately, and emits status messages; fails with a fatal error if a target
# is missing or not static.
#
# Arguments:
#   target - The target to link the static libraries to.
#
function(target_link_static_libraries target) 
    message(STATUS "Linking static libraries to target: ${target}")
    increment_log_indent()

    list(LENGTH ARGN num_extra_args)
    if(num_extra_args EQUAL 0)
        return()
    endif()

    foreach(static_lib ${ARGN})
        if(TARGET ${static_lib})
            get_target_property(target_type ${static_lib} TYPE)
            if(target_type STREQUAL "STATIC_LIBRARY")
                message(STATUS "Linking static library: ${static_lib} to target: ${target}")
                target_link_libraries(${target} PRIVATE ${static_lib})
            else()
                message(FATAL_ERROR "Target is not a static library: ${static_lib} (type: ${target_type})")
            endif()
        else()
            message(FATAL_ERROR "Static library target not found: ${static_lib}")
        endif()
    endforeach()

    decrement_log_indent()
    message(STATUS "Finished linking static libraries to target: ${target}")
endfunction()

# =================================================================================
# Function to print properties of a target (for debugging purposes)
#
function(print_target_properties target)
    if(NOT TARGET ${target})
        message(FATAL_ERROR "Fatal Error in function: ${CMAKE_CURRENT_FUNCTION}(): Target not found: ${target}")
    endif()

    message(STATUS "Properties for target: ${target}")
    increment_log_indent()
    
    get_target_property(target_type ${target} TYPE)
    message(STATUS "Target Type: ${target_type}")
    
    get_target_property(target_sources ${target} SOURCES)
    _print_list_properties(SOURCES "Sources" ${target_sources})
    
    get_target_property(target_include_dirs ${target} INCLUDE_DIRECTORIES)
    _print_list_properties(INCLUDE_DIRECTORIES "Include Directories" ${target_include_dirs})

    get_target_property(target_interface_include_dirs ${target} INTERFACE_INCLUDE_DIRECTORIES)
    _print_list_properties(INTERFACE_INCLUDE_DIRECTORIES "Interface Include Directories" ${target_interface_include_dirs})

    get_target_property(target_compile_features ${target} INTERFACE_COMPILE_FEATURES)
    _print_list_properties(INTERFACE_COMPILE_FEATURES "Interface Compile Features" ${target_compile_features})

    get_target_property(target_link_libraries ${target} LINK_LIBRARIES)
    _print_list_properties(LINK_LIBRARIES "Link Libraries" ${target_link_libraries})

    get_target_property(target_interface_link_libraries ${target} INTERFACE_LINK_LIBRARIES)
    _print_list_properties(INTERFACE_LINK_LIBRARIES "Interface Link Libraries" ${target_interface_link_libraries})

    get_target_property(target_interface_link_libraries_direct ${target} INTERFACE_LINK_LIBRARIES_DIRECT)
    _print_list_properties(INTERFACE_LINK_LIBRARIES_DIRECT "Interface Link Libraries (Direct)" ${target_interface_link_libraries_direct})

    decrement_log_indent()
endfunction()

# =================================================================================

function(_print_list_properties list_name list_description list_values)
    message(STATUS "${list_name} ${list_description}:")
    increment_log_indent()
        foreach(value ${list_values})
            message(STATUS ${value})
        endforeach()
    decrement_log_indent()
endfunction()

# =================================================================================
# =================================================================================

# =================================================================================================
# Sets MSVC multiprocessor compile options for Visual Studio builds
#
# This function checks if multiprocessor compilation is enabled and sets the appropriate
# /MP or /MP<n> flag for MSVC. It validates the user-supplied processor cap and provides
# status messages for clarity. Should be called from the main CMakeLists.txt.
#
# Usage: evil_audio_set_msvc_multiprocessor_options()
#
# Variables:
#   EVILAUDIO_MSVC_ENABLE_MULTIPROCESSOR (option) - Enable/disable multiprocessor build
#   EVILAUDIO_MSVC_MAX_PARALLEL_COMPILE (cache string) - Optional processor cap for /MP<n>
# =================================================================================================

function(evil_audio_set_msvc_multiprocessor_options)
    set(EVILAUDIO_MSVC_MAX_PARALLEL_COMPILE 2 CACHE STRING
        "Optional processor cap for MSVC /MP<n>. Leave empty to let MSVC choose automatically")

    if(MSVC AND EVILAUDIO_MSVC_ENABLE_MULTIPROCESSOR)
        if(EVILAUDIO_MSVC_MAX_PARALLEL_COMPILE)
            if(NOT EVILAUDIO_MSVC_MAX_PARALLEL_COMPILE MATCHES "^[0-9]+$")
                message(FATAL_ERROR
                    "EVILAUDIO_MSVC_MAX_PARALLEL_COMPILE must be a non-negative integer (for example: 8)")
            endif()
            add_compile_options("/MP${EVILAUDIO_MSVC_MAX_PARALLEL_COMPILE}")
            message(STATUS "Enabled MSVC multiprocessor compilation: /MP${EVILAUDIO_MSVC_MAX_PARALLEL_COMPILE}")
        else()
            add_compile_options("/MP")
            message(STATUS "Enabled MSVC multiprocessor compilation: /MP (automatic processor count)")
        endif()
    endif()
endfunction()

# =================================================================================================
