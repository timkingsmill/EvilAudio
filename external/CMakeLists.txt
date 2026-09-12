include_guard(GLOBAL)

# Setup global options and variables for the JUCE project.

# Disable the building of JUCE helper tools
# since we don't need them for this project.
set(JUCE_BUILD_HELPER_TOOLS OFF FORCE)
set(JUCE_MODULES_ONLY ON FORCE)

add_subdirectory(JUCE)



