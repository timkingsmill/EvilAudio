# Set the folder structure for IDEs
set(CMAKE_FOLDER ${PROJECT_NAME}/libs/evilaudio) 

# Create custom JUCE module interface libraries
# This is where we create the custom JUCE module interface 
# libraries for EvilAudio.
juce_add_modules(
    ALIAS_NAMESPACE evil
    evilaudio_core
    ##evilaudio_audio_devices
    evilaudio_lookandfeel
    evilaudio_eq
)

# Create static libraries for the custom JUCE interface modules.
add_juce_module_static_library(evil::evilaudio_core)
##add_juce_module_static_library(evil::evilaudio_audio_devices)
add_juce_module_static_library(evil::evilaudio_lookandfeel)
add_juce_module_static_library(evil::evilaudio_eq) 


