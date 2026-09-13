include_guard(GLOBAL)
# -----------------------------------------------------------------------------------------------
# EvilDAW executable target.

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_FOLDER EvilAudio/applications/EvilDAW)

project(EvilDAW VERSION 0.1.0 LANGUAGES C CXX)

message(STATUS "Configuring EvilDAW Application...")
increment_log_indent()

add_executable(${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} 
    PROPERTIES
        WIN32_EXECUTABLE TRUE
)

target_sources(${PROJECT_NAME}
    PRIVATE
        evil_daw_main.cpp
)

# Organize source files into groups in Visual Studio.
get_target_property(target_sources ${PROJECT_NAME} SOURCES)

# Create the Source files group.
source_group(
    TREE "${CMAKE_CURRENT_SOURCE_DIR}"
    PREFIX "Source Files"
    FILES 
        ${target_sources}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        EvilDAWLib
        #evil::evil_eq_editors_lib
        #evil::juce_core_lib
)

decrement_log_indent()

# -----------------------------------------------------------------------------------------------