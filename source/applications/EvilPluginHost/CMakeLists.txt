# -----------------------------------------------------------------------------------------------
# EvilPluginHost executable target.

set(CMAKE_FOLDER EvilAudio/applications/EvilPluginHost)

project(EvilPluginHost VERSION 0.1.0 LANGUAGES C CXX)   

evil_audio_add_gui_app(EvilPluginHost
    PRODUCT_NAME "EvilPluginHost"   
    VERSION ${PROJECT_VERSION}
    BUNDLE_ID "com.evilaudio.evilPluginHost"
    COMPANY_PRODUCT_NAME "EvilAudio EvilPluginHost"
    COMPANY_NAME "EvilAudio"
)

# Create the JuceHeader.h for this target.
juce_generate_juce_header(EvilPluginHost)

# Define preprocessor definitions for this target.
target_compile_definitions(EvilPluginHost      
    PRIVATE
        DONT_SET_USING_JUCE_NAMESPACE=1
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:EvilPluginHost,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:EvilPluginHost,JUCE_VERSION>")

target_link_libraries(EvilPluginHost     
    PRIVATE 
        juce::juce_recommended_warning_flags 
        juce::juce_recommended_config_flags 
        juce::juce_recommended_lto_flags
)

set( 
    src_files
        EvilPluginHost.cpp

        gui/MainWindowContent.cpp
        gui/MainWindowContent.h
)

target_sources(EvilPluginHost
    PRIVATE
        ${src_files}
)

source_group(
    TREE "${CMAKE_CURRENT_SOURCE_DIR}"
    PREFIX "Source Files"
    FILES ${src_files}
)

target_include_directories(EvilPluginHost    
    PRIVATE 
        include)

        
target_link_static_libraries(EvilPluginHost
    # JUCE static libraries
    evil::juce_audio_devices_lib
    evil::juce_audio_utils_lib
    evil::juce_dsp_lib

    evil::evil_audio_core_lib
    evil::evil_audio_eq_lib
)

# =================================================================================================
