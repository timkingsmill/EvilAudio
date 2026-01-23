
# Create custom JUCE module interface libraries
juce_add_modules(
    ALIAS_NAMESPACE evilaudio
    evil_audio_core
    evil_audio_eq
    evil_audio_lookandfeel
)

# Set the folder structure for IDEs
set(CMAKE_FOLDER ${PROJECT_NAME}/libs/evilaudio) 

# Create static libraries for the custom JUCE interface modules.
add_juce_module_static_library(evilaudio::evil_audio_core)
add_juce_module_static_library(evilaudio::evil_audio_eq) 
add_juce_module_static_library(evilaudio::evil_audio_lookandfeel)


