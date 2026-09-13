include_guard(GLOBAL)

# =================================================================================
# Detects the ASIO SDK in source/libs/asiosdk/common, enables JUCE ASIO support on Windows,
# and sets ASIO_SDK_PATH accordingly; otherwise warns and disables ASIO support.
# The ASIO SDK can be downloaded from https://developer.steinberg.help/display/ASIOSDK/ASIO+SDK+Download
# 
function(evil_detect_asio_sdk)
    set(ASIO_SDK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/source/libs/asiosdk/common")
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/source/libs/asiosdk/common")
        if (WIN32)
            message(STATUS "ASIO SDK found at ${ASIO_SDK_DIR}")
            set(ASIO_SDK_PATH "${ASIO_SDK_DIR}" CACHE PATH "Path to the ASIO SDK")
            set(JUCE_ASIO_SUPPORT TRUE CACHE BOOL "Enable ASIO support in JUCE")
        endif()
    else()
        message(WARNING "ASIO SDK not found in ${ASIO_SDK_DIR}. ASIO support will be disabled.")
        set(ASIO_SDK_PATH "" CACHE PATH "Path to the ASIO SDK")
    endif()
endfunction()

# =================================================================================

