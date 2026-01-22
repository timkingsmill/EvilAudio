
# Set the folder structure for IDEs
cmake_path(GET CMAKE_CURRENT_SOURCE_DIR PARENT_PATH parent_path)
cmake_path(GET parent_path FILENAME parent_folder_name)
cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME current_file_name)
set(CMAKE_FOLDER ${parent_folder_name}/${current_file_name})

#[[
function(get_all_targets _result _dir)
    # Recursively get targets from subdirectories
    get_property(_subdirs DIRECTORY "${_dir}" PROPERTY SUBDIRECTORIES)
    foreach(_subdir IN LISTS _subdirs)
        get_all_targets(${_result} "${_subdir}")
    endforeach()

    # Get targets from the current directory
    get_property(_sub_targets DIRECTORY "${_dir}" PROPERTY BUILDSYSTEM_TARGETS)

    # Combine and set the result in the parent scope
    set(${_result} ${${_result}} ${_sub_targets} PARENT_SCOPE)
endfunction()

# Usage:
get_all_targets(ALL_TARGETS ${CMAKE_SOURCE_DIR})
message("All targets: ${ALL_TARGETS}")
]]#

# Create static libraries for JUCE modules.
add_juce_module_static_library(juce::juce_core)
add_juce_module_static_library(juce::juce_data_structures)
add_juce_module_static_library(juce::juce_events)
add_juce_module_static_library(juce::juce_graphics)
add_juce_module_static_library(juce::juce_gui_basics)