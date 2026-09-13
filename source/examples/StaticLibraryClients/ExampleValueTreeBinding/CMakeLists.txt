
include_guard(GLOBAL)
# Example console app demonstrating JUCE ValueTree binding helpers.

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_FOLDER EvilAudio/examples/StaticLibraryClients)

project(ExampleValueTreeBinding VERSION 0.1.0 LANGUAGES C CXX)

add_executable(${PROJECT_NAME}
    EXCLUDE_FROM_ALL
    example_value_tree_binding.cpp
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        evil::evil_mvc_lib
)
