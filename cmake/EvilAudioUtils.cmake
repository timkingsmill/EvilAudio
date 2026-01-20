include_guard(GLOBAL)

#===============================================================================================

cmake_minimum_required(VERSION 4.2.0)

#===============================================================================================
#[[
Function: add_juce_module_static_library
Description: Creates a static library target for a JUCE module with proper configuration,
             include paths, and compile definitions.

Arguments:
    source_module_name (required)
        The name of the JUCE module to create a static library for.
        Can include alias prefix which will be stripped before processing.

Behavior:
    - Validates that the module target exists
    - Locates the module source directory
    - Searches for module header file (.h or .hpp extension)
    - Collects all module source and header files
    - Creates a static library target named "evil_<module_name>"
    - Sets INTERFACE properties for module sources, headers, and path
    - Applies special configuration for juce_core module (system-specific linking)
    - Configures recommended JUCE compiler flags and warning levels
    - Sets compile definitions (JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED, JUCE_WEB_BROWSER, etc.)
    - Exports include directories as PUBLIC

Output Properties:
    - INTERFACE_JUCE_MODULE_SOURCES: List of module source files
    - INTERFACE_JUCE_MODULE_HEADERS: List of module header files
    - INTERFACE_JUCE_MODULE_PATH: Path to module directory

Dependencies:
    Helper functions:
    - _strip_alias_prefix()
    - _get_juce_module_path()
    - _get_juce_module_sources()
    - _add_static_library()
    - _add_standard_defs()

Special Cases:
    - juce_core: Adds atomic wrapper, execinfo on BSD, Android NDK sources on Android
]]
function (add_juce_module_static_library juce_module_name)  

    message(STATUS  "-------------------------------------------------------------------------")
    message (STATUS "Adding JUCE Module Static Library for module: ${juce_module_name}")

    # Check that the module target exists. If it doesn't, throw an error.
    if(NOT TARGET ${juce_module_name})
        message(FATAL_ERROR "Module ${juce_module_name} is not a valid or existing cmake target.")
    endif()
    # Strip any alias prefix from the module name.
    _strip_alias_prefix(${juce_module_name} stripped_module_name)
    set(juce_module_name ${stripped_module_name})

    _get_juce_module_path(${juce_module_name} source_module_path)
    
    # Fail if the path to the module source directory does not exist
    if(NOT EXISTS "${source_module_path}")
        message(FATAL_ERROR " ${juce_module_name} source directory does not exist: ${source_module_path}")
    endif()

    message(STATUS "\tFound sources path for JUCE [${juce_module_name}] module : ${source_module_path}")
    set(library_target_name "evil_${juce_module_name}")
    message(STATUS "\tStatic Library Target Name: ${library_target_name}")

    # Check that the module header file exists
    set(module_header_name "${juce_module_name}.h")
    
    # First check for .h extension, then .hpp
    if(NOT EXISTS "${source_module_path}/${juce_module_name}/${module_header_name}")
        set(module_header_name "${module_header_name}pp")
    endif()

    # Fail if the module header file does not exist
    if(NOT EXISTS "${source_module_path}/${juce_module_name}/${module_header_name}")
        message(FATAL_ERROR "${juce_module_name} Module header does not exist: ${source_module_path}/${module_header_name}")
    endif()

    # Found the module header
    message(STATUS "\tFound module header: ${module_header_name}")

    # Get the module source and header files
    _get_juce_module_sources("${source_module_path}/${juce_module_name}" module_sources module_headers)
    
    #[[
    foreach(filename ${module_headers})
        message(STATUS "Module header: ${filename}")
    endforeach()
    foreach(filename ${module_sources})
        message(STATUS "Module source: ${filename}")
    endforeach()
    ]]
    
    set(all_module_sources)
    list(APPEND all_module_sources ${module_sources})

    set(lib_target_name "evil_${juce_module_name}")
    _add_static_library("${lib_target_name}" "${all_module_sources}")

    set_target_properties(${lib_target_name} PROPERTIES
        INTERFACE_JUCE_MODULE_SOURCES   "${module_sources}"
        INTERFACE_JUCE_MODULE_HEADERS   "${module_headers}"
        INTERFACE_JUCE_MODULE_PATH      "${source_module_path}"
    )

    #if(JUCE_ENABLE_MODULE_SOURCE_GROUPS)
    #    message(STATUS "\tAdding module headers to: ${lib_target_name}")
    #    target_sources(${lib_target_name} INTERFACE ${module_headers})
    #endif()

    # Config special case juce_core  
    if(${juce_module_name} STREQUAL "juce_core")

        message(STATUS "SPECIAL CASE")

        _add_standard_defs(${lib_target_name})

        target_link_libraries(${lib_target_name} INTERFACE juce::juce_atomic_wrapper)

        if(CMAKE_SYSTEM_NAME MATCHES ".*BSD")
            target_link_libraries(${lib_target_name} INTERFACE execinfo)
        elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")
            target_sources(${lib_target_name} INTERFACE "${ANDROID_NDK}/sources/android/cpufeatures/cpu-features.c")
            target_include_directories(${lib_target_name} INTERFACE "${ANDROID_NDK}/sources/android/cpufeatures")
            target_link_libraries(${lib_target_name} INTERFACE android log)
        endif()
    endif()

    target_link_libraries(${lib_target_name}
        PRIVATE
            #juce::juce_core
            # If you're using your own JUCE-style modules,
            # you should link those here too
        PUBLIC
            juce::juce_recommended_config_flags
            juce::juce_recommended_lto_flags
            juce::juce_recommended_warning_flags
    )

    # We're linking the modules privately, but we need to export
    # their compile flags
    target_compile_definitions(${lib_target_name}
        PUBLIC
            JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=0
            JUCE_WEB_BROWSER=0
            JUCE_USE_CURL=0
            JUCE_INCLUDE_ZLIB_CODE=1
        INTERFACE
            $<TARGET_PROPERTY:${lib_target_name},COMPILE_DEFINITIONS>
    )

    # We also need to export the include directories for the modules
    # Must be public to add the include path to both this library
    # and its clients.
    target_include_directories(${lib_target_name}
        PUBLIC
            "${source_module_path}"
    )

    message(STATUS "-------------------------------------------------------------------------")
endfunction()


#===============================================================================================
#
# _add_static_library
# 
# Private function to create a static library target with the given sources.
#
# ARGUMENTS:
#   target  - The name of the static library target to create
#   sources - List of source files to compile into the static library
#
# DESCRIPTION:
#   This is an internal helper function used to add a static library target
#   to the CMake project. It should not be called directly from outside this module.
#
function(_add_static_library target sources)
    add_library(${target} STATIC)
    foreach(filename ${sources})
        message(STATUS "\tAdding source file to ${target}: ${filename}")
    endforeach()
    target_sources(${target} PRIVATE ${sources})
endfunction()

#===============================================================================================
#
# _get_juce_module_sources
# 
# Internal function to extract and categorize source files from a JUCE module.
#
# Arguments:
#   module_path (in)      - Path to the JUCE module directory
#   built_sources (out)   - Variable name to store compiled source files
#   other_sources (out)   - Variable name to store other source files (headers, etc.)
#
# Description:
#   This function scans a JUCE module directory and separates source files into
#   two categories: files that need to be compiled and other supporting files.
#   Results are stored in the parent scope using the provided variable names.
#
# Example:
#   _get_juce_module_sources("${JUCE_MODULES_PATH}/juce_core" CORE_SOURCES CORE_HEADERS)
function (_get_juce_module_sources module_path built_sources other_sources)
    message(STATUS "\tCollecting sources from the module's path: ${module_path}")
 
    get_filename_component(module_parent_path ${module_path} DIRECTORY)
    get_filename_component(module_name ${module_path} NAME)

    file(GLOB_RECURSE all_module_files
        CONFIGURE_DEPENDS LIST_DIRECTORIES FALSE
        RELATIVE "${module_parent_path}"
        "${module_path}/*"
    )

    # Filter to only include .c, .cpp, .cc, .cxx, .s, .asm files at the top level of the module directory
    set(module_cpp_files "${all_module_files}")
    set(regex "^${module_name}/${module_name}[^/]*\\.(c|cc|cpp|cxx|s|asm)$")
    #message(STATUS ${regex})
    list(FILTER module_cpp_files INCLUDE REGEX "${regex}")
    #foreach (source_file ${module_cpp_files})
    #    message(STATUS "After filtering all module files: ${source_file}")
    #endforeach()

    set(module_files_to_build)
    foreach(filename IN LISTS module_cpp_files)
        #message(STATUS "module_cpp: ${filename}")
        _should_build_source_file("${filename}" should_build_file)
        if(should_build_file)
            list(APPEND module_files_to_build "${filename}")
        endif()
    endforeach()

    if(NOT "${module_files_to_build}" STREQUAL "")
        list(REMOVE_ITEM headers ${module_files_to_build})
    endif()
  
    # Now we need to make the paths relative to the module path
    set(headers ${all_module_files})
    set(output_path "${module_parent_path}")
    foreach(source_list ITEMS module_files_to_build headers)
        list(TRANSFORM ${source_list} PREPEND "${output_path}/")
    endforeach()

    set(${built_sources} ${module_files_to_build} PARENT_SCOPE)
    set(${other_sources} ${headers} PARENT_SCOPE)

endfunction()

#===============================================================================================

function(_add_standard_defs lib_target)
    _juce_get_debug_config_genex(debug_config)

    target_compile_definitions(${lib_target} INTERFACE
        JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1
        $<IF:${debug_config},DEBUG=1 _DEBUG=1,NDEBUG=1 _NDEBUG=1>
        $<$<PLATFORM_ID:Android>:JUCE_ANDROID=1>)
endfunction()

#===============================================================================================

function(_should_build_source_file filename output_var)
    message(STATUS "\tShould build this: ${filename}")

    get_filename_component(trimmed_filename "${filename}" NAME_WE)
    string(TOLOWER "${trimmed_filename}" trimmed_filename_lowercase)

    set(system_name_regex_for_suffix
        "android\;Android"
        "ios\;iOS"
        "linux\;Linux|.*BSD"
        "mac\;Darwin"
        "osx\;Darwin"
        "windows\;Windows")

    set(result TRUE)

    foreach(pair IN LISTS system_name_regex_for_suffix)
        list(GET pair 0 suffix)
        list(GET pair 1 regex)

        if((trimmed_filename_lowercase MATCHES "_${suffix}$") AND NOT (CMAKE_SYSTEM_NAME MATCHES "${regex}"))
            set(result FALSE)
        endif()
    endforeach()

    set("${output_var}" "${result}" PARENT_SCOPE)
endfunction()

#===============================================================================================

function (_strip_alias_prefix module_name return_var)
    string(REGEX REPLACE "^[a-zA-Z0-9]+::" "" stripped_name "${module_name}")
    set(${return_var} "${stripped_name}" PARENT_SCOPE)
endfunction()

#===============================================================================================
# Function to get JUCE module path for a given module
# Usage: _get_juce_module_path(<module_name> <return_var>)
# Example: _get_juce_module_path(JUCE::juce_core JUCE_CORE_PATH)
function(_get_juce_module_path module_name return_var)
    get_target_property(module_path ${module_name} INTERFACE_JUCE_MODULE_PATH)
    if(NOT module_path STREQUAL "module_path-NOTFOUND")
        set(${return_var} "${module_path}" PARENT_SCOPE)
    else()
        message(FATAL_ERROR "Module ${module_name} does not have a valid INTERFACE_JUCE_MODULE_PATH property.")
    endif()
endfunction()

#===============================================================================================

# Function to get JUCE_CMAKE_UTILS_DIR
# If JUCE_CMAKE_UTILS_DIR is not defined, try to get it from environment variable
# If still not defined, throw an error
function (get_juce_cmake_utils_dir return_var)
    if(DEFINED JUCE_CMAKE_UTILS_DIR)
        set(${return_var} "${JUCE_CMAKE_UTILS_DIR}" PARENT_SCOPE)
    else()
        if(DEFINED ENV{JUCE_CMAKE_UTILS_DIR})
            set(JUCE_CMAKE_UTILS_DIR "$ENV{JUCE_CMAKE_UTILS_DIR}")
            set(${return_var} "${JUCE_CMAKE_UTILS_DIR}" PARENT_SCOPE)
        else()
            message(FATAL_ERROR "JUCE_CMAKE_UTILS_DIR is not defined. Please set this variable to point to the JUCE CMake Utils directory.")
        endif()
    endif()
endfunction()

# ================================================================================================    