cmake_minimum_required(VERSION 4.2.0)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
# Set C++23 as the default for all targets
set(CMAKE_CXX_STANDARD 23)

add_subdirectory(libs)
# Include folder for EvilAudio custom juce type modules.
add_subdirectory(modules)

#add_subdirectory(plugins/EvilEQPlugin)

add_subdirectory(applications/EvilDAW)
add_subdirectory(applications/EvilEQ)
#add_subdirectory(applications/EvilLookAndFeel)

