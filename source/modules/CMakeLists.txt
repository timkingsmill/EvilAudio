# Set the folder structure for IDEs
set(CMAKE_FOLDER ${PROJECT_NAME}/libs/evilaudio) 

# Create custom JUCE module interface libraries
# This is where we create the custom JUCE module interface 
# libraries for EvilAudio.
juce_add_modules(
    ALIAS_NAMESPACE evil
    evil_audio_core
    evil_audio_eq
    evil_audio_lookandfeel
)

# Create static libraries for the custom JUCE interface modules.
add_juce_module_static_library(evil::evil_audio_core)
add_juce_module_static_library(evil::evil_audio_eq) 
add_juce_module_static_library(evil::evil_audio_lookandfeel)


