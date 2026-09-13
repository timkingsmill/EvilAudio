include_guard(GLOBAL)

# ------------------------------------------------------------------------------------------------
# \file EvilAudioCMakeUtils.cmake
# \brief Utility CMake functions for EvilAudio project.
#
# This file contains helper functions for project organization and build configuration.
# ------------------------------------------------------------------------------------------------


# ------------------------------------------------------------------------------------------------
## \brief Assigns GoogleTest static library targets to a solution folder.
#
# This helper places only the GoogleTest static library targets in the requested
# solution folder to keep the Visual Studio target tree organized.
#
# \param solution_folder The name of the solution folder to assign targets to.
function(apply_google_test_static_lib_folder solution_folder)
    if(NOT TARGET gmock)
        message(WARNING "Target 'gmock' not found. Make sure to include this function after FetchContent_MakeAvailable(googletest).")
    else()
        set_target_properties(gmock PROPERTIES FOLDER "${solution_folder}")
    endif()

    if(NOT TARGET gmock_main)
        message(WARNING "Target 'gmock_main' not found. Make sure to include this function after FetchContent_MakeAvailable(googletest).")
    else()
        set_target_properties(gmock_main PROPERTIES FOLDER "${solution_folder}")
    endif()

    if(NOT TARGET gtest)
        message(WARNING "Target 'gtest' not found. Make sure to include this function after FetchContent_MakeAvailable(googletest).")
    else()
        set_target_properties(gtest PROPERTIES FOLDER "${solution_folder}")
    endif()

    if(NOT TARGET gtest_main)
        message(WARNING "Target 'gtest_main' not found. Make sure to include this function after FetchContent_MakeAvailable(googletest).")
    else()
        set_target_properties(gtest_main PROPERTIES FOLDER "${solution_folder}")
    endif()
endfunction()

# ------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------
