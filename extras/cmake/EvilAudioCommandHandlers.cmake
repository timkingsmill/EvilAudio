include_guard(GLOBAL)

function(evilaudio_handle_configure_time_command requested_command)
    set(_supported_commands "none" "configure")

    set(_command "${requested_command}")
    string(STRIP "${_command}" _command)
    string(TOLOWER "${_command}" _command)

    if(_command STREQUAL "")
        set(_command "none")
    endif()

    if(_command STREQUAL "none")
        return()
    endif()

    if(_command STREQUAL "configure")
        message(STATUS "Handling -DEVIL_AUDIO_COMMAND=${_command}")
        return()
    endif()

    string(JOIN ", " _supported_commands_csv ${_supported_commands})
    message(FATAL_ERROR
        "Unsupported -DEVIL_AUDIO_COMMAND value: '${requested_command}'. "
        "Supported values: ${_supported_commands_csv}")
endfunction()
