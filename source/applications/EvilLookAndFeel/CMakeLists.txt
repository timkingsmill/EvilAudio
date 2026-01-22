# -----------------------------------------------------------------------------------------------
# EvilLookAndFeel executable target.

set(CMAKE_FOLDER EvilAudio/applications/EvilLookAndFeel)

project(EvilLookAndFeel VERSION 0.1.0 LANGUAGES C CXX)

juce_add_gui_app(EvilLookAndFeel
    PRODUCT_NAME "EvilLookAndFeel"
    VERSION ${PROJECT_VERSION}
    BUNDLE_ID "com.evilaudio.evillookandfeel"
    COMPANY_PRODUCT_NAME "EvilAudio LookAndFeel"
    COMPANY_NAME "EvilAudio"
)

# Create the JuceHeader.h for this target.
juce_generate_juce_header(EvilLookAndFeel)

target_compile_definitions(EvilLookAndFeel         
    PRIVATE
        DONT_SET_USING_JUCE_NAMESPACE=1
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:EvilLookAndFeel,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:EvilLookAndFeel,JUCE_VERSION>")

target_include_directories(EvilLookAndFeel      
    PRIVATE 
        include)

# Add the source files for this target.
add_subdirectory(include)    
add_subdirectory(source)    

target_link_libraries(EvilLookAndFeel
    PRIVATE
        evilaudio::evilaudio_core
        evilaudio::evilaudio_lookandfeel
)

target_link_libraries(EvilLookAndFeel        
    PRIVATE 
        juce::juce_recommended_warning_flags 
        juce::juce_recommended_config_flags 
        juce::juce_recommended_lto_flags
)

target_link_libraries(EvilLookAndFeel
    PRIVATE 
        juce::juce_audio_utils 
        juce::juce_audio_devices
        juce::juce_audio_basics 
        juce::juce_audio_plugin_client
        juce::juce_core
        juce::juce_dsp 
        juce::juce_gui_extra 
)

# =================================================================================================
