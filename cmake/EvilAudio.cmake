include_guard(GLOBAL)

# =================================================================================
# It is common for CMake projects to define a custom message function 
# that wraps the built-in message command. This allows for additional 
# formatting, logging, or other custom behavior when printing messages.
function(message level)


    # Forward all arguments to the built-in message command after processing
    _message(${ARGV0} ${ARGN})
endfunction()

# =================================================================================
# Utility functions for creating Evil Audio application targets.
include(cmake/EvilAudioApplicationUtils.cmake) 

# =================================================================================
# Utility functions for linking Evil Audio static libraries to an application.
include(cmake/EvilAudioStaticLibUtils.cmake)

# =================================================================================
# Formats given string with colors and writes the result in
# the COLOR_FORMATTED_TEXT variable, which can be used in the parent scope.
#
# Example:  colorFormatText(COLOR BLUE "My blue text")
#           colorFormatText(BOLD COLOR RED "My bold red text")
#
# To print: message("${COLOR_FORMATTED_TEXT}")
#
function(colorFormatText)
    cmake_parse_arguments(PARSE_ARGV 0 "_TEXT" "BOLD" "COLOR" "")
    set(_FORMAT_OPTIONS -E cmake_echo_color --no-newline)
    
    # Do we have a color attribute?
    if(_TEXT_COLOR)
        string(TOLOWER "${_TEXT_COLOR}" _TEXT_COLOR_LOWER)
        
        # Is it a valid color attribute?
        if(${_TEXT_COLOR_LOWER} MATCHES "^normal|black|red|green|yellow|blue|magenta|cyan|white")
            list(APPEND _FORMAT_OPTIONS --${_TEXT_COLOR_LOWER})
        endif()
    endif()
    
    # Do we have a BOLD attribute?
    if(_TEXT_BOLD)
        list(APPEND _FORMAT_OPTIONS --bold)
    endif()
    
    # Run CMake command to format text and write result to _FORMATTED_TEXT_RESULT
    execute_process(COMMAND ${CMAKE_COMMAND} -E env CLICOLOR_FORCE=1 ${CMAKE_COMMAND} ${_FORMAT_OPTIONS} ${_TEXT_UNPARSED_ARGUMENTS}
                    OUTPUT_VARIABLE _FORMATTED_TEXT_RESULT
                    ECHO_ERROR_VARIABLE)
    
    # Save result into COLOR_FORMATTED_TEXT for parent scope access
    set(COLOR_FORMATTED_TEXT ${_FORMATTED_TEXT_RESULT} PARENT_SCOPE)
endfunction()

# Formats given string with colors and appends the result to
# the COLOR_FORMATTED_TEXT_COMBINED variable, which can be used
# in the parent scope.
#
# Example:  colorFormatTextAppend(COLOR BLUE "My blue text")
#           colorFormatTextAppend(BOLD COLOR RED "My bold red text")
#
# To print: message("${COLOR_FORMATTED_TEXT_COMBINED}")
#
function(colorFormatTextAppend)
    colorFormatText(${ARGN})
    
    # Append formatted text to COLOR_FORMATTED_TEXT_COMBINED
    set(COLOR_FORMATTED_TEXT_COMBINED "${COLOR_FORMATTED_TEXT_COMBINED}${COLOR_FORMATTED_TEXT}" PARENT_SCOPE)
endfunction()

# Directly prints formatted text
#
# Example:  messageWithColor(COLOR BLUE "My blue text")
#           messageWithColor(BOLD COLOR RED "My bold red text")
#
function(messageWithColor)
    colorFormatText(${ARGN})
    message(${COLOR_FORMATTED_TEXT})
endfunction()

# =================================================================================


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

