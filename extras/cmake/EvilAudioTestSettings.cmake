include_guard(GLOBAL)

# =================================================================================================
# Adds a .runsettings file to Visual Studio Solution Items
#
# Usage: evil_audio_add_runsettings_to_solution_items(<runsettings_file>)
# =================================================================================================
function(evil_audio_add_runsettings_to_solution_items runsettings_file)
    if(WIN32 AND CMAKE_GENERATOR MATCHES "Visual Studio")
        if(EXISTS "${runsettings_file}")
            set_property(DIRECTORY APPEND PROPERTY VS_SOLUTION_ITEMS "${runsettings_file}")
            message(STATUS "Added runsettings file to Visual Studio Solution Items: ${runsettings_file}")
        else()
            message(STATUS "Runsettings file not found: ${runsettings_file}")
        endif()
    endif()
endfunction()

# =================================================================================================

function(evil_audio_setup_google_test)
    include(FetchContent)
    FetchContent_Declare(googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
    )

    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    apply_google_test_static_lib_folder("Testing/google_test")

endfunction()

# --------------------------------------------------------------------------------
# Recursively find all .cpp files in the tests subdirectory
function(evil_audio_find_test_sources test_sources)
    file(GLOB_RECURSE found_cpp_files
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp"
    )

    message(STATUS "Found test source files in ${CMAKE_CURRENT_SOURCE_DIR}/tests:")
    increment_log_indent()
    foreach(file IN LISTS found_cpp_files)
        message(STATUS "${file}")
    endforeach()
    decrement_log_indent()
    message(STATUS "===============================================================")
    set(${test_sources} ${found_cpp_files} PARENT_SCOPE)
endfunction()

# =================================================================================================
