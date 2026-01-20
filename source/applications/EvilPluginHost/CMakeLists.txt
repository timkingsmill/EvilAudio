# -----------------------------------------------------------------------------------------------
# EvilPluginHost executable target.

set(CMAKE_FOLDER EvilAudio/applications/EvilPluginHost)

project(EvilPluginHost VERSION 0.1.0 LANGUAGES C CXX)   

juce_add_gui_app(EvilPluginHost
    PRODUCT_NAME "EvilPluginHost"   
    VERSION ${PROJECT_VERSION}
    BUNDLE_ID "com.evilaudio.evilPluginHost"
    COMPANY_PRODUCT_NAME "EvilAudio EvilPluginHost"
    COMPANY_NAME "EvilAudio"
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

# Create the JuceHeader.h for this target.
juce_generate_juce_header(EvilPluginHost)

target_compile_definitions(EvilPluginHost      
    PRIVATE
        DONT_SET_USING_JUCE_NAMESPACE=1
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:EvilPluginHost,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:EvilPluginHost,JUCE_VERSION>")

target_include_directories(EvilPluginHost    
    PRIVATE 
        include)

target_link_libraries(EvilPluginHost
    PRIVATE
        evilaudio::evilaudio_core
        evilaudio::evilaudio_eq
)

target_link_libraries(EvilPluginHost     
    PRIVATE 
        juce::juce_recommended_warning_flags 
        juce::juce_recommended_config_flags 
        juce::juce_recommended_lto_flags
)

target_link_libraries(EvilPluginHost
    PRIVATE 
        juce::juce_audio_utils 
        #juce::juce_audio_devices
        #juce::juce_audio_basics 
        #juce::juce_audio_plugin_client
        juce::juce_core
        #juce::juce_dsp 
        #juce::juce_gui_extra 
)

target_add_module_source_groups(EvilPluginHost juce)
target_add_module_source_groups(EvilPluginHost evilaudio)

# =================================================================================================
