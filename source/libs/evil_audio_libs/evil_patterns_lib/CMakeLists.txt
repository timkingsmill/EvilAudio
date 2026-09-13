include_guard(GLOBAL)

# evil_mvc_lib CMake target definition.
#
# This file declares the static library, its include paths, link dependencies,
# and the source/header files exported as part of the public API surface.

# -----------------------------------------------------------------------------------------------
# evil_patterns_lib static library target.

project(evil_patterns_lib VERSION 0.1.0 LANGUAGES C CXX)

reset_log_indent()
message(STATUS "Configuring static library target: ${PROJECT_NAME}")
increment_log_indent()

add_library(${PROJECT_NAME} STATIC)
add_library(evil::evil_patterns_lib ALIAS ${PROJECT_NAME})

target_compile_definitions(${PROJECT_NAME}
    PUBLIC
        JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=0
        DONT_SET_USING_JUCE_NAMESPACE=1
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        $<BUILD_INTERFACE:${JUCE_MODULES_DIR}>
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        evil::juce_gui_basics_lib
    PUBLIC
        evil::evil_core_lib
        evil::juce_data_structures_lib
)

target_sources(${PROJECT_NAME}
    PUBLIC
        evil_patterns_lib.h
    PRIVATE
        evil_patterns_lib.cpp
)

add_subdirectory(mvc)

# Organize source files into groups in Visual Studio.
get_target_property(target_sources ${PROJECT_NAME} SOURCES)

# Create the Source files group.
source_group(
    TREE "${CMAKE_CURRENT_SOURCE_DIR}"
    PREFIX "Source Files"
    FILES 
        ${target_sources}
)

decrement_log_indent()
message(STATUS "Finished configuring ${PROJECT_NAME} Static Library.")
message(STATUS "=================================================================================")






