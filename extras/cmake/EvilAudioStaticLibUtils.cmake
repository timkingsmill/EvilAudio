include_guard(GLOBAL)

# =============================================================================

set(EVIL_STATIC_LIBS_BLACKLIST
    "juce_audio_plugin_client"
)

# =============================================================================

## @brief Creates a static library target for a JUCE module.
##
## @param source_path Full path to the JUCE module directory.
## @details The module must contain a header matching the module directory
## name, such as juce_core/juce_core.h. Blacklisted modules are skipped.
## Dependencies are read from module metadata and linked to the target.
## @throws FATAL_ERROR if source_path or its module header is invalid.
function(evil_add_static_lib source_path)
    # Validate the source path and locate the module header.
    _evil_validate_library_sources_path(${source_path} IsValid module_header_file)
    if(NOT IsValid)
        message(FATAL_ERROR "Invalid source path for static library: ${module_header_file}")
    endif()

    get_filename_component(module_name ${source_path} NAME)
    message(STATUS "Adding static library for JUCE module: ${module_name}")

    # Derive the target name from the module header file.
    get_filename_component(lib_target_name ${module_header_file} NAME_WE)  

    if(lib_target_name IN_LIST EVIL_STATIC_LIBS_BLACKLIST)
        message(STATUS "Skipping static library target for blacklisted module: ${lib_target_name}")
        return()
    endif()

    set(lib_target_name ${lib_target_name}_lib)
    project(${lib_target_name} VERSION 0.1.0 LANGUAGES C CXX)

    add_library(${lib_target_name} STATIC EXCLUDE_FROM_ALL)
    add_library(evil::${lib_target_name} ALIAS ${lib_target_name})

    _evil_get_library_sources(${source_path} source_files header_files)
    
    # Header sources are intentionally not added separately.
    target_sources(${lib_target_name} 
        PRIVATE 
            ${source_files} 
        PUBLIC
            ${module_header_file})

    # Expose the parent and module directories for module and dependency headers.
    get_filename_component(_all_libraries_folder_name ${source_path} DIRECTORY)  

    target_include_directories(${lib_target_name}      
        PUBLIC
            ${_all_libraries_folder_name}
        PUBLIC
            ${source_path}
    )
    
    # Public-header installation is currently disabled.

    #[[
        if(NOT "${header_files}" STREQUAL "")
            message(STATUS "Setting public headers for target ${lib_target_name}: ${header_files}")
            set_target_properties(${lib_target_name} PROPERTIES PUBLIC_HEADER "${header_files}")
        endif()
        ]]

    # Define JUCE module and project-wide compile definitions.
    target_compile_definitions(${lib_target_name}
        PUBLIC 
            JUCE_MODULE_AVAILABLE_${module_name}=1
    
        PUBLIC
            JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=0
            DONT_SET_USING_JUCE_NAMESPACE=1
            # Enable these features explicitly only when the consuming app requires them.
            JUCE_WEB_BROWSER=0
            JUCE_USE_CURL=0
    )

    # Apply Windows- and MSVC-specific options where required.
    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        if((CMAKE_CXX_COMPILER_ID STREQUAL "MSVC") OR (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC"))

            message(STATUS "Applying Windows-specific compile options for target ${lib_target_name}")
            increment_log_indent()

            if(lib_target_name MATCHES "evil_graphics")
                target_compile_options(
                    ${lib_target_name} 
                        PUBLIC 
                            #JUCE_DISABLE_COREGRAPHICS_FONT_SMOOTHING=1
                            #JUCE_INCLUDE_PNGLIB_CODE=0
                            #JUCE_INCLUDE_JPEGLIB_CODE=0
                            #USE_COREGRAPHICS_RENDERING=0
                )
            endif()

            if(lib_target_name MATCHES "juce_core_lib|juce_gui_basics_lib|juce_graphics_lib")
                message(STATUS "Adding compile definitions option to [${lib_target_name}] set path to zlib files")
                target_compile_definitions(
                    ${lib_target_name} 
                        PUBLIC 
                            JUCE_INCLUDE_ZLIB_CODE=1
                            JUCE_ZLIB_INCLUDE_PATH="zip/zlib/zlib.h"
                )
            endif()

            if(lib_target_name MATCHES "juce_core_lib|juce_gui_basics_lib|juce_graphics_lib")
                message(STATUS "Adding /bigobj compile option to [${lib_target_name}] to avoid C1128 errors")
                target_compile_options(
                    ${lib_target_name} 
                        PUBLIC 
                            /bigobj
                )
            endif()
            
            if(lib_target_name MATCHES "juce_audio_devices_lib")
                if(JUCE_ASIO_SUPPORT)
                    # The ASIO SDK is expected in source/libs/asiosdk/common.
                    message(STATUS "Enabling ASIO support for ${lib_target_name}")
                    message(STATUS "ASIO_SDK_DIR is set to: ${ASIO_SDK_DIR}")

                    target_compile_definitions(${lib_target_name} PUBLIC JUCE_ASIO=1)
                    target_include_directories(${lib_target_name} PUBLIC "${ASIO_SDK_DIR}")
                endif()
            endif()

            decrement_log_indent()
        endif()
    endif()

    # Read module dependencies and link them to the generated target.
    set(metadata_dict)
    _evil_extract_metadata_block(JUCE_MODULE_DECLARATION "${module_header_file}" metadata_dict)
    _evil_get_metadata_value("${metadata_dict}" "dependencies" dependencies)

    foreach(dependency ${dependencies})
        string(TOLOWER "${dependency}" dependency_lowercase)
        set(dependency_target_name ${dependency_lowercase}_lib)
        message(STATUS "Adding link library dependency: ${dependency_target_name}")
        target_link_libraries(${lib_target_name} PRIVATE ${dependency_target_name})
        # Add the dependency's module directory to the include path.
        get_filename_component(_modules_path ${source_path} DIRECTORY)  
        set(dependency_include_dir ${_modules_path}/${dependency})
        message(STATUS "Adding include directory for dependency ${dependency_target_name}: ${dependency_include_dir}")
        target_include_directories(${lib_target_name} PUBLIC ${dependency_include_dir})
    endforeach()

    # Uncomment to inspect the generated target properties while debugging.
    #print_target_properties(${lib_target_name})
    
    reset_log_indent()
    message(STATUS "Finished adding static library target: ${lib_target_name}")
    message(STATUS "=================================================================================")

endfunction()

# =============================================================================

## @brief Validates a library source path and locates its module header.
##
## @param source_path Library source directory to validate.
## @param IsValid Variable receiving TRUE when the path and header are valid.
## @param module_header_file Variable receiving the module header path.
## @details The expected header name is derived from the source directory.
## A missing directory is fatal; a missing header emits a warning.
function(_evil_validate_library_sources_path source_path IsValid module_header_file)
    if(NOT EXISTS ${source_path})
        message(FATAL_ERROR "Directory does not exist: ${source_path}")
        set(${IsValid} FALSE PARENT_SCOPE)
    elseif(NOT IS_DIRECTORY ${source_path})
        message(FATAL_ERROR "Path is not a directory: ${source_path}")
        set(${IsValid} FALSE PARENT_SCOPE)
    else()
        # Uncomment to log valid source directories during debugging.
        set(${IsValid} TRUE PARENT_SCOPE)
    endif()

    # Derive the expected module header name from the directory name.
    get_filename_component(module_name ${source_path} NAME)  
    set(module_header ${source_path}/${module_name}.h)

    # Validate the expected module header.
    if(NOT EXISTS ${module_header})
        message(WARNING "Module header file not found: ${module_header}")
        set(${IsValid} FALSE PARENT_SCOPE)
    else()
        get_filename_component(filename "${module_header}" NAME)
        message(STATUS "Module header file found: ${filename}")
        set(${IsValid} TRUE PARENT_SCOPE)
        set(${module_header_file} ${module_header} PARENT_SCOPE)
    endif()
endfunction()

# =============================================================================

## @brief Collects buildable source files from a library source directory.
##
## @param source_path Library source directory to scan recursively.
## @param source_files Variable receiving the absolute source file paths.
## @param header_files Reserved output variable for header files.
## @details Files are filtered by module naming conventions and platform.
function(_evil_get_library_sources source_path source_files header_files)
    # Clear output lists before populating them.
    set(source_files)
    set(header_files)

    # Discover candidate source and header files recursively.
    file(GLOB_RECURSE potential_files RELATIVE ${source_path} ${source_path}/*.cpp ${source_path}/*.c ${source_path}/*.h)

    # Derive the module name used by the source-file filter.
    get_filename_component(library_name ${source_path} NAME)
    
    # Keep source files that follow the JUCE module naming convention.
    set(potential_source_files ${potential_files})
    list(FILTER potential_source_files INCLUDE REGEX "^${library_name}[^/]*\\.(c|cc|cpp|cxx|s|asm)$")

    foreach(source ${potential_files})
        if(source IN_LIST potential_source_files)
            _evil_should_build_library_file(${source} should_build)
            if(should_build)
                list(APPEND source_files ${source})
            endif() 
        endif()
    endforeach()

    #[[
    # Disabled: collect header files for a future PUBLIC_HEADER configuration.
    set(potential_header_files ${potential_files})
    
    # Remove source files so only headers remain.
    if(NOT "${source_files}" STREQUAL "")
        # Uncomment to log header filtering during debugging.
        list(REMOVE_ITEM potential_header_files ${source_files})
    endif()

    foreach(header ${potential_header_files})
        # Uncomment to log each discovered header during debugging.
        list(APPEND header_files ${header})
    endforeach()
    ]]

    list(TRANSFORM source_files PREPEND ${source_path}/)
    set(source_files ${source_files} PARENT_SCOPE)
endfunction()

# =============================================================================

## @brief Determines whether a source file applies to the current platform.
##
## @param filename Source file name to inspect.
## @param should_build Variable receiving TRUE when the file should be built.
## @details Recognized suffixes include _android, _ios, _linux, _mac, _osx,
## and _windows. A recognized suffix for another platform excludes the file.
function(_evil_should_build_library_file filename should_build)
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

    set("${should_build}" "${result}" PARENT_SCOPE)
endfunction()

# =============================================================================

## @brief Extracts a named metadata block from a module header.
##
## @param BLOCKID Metadata block identifier.
## @param file_with_block File containing the metadata block markers.
## @param metadata_dict Variable receiving key=[value1;value2] records.
## @throws FATAL_ERROR if file_with_block does not exist.
function(_evil_extract_metadata_block BLOCKID file_with_block metadata_dict)

    if(NOT EXISTS ${file_with_block})
        message(FATAL_ERROR "Unable to find file ${file_with_block}")
    else()
        get_filename_component(filename "${file_with_block}" NAME)
        message(STATUS "Extracting metadata block ${BLOCKID} from file: ${filename}")
    endif()
    
    increment_log_indent()

    # Clear parser state before reading the metadata block.
    set(metadata_dict)
    set(append NO)
    set(metadata_key)

    set(result_dict)
    # Read the file as a list of lines.
    file(STRINGS ${file_with_block} module_header_contents)


    foreach(line IN LISTS module_header_contents)
        # Uncomment to log every parsed line during debugging.
        if(NOT append)
            if(line MATCHES "[\t ]*BEGIN_${BLOCKID}[\t ]*")
                # Uncomment to log the start marker during debugging.
                increment_log_indent()
                set(append YES)
            endif()
            continue()
        endif()

        if(append AND (line MATCHES "[\t ]*END_${BLOCKID}[\t ]*"))
            decrement_log_indent()
            # Uncomment to log the end marker during debugging.
            break()
        endif()
        
        if(line MATCHES "^[\t ]*([a-zA-Z]+):")
            set(metadata_key "${CMAKE_MATCH_1}")
        endif()

        # Remove the key and split the remaining value into a list.
        string(REGEX REPLACE "^[\t ]*${metadata_key}:[\t ]*" "" line "${line}")
        string(REGEX REPLACE "[\t ,]+" ";" metadata_value "${line}")
        
        # Ignore empty values.
        if (metadata_value STREQUAL "")
            continue()
        endif()

        LIST(APPEND result_dict "${metadata_key}=[${metadata_value}]")
        set(metadata_dict [${result_dict}] PARENT_SCOPE)

    endforeach()
    decrement_log_indent()
endfunction()

# =============================================================================

## @brief Retrieves one value from a metadata dictionary.
##
## @param metadata_dict Metadata records in key=[value1;value2] format.
## @param key Metadata key to find.
## @param out_value Variable receiving the value associated with key.
## @details If the key is not found, out_value remains unchanged.
function(_evil_get_metadata_value metadata_dict key out_value)
    message(STATUS "Fetching metadata for key [${key}]")
    increment_log_indent()
 
    foreach(record ${metadata_dict})

        # Remove the outer brackets before splitting the record into pairs.
        string(REGEX REPLACE "^\\[(.*)\\]$" "\\1" record "${record}")
        foreach(pair ${record})
            # Split each pair into its key and value.
            string(REGEX MATCH "([^=]+)=(.*)" _ "${pair}")
            set(current_key "${CMAKE_MATCH_1}")
            set(current_value "${CMAKE_MATCH_2}")
          
            string(REGEX REPLACE "^\\[(.*)\\]$" "\\1" current_value "${current_value}")

            if(current_key STREQUAL key)
                message(STATUS "Found metadata value for key [${key}] Value: ${current_value}")
                set("${out_value}" "${current_value}" PARENT_SCOPE)
                decrement_log_indent()
                return()
            endif()    
        endforeach()
    endforeach()
    message(STATUS "Metadata for key [${key}] not found.")
    
    decrement_log_indent()
    message(STATUS "Finished configuring target: ${PROJECT_NAME}")
    message(STATUS "=================================================================================")
endfunction()

# =============================================================================