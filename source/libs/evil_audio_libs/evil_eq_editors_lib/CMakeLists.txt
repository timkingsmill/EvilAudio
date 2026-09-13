include_guard(GLOBAL)

project(evil_eq_editors_lib VERSION 0.1.0 LANGUAGES C CXX)

message(STATUS "Configuring target: ${PROJECT_NAME}")
increment_log_indent()

add_library(evil_eq_editors_lib STATIC)
add_library(evil::evil_eq_editors_lib ALIAS evil_eq_editors_lib)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=0
        DONT_SET_USING_JUCE_NAMESPACE=1
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        evil::evil_core_lib
)

target_include_directories(${PROJECT_NAME}
	PRIVATE 
        $<BUILD_INTERFACE:${JUCE_MODULES_DIR}>
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_sources(${PROJECT_NAME}
	PRIVATE
       	evil_eq_editors_lib.cpp
	
        eq/ParametricEqualiserEditor.h
        eq/ParametricEqualiserEditor.cpp
		eq/ParametricEqualiserProcessor.h
		eq/ParametricEqualiserProcessor.cpp

	PUBLIC
		evil_eq_editors_lib.h
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

# Print the properties of the target for debugging purposes. 
# This is useful to verify that the target has been configured correctly 
# with the expected sources, include directories, compile definitions, and linked libraries.
print_target_properties(${PROJECT_NAME})

# Decrement the log indent level and print a message indicating that configuration is complete.
decrement_log_indent()
message(STATUS "Finished configuring target: ${PROJECT_NAME}")  
message(STATUS "=================================================================================")