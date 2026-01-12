# -----------------------------------------------------------------------------------------------
# EvilDAW executable target.

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_FOLDER EvilAudio/applications/EvilDAW)

project(EvilDAW VERSION 0.1.0 LANGUAGES C CXX)

# Set EvilDAW as the startup project in Visual Studio
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT EvilDAW)

juce_add_gui_app(EvilDAW
    PRODUCT_NAME "EvilDAW"
    VERSION ${PROJECT_VERSION}
    BUNDLE_ID "com.evilaudio.evildaw"
    COMPANY_PRODUCT_NAME "EvilAudio DAW"
    COMPANY_NAME "EvilAudio"
)

# Create the JuceHeader.h for this target.
juce_generate_juce_header(EvilDAW)

set( 
    CPP_FILES
        source/EvilDAW.cpp
)

target_sources(EvilDAW
    PRIVATE
        ${CPP_FILES}
)

source_group(
    TREE "${CMAKE_CURRENT_SOURCE_DIR}/source"
    PREFIX "Tims Source Files"
    FILES ${CPP_FILES}
)

target_compile_definitions(EvilDAW
    PRIVATE
        DONT_SET_USING_JUCE_NAMESPACE=1
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:EvilDAW,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:EvilDAW,JUCE_VERSION>")

target_include_directories(EvilDAW 
    PRIVATE
        include)

# Add the include files for this target.
add_subdirectory(include)

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
#add_subdirectory(source)

# Retrieve the sources into a variable named 'target_sources_list'
#get_target_property(target_sources_list EvilDAW SOURCES)
#message(STATUS "Sources for EvilDAW: ${target_sources_list}")
#message(STATUS "--------------------------------")

# Link EvilDAW against the evilaudio.eq module.
target_link_libraries(EvilDAW
    PRIVATE 
        evilaudio::evilaudio_eq
)

target_link_libraries(EvilDAW 
    PRIVATE 
        juce::juce_recommended_warning_flags 
        juce::juce_recommended_config_flags 
        juce::juce_recommended_lto_flags
)

target_link_libraries(EvilDAW
    PRIVATE 
        juce::juce_gui_extra 
        juce::juce_audio_utils 
        juce::juce_dsp 
        juce::juce_audio_basics 
        juce::juce_core
)   

# Retrieve the sources into a variable named 'target_sources_list'
#get_target_property(target_sources_list EvilDAW SOURCES)
#message(STATUS "--------------------------------")
#message(STATUS "Sources for EvilDAW: ${target_sources_list}")
#message(STATUS "--------------------------------")


# -----------------------------------------------------------------------------------------------