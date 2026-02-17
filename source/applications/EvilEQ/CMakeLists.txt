# -----------------------------------------------------------------------------------------------
# EvilEQ executable target.

project(EvilEQ VERSION 0.1.0 LANGUAGES C CXX)   

set(CMAKE_FOLDER EvilAudio/applications/EvilEQ)

evil_audio_add_gui_app(EvilEQ
    PRODUCT_NAME "EvilEQ"   
    VERSION ${PROJECT_VERSION}
    BUNDLE_ID "com.evilaudio.evilEq"
    COMPANY_PRODUCT_NAME "EvilAudio EvilEQ"
    COMPANY_NAME "EvilAudio"
)

# Create the JuceHeader.h for this target.
juce_generate_juce_header(EvilEQ)

target_compile_definitions(EvilEQ      
    PRIVATE
        DONT_SET_USING_JUCE_NAMESPACE=1
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:EvilEQ,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:EvilEQ,JUCE_VERSION>")

target_link_libraries(EvilEQ     
    PRIVATE 
        juce::juce_recommended_warning_flags 
        juce::juce_recommended_config_flags 
        juce::juce_recommended_lto_flags
)

target_include_directories(EvilEQ    
    PRIVATE 
        include)

# Add the source files for this target.
add_subdirectory(source)

target_link_static_libraries(EvilEQ
    # JUCE static libraries
    evil::juce_audio_devices_lib
    evil::juce_audio_utils_lib
    evil::juce_dsp_lib

    # EvilAudio static libraries
    evil::evilaudio_core_lib
    evil::evilaudio_lookandfeel_lib
    evil::evilaudio_eq_lib
)

# =================================================================================================
