# -----------------------------------------------------------------------------------------------
# EvilDAW executable target.

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_FOLDER EvilAudio/applications/EvilDAW)

project(EvilDAW VERSION 0.1.0 LANGUAGES C CXX)

# Set EvilDAW as the startup project in Visual Studio
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT EvilDAW)

evil_audio_add_gui_app(EvilDAW
    PRODUCT_NAME "EvilDAW"
    VERSION ${PROJECT_VERSION}
    BUNDLE_ID "com.evil_audio.evildaw"
    COMPANY_PRODUCT_NAME "Evil Audio DAW"
    COMPANY_NAME "Evil Audio"
)

# Create the JuceHeader.h for this target.
juce_generate_juce_header(EvilDAW)

#[[
set( 
    CPP_FILES
        source/EvilDAW.cpp
)

target_sources(EvilDAW
    PRIVATE
        ${CPP_FILES}
)
        ]]

#[[
source_group(
    TREE "${CMAKE_CURRENT_SOURCE_DIR}/source"
    PREFIX "Tims Source Files"
    FILES ${CPP_FILES}
)
    ]]

  
target_compile_definitions(EvilDAW
    PRIVATE
        #JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=0
        DONT_SET_USING_JUCE_NAMESPACE=1
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:EvilDAW,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:EvilDAW,JUCE_VERSION>"
)

# Add the include files for this target.
add_subdirectory(include)

# Set the include directories for this target.
target_include_directories(EvilDAW      
    PRIVATE 
        include
)

# Add the source files for this target.
#source_group("My Source Files\\module" 
#    FILES
#        source/EvilDAW.cpp
#)

#set(MODULE_SOURCES source/EvilDAW.cpp)
#target_sources(EvilDAW 
#    PRIVATE
#        ${MODULE_SOURCES}
#)
add_subdirectory(source)

# Link EvilDAW against the evilaudio.eq module.

target_link_libraries(EvilDAW
    PUBLIC 
        evil_juce_audio_core    
        evil_juce_audio_basics
        evil_juce_audio_devices
        evil_juce_audio_formats
        evil_juce_audio_processors
        evil_juce_audio_utils
        evil_juce_audio_eq
        evil_juce_audio_file_utils
        evil_juce_audio_gui_basics
)

# Link recommended flags.
target_link_libraries(EvilDAW 
    PRIVATE 
        juce::juce_recommended_warning_flags 
        juce::juce_recommended_config_flags 
        juce::juce_recommended_lto_flags
)

# DONT link to JUCE modules. Link EvilDAW against the static libraries.
# See libs/EvilAudio/CMakeLists.txt for JUCE module static library creation.
#[[
target_link_libraries(EvilDAW
    PRIVATE 
        juce::juce_gui_extra 
        juce::juce_audio_utils 
        juce::juce_dsp 
        juce::juce_audio_basics 
        juce::juce_core
)  
        ]] 

# -----------------------------------------------------------------------------------------------