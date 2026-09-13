include_guard(GLOBAL)

project(evil_core_lib VERSION 0.1.0 LANGUAGES C CXX)

message(STATUS "Configuring target: ${PROJECT_NAME}")
increment_log_indent()

add_library(evil_core_lib STATIC)
add_library(evil::evil_core_lib ALIAS evil_core_lib)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        evil::juce_core_lib
)

target_include_directories(${PROJECT_NAME}
    PRIVATE 
        $<BUILD_INTERFACE:${JUCE_MODULES_DIR}>
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_sources(${PROJECT_NAME}
	PRIVATE
		evil_core_lib.cpp

        time/evil_time.cpp
        time/evil_time.h

	PUBLIC
		evil_core_lib.h
)

# Organize source files into groups in Visual Studio.
get_target_property(target_sources ${PROJECT_NAME} SOURCES)

# Create the Source files group.
source_group(
    TREE "${PROJECT_SOURCE_DIR}"
    PREFIX "Source Files"
    FILES 
        ${target_sources}
)

#[[
cmake_print_properties(
    TARGETS ${PROJECT_NAME} 
    PROPERTIES 
        SOURCES
        INTERFACE_INCLUDE_DIRECTORIES
)
]]

print_target_properties(${PROJECT_NAME})

decrement_log_indent()
message(STATUS "Finished configuring target: ${PROJECT_NAME}")
message(STATUS "=================================================================================")