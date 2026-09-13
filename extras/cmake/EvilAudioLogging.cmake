include_guard(GLOBAL)

## =================================================================================
## @file EvilAudioLogging.cmake
## @brief Logging helpers for formatted and indented CMake output.
##
## @details
## This module centralizes logging behavior used across the EvilAudio build system.
## It provides:
## - A wrapper around CMake's built-in `message()` command.
## - Global indentation management for nested logging scopes.
##
## Indentation state is tracked in the global property `GLOBAL_MESSAGE_INDENT`
## and mirrored into `CMAKE_MESSAGE_INDENT` for the caller scope.

## =================================================================================
## @brief Initializes the global indentation property used by logging helpers.
##
## @details
## Stores indentation as a CMake list where each element is one indentation unit
## (`"    "`). The list representation makes append/pop operations simple.
set_property(GLOBAL PROPERTY GLOBAL_MESSAGE_INDENT "")

## =================================================================================
## @brief Wrapper for CMake's built-in `message()` command.
##
## @details
## The wrapper forwards all arguments to `_message(...)`, which is the original
## built-in implementation preserved by CMake when overriding `message`.
##
## This hook allows centralized formatting, filtering, or additional behavior
## to be added later without changing call sites.
##
## @param level Message mode (for example `STATUS`, `WARNING`, `FATAL_ERROR`).
## @param ARGN Remaining message text/arguments passed through verbatim.
function(message level)
    # Preserve native message behavior via the original built-in command.
    _message(${ARGV0} ${ARGN})
endfunction()

## =================================================================================
## @brief Increases the current logging indentation level.
##
## @details
## Appends one indentation unit (four spaces) to `GLOBAL_MESSAGE_INDENT`, then
## updates `CMAKE_MESSAGE_INDENT` in the parent scope so subsequent `message()`
## calls in that scope render with deeper indentation.
##
## @note Intended for use at the start of nested operations.
function(increment_log_indent)
    get_property(global_message_indent GLOBAL PROPERTY GLOBAL_MESSAGE_INDENT)
    list(APPEND global_message_indent "    ")
    set(CMAKE_MESSAGE_INDENT "${global_message_indent}" PARENT_SCOPE)
    set_property(GLOBAL PROPERTY GLOBAL_MESSAGE_INDENT "${global_message_indent}")
endfunction()

## =================================================================================
## @brief Decreases the current logging indentation level.
##
## @details
## Removes one indentation unit from `GLOBAL_MESSAGE_INDENT`, then writes the
## updated value to `CMAKE_MESSAGE_INDENT` in the parent scope.
##
## @note Intended to pair with `increment_log_indent()`.
## @warning Calling this when no indentation unit exists may fail because
## `list(POP_BACK ...)` requires a non-empty list.
function(decrement_log_indent)
    get_property(global_message_indent GLOBAL PROPERTY GLOBAL_MESSAGE_INDENT)
    list(POP_BACK global_message_indent)
    set(CMAKE_MESSAGE_INDENT "${global_message_indent}" PARENT_SCOPE)
    set_property(GLOBAL PROPERTY GLOBAL_MESSAGE_INDENT "${global_message_indent}")
endfunction()

## =================================================================================
## @brief Resets logging indentation to root depth.
##
## @details
## Clears both the global indentation store and the caller-visible
## `CMAKE_MESSAGE_INDENT` value.
function(reset_log_indent)
    set_property(GLOBAL PROPERTY GLOBAL_MESSAGE_INDENT "")
    set(CMAKE_MESSAGE_INDENT "" PARENT_SCOPE)
endfunction()

## =================================================================================
## =================================================================================
