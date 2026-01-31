
# Set the folder structure for IDEs
cmake_path(GET CMAKE_CURRENT_SOURCE_DIR PARENT_PATH parent_path)
cmake_path(GET parent_path FILENAME parent_folder_name)
##cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME current_file_name)
set(CMAKE_FOLDER ${PROJECT_NAME}/${parent_folder_name}/juce) #${current_file_name})

# Function to print all targets in the project (for debugging purposes)
function(print_all_targets DIR)
    get_property(TGTS DIRECTORY "${DIR}" PROPERTY BUILDSYSTEM_TARGETS)
    foreach(TGT IN LISTS TGTS)
        message(STATUS "\tJUCE ModuleTarget: ${TGT}")
        # TODO: Do something about it
    endforeach()

    get_property(SUBDIRS DIRECTORY "${DIR}" PROPERTY SUBDIRECTORIES)
    foreach(SUBDIR IN LISTS SUBDIRS)
        print_all_targets("${SUBDIR}")
    endforeach()
endfunction()

print_all_targets(${CMAKE_SOURCE_DIR})

# Create static libraries for JUCE modules.
add_juce_module_static_library(juce::juce_core)
add_juce_module_static_library(juce::juce_audio_basics)
add_juce_module_static_library(juce::juce_audio_devices)
add_juce_module_static_library(juce::juce_audio_formats)
add_juce_module_static_library(juce::juce_audio_processors) 
add_juce_module_static_library(juce::juce_audio_utils)
add_juce_module_static_library(juce::juce_dsp)
add_juce_module_static_library(juce::juce_data_structures)
add_juce_module_static_library(juce::juce_events)
add_juce_module_static_library(juce::juce_graphics)
add_juce_module_static_library(juce::juce_gui_basics)
add_juce_module_static_library(juce::juce_gui_extra)
