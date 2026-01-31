include_guard(GLOBAL)

# =================================================================================
# Utility functions for creating Evil Audio application targets.
include(cmake/EvilAudioApplicationUtils.cmake) 

# =================================================================================
# Utility functions for linking Evil Audio static libraries to an application.
include(cmake/EvilAudioStaticLibUtils.cmake)

# =================================================================================
# Utility functions for logging with indentation.
#
set_property(GLOBAL PROPERTY GLOBAL_MESSAGE_INDENT "")

# Increments the global log indentation level used by the build system's logging
# helpers so nested operations can be visually aligned in output.
function(increment_log_indent)
    get_property(global_message_indent GLOBAL PROPERTY GLOBAL_MESSAGE_INDENT)
    list(APPEND global_message_indent "    ")
    set(CMAKE_MESSAGE_INDENT "${global_message_indent}" PARENT_SCOPE)
    set_property(GLOBAL PROPERTY GLOBAL_MESSAGE_INDENT "${global_message_indent}")
endfunction()

# Decreases the current logging indentation level.
# Intended to be paired with an increment to restore previous indent depth.
# Safe to call when no indentation is set (should clamp at zero).
function(decrement_log_indent)
    get_property(global_message_indent GLOBAL PROPERTY GLOBAL_MESSAGE_INDENT)
    list(POP_BACK global_message_indent)
    set(CMAKE_MESSAGE_INDENT "${global_message_indent}" PARENT_SCOPE)
    set_property(GLOBAL PROPERTY GLOBAL_MESSAGE_INDENT "${global_message_indent}")
endfunction()

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

