include_guard(GLOBAL)

# =============================================================================

set(EVIL_STATIC_LIBS_BLACKLIST
    "juce_audio_plugin_client"
)

# =============================================================================
# Module path <source_path> is the full path to the module directory 
# (e.g. /path/to/JUCE/modules/juce_core). 
# The module header file is expected to be in the format 
# "juce_<module>.h" (e.g. juce_core.h) and expected to be located in the 
# module directory. 
#
function(evil_add_static_lib source_path)
    # Validate the source path and check for the module header file    
    _evil_validate_library_sources_path(${source_path} IsValid module_header_file)
    if(NOT IsValid)
        message(FATAL_ERROR "Invalid source path for static library: ${module_header_file}")
    endif()

    get_filename_component(module_name ${source_path} NAME)
    message(STATUS "Adding static library for JUCE module: ${module_name}")

    # Derive the library target name from the module header file name.
    # Note: The library target name is derived from the module header file name, 
    # which is expected to be in the format "juce_<module>.h". The "juce" prefix 
    # is replaced with "evil" to create a unique target name for our static library. 
    # This allows us to link against the JUCE modules without installing JUCE itself, and it 
    # also helps to avoid naming conflicts with any existing JUCE targets if JUCE were to be 
    # included as a submodule in the future.
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
    
    #target_sources(${lib_target_name} PRIVATE ${header_files})
    target_sources(${lib_target_name} 
        PRIVATE 
            ${source_files} 
        PUBLIC
            ${module_header_file})

    # Set the include directories for this target.
    # Note: The `include` directory is added as a private include directory, 
    # which means that it will only be used when compiling the source files of this target. 
    # This is a common practice to keep the public interface of the library clean and avoid
    # exposing internal implementation details to other targets that might link against this library.

    
    # The include path is set to the module directory itself, which allows 
    # the source files of this module to include the headers using the correct path.
    get_filename_component(_all_libraries_folder_name ${source_path} DIRECTORY)  

    target_include_directories(${lib_target_name}      
        PUBLIC
            ${_all_libraries_folder_name}
        PUBLIC
            ${source_path}
    )
    
    # Get the list of public header files for this library target. This is used to 
    # set the PUBLIC_HEADER property of the target, which allows CMake to know which header files 
    # are part of the public interface of the library. This is important for generating export 
    # headers and for other targets that might link against this library to know which headers they can include.

    #[[
        if(NOT "${header_files}" STREQUAL "")
            message(STATUS "Setting public headers for target ${lib_target_name}: ${header_files}")
            set_target_properties(${lib_target_name} PROPERTIES PUBLIC_HEADER "${header_files}")
        endif()
        ]]

    # Define preprocessor definitions for this target.
    target_compile_definitions(${lib_target_name}
        PUBLIC 
            JUCE_MODULE_AVAILABLE_${module_name}=1
    
        PUBLIC
            JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=0
            DONT_SET_USING_JUCE_NAMESPACE=1
            # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
            JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
            JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
    )

    # Set platform-specific compile options and definitions based on the module name and the current system.
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
                    # Note: The ASIO SDK is expected to be located in 
                    # the "source/libs/asiosdk/common" directory.
                    message(STATUS "Enabling ASIO support for ${lib_target_name}")
                    message(STATUS "ASIO_SDK_DIR is set to: ${ASIO_SDK_DIR}")

                    # Should this be public or private? If it's public, then any target 
                    #that links against this library will also get these definitions 
                    # and include directories, which might be necessary if they need to 
                    # use ASIO functionality directly. If it's private, then only the 
                    # source files of this library will have access to these definitions and include directories.
                    
                    target_compile_definitions(${lib_target_name} PUBLIC JUCE_ASIO=1)
                    target_include_directories(${lib_target_name} PUBLIC "${ASIO_SDK_DIR}")
                endif()
            endif()

            decrement_log_indent()
        endif()
    endif()

    # Get list of dependencies for this library target. This is used to set the target_link_libraries for 
    # this target, which allows CMake to know which other targets this library depends on. 
    # This is important for ensuring that the correct libraries are linked when building the application that uses this library.
    set(metadata_dict)
    _evil_extract_metadata_block(JUCE_MODULE_DECLARATION "${module_header_file}" metadata_dict)
    _evil_get_metadata_value("${metadata_dict}" "dependencies" dependencies)

    foreach(dependency ${dependencies})
        string(TOLOWER "${dependency}" dependency_lowercase)
        set(dependency_target_name ${dependency_lowercase}_lib)
        message(STATUS "Adding link library dependency: ${dependency_target_name}")
        target_link_libraries(${lib_target_name} PRIVATE ${dependency_target_name})
        # Create the include directory for this dependency and add it to the include directories for this target. 
        # This allows the source files of this target to include the headers of the dependency using the correct path.
        get_filename_component(_modules_path ${source_path} DIRECTORY)  
        set(dependency_include_dir ${_modules_path}/${dependency})
        message(STATUS "Adding include directory for dependency ${dependency_target_name}: ${dependency_include_dir}")
        target_include_directories(${lib_target_name} PUBLIC ${dependency_include_dir})
    endforeach()

    # Print the properties of the target for debugging purposes. 
    # This is useful to verify that the target has been configured correctly 
    # with the expected sources, include directories, compile definitions, and linked libraries.

    #print_target_properties(${lib_target_name})
    
    reset_log_indent()
    message(STATUS "Finished adding static library target: ${lib_target_name}")
    message(STATUS "=================================================================================")

endfunction()

# =============================================================================

# This function validates the provided source path for a library and checks for 
# the existence of the module header file.
# It checks if the path exists and is a directory, and then looks for a 
# header file that matches the expected naming convention (e.g., "juce_<module>.h"). 
# If the path is valid and the module header file is found, it sets the output 
# variables accordingly; otherwise, it reports errors or warnings.
function(_evil_validate_library_sources_path source_path IsValid module_header_file)
    if(NOT EXISTS ${source_path})
        message(FATAL_ERROR "Directory does not exist: ${source_path}")
        set(${IsValid} FALSE PARENT_SCOPE)
    elseif(NOT IS_DIRECTORY ${source_path})
        message(FATAL_ERROR "Path is not a directory: ${source_path}")
        set(${IsValid} FALSE PARENT_SCOPE)
    else()
        #message(STATUS "Path is a valid directory: ${source_path}")
        set(${IsValid} TRUE PARENT_SCOPE)
    endif()

    # Get the directory name from source path
    get_filename_component(module_name ${source_path} NAME)  
    set(module_header ${source_path}/${module_name}.h)

    # Check for the existence of the module header file
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

function(_evil_get_library_sources source_path source_files header_files)
    # Clear the source files and header lists before populating them 
    # to ensure we don't have duplicates from previous calls.
    set(source_files)
    set(header_files)

    # Use GLOB_RECURSE to find all potential source and header files in the directory.
    file(GLOB_RECURSE potential_files RELATIVE ${source_path} ${source_path}/*.cpp ${source_path}/*.c ${source_path}/*.h)

    # Get the library name from source path to use for filtering source files.
    get_filename_component(library_name ${source_path} NAME)  
    
    # Filter the potential source files to include only those that match the expected 
    # naming convention for source files in JUCE modules.
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
    # The remaining potential files that were not classified as 
    # source files are considered potential header files.
    set(potential_header_files ${potential_files})
    
    # Remove any source files from the potential header files list to 
    # ensure we only have header files in that list.
    if(NOT "${source_files}" STREQUAL "")
        #message (STATUS "Filtering header files for library: ${library_name}")
        list(REMOVE_ITEM potential_header_files ${source_files})
    endif()

    foreach(header ${potential_header_files})
        #message(STATUS "Found header file: ${header}")
        list(APPEND header_files ${header})
    endforeach()
    ]]

    list(TRANSFORM source_files PREPEND ${source_path}/)
    set(source_files ${source_files} PARENT_SCOPE)
endfunction()

# =============================================================================

# This function checks if a given source file should be included in the library build 
# based on its filename and the current system.
# The function looks for specific suffixes in the filename (e.g., "_android", "_ios", etc.) that 
# indicate the file is intended for a specific platform. If the suffix matches the current system, 
# the file is included; otherwise, it is excluded from the build.
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

function(_evil_extract_metadata_block BLOCKID file_with_block metadata_dict)

    if(NOT EXISTS ${file_with_block})
        message(FATAL_ERROR "Unable to find file ${file_with_block}")
    else()
        get_filename_component(filename "${file_with_block}" NAME)
        message(STATUS "Extracting metadata block ${BLOCKID} from file: ${filename}")
    endif()
    
    increment_log_indent()

    # Clear the metadata dictionary list before populating it 
    # to ensure we don't have duplicates from previous calls.
    set(metadata_dict)
    set(append NO)
    set(metadata_key)

    set(result_dict)
    # Read the file contents into a list of lines
    file(STRINGS ${file_with_block} module_header_contents)


    foreach(line IN LISTS module_header_contents)
        #message(STATUS "Read line: ${line}")
        if(NOT append)
            if(line MATCHES "[\t ]*BEGIN_${BLOCKID}[\t ]*")
                #message(STATUS "Found BEGIN_${BLOCKID}")
                increment_log_indent()
                set(append YES)
            endif()
            continue()
        endif()

        if(append AND (line MATCHES "[\t ]*END_${BLOCKID}[\t ]*"))
            decrement_log_indent()
            #message(STATUS "Found END_${BLOCKID}")
            break()
        endif()
        
        if(line MATCHES "^[\t ]*([a-zA-Z]+):")
            #message(STATUS "Found metadata key: ${CMAKE_MATCH_1}")
            set(metadata_key "${CMAKE_MATCH_1}")
        endif()

        # Remove the key and any leading whitespace from the line to get the value, 
        # and then split the value by commas into a list.
        string(REGEX REPLACE "^[\t ]*${metadata_key}:[\t ]*" "" line "${line}")
        string(REGEX REPLACE "[\t ,]+" ";" metadata_value "${line}")
        
        # Skip empty values to avoid adding empty entries to the metadata dictionary.
        if (metadata_value STREQUAL "")
            continue()
        endif()

        #message(STATUS "Extracted metadata - Key: [${metadata_key}] Value: [${metadata_value}]")

        LIST(APPEND result_dict "${metadata_key}=[${metadata_value}]")
        set(metadata_dict [${result_dict}] PARENT_SCOPE)

    endforeach()
    decrement_log_indent()
endfunction()

# =============================================================================

# This function retrieves a specific metadata value from the metadata 
# dictionary list based on the provided key.
# The metadata dictionary is expected to be a list of strings 
# in the format "key=[value1;value2;...]".
function(_evil_get_metadata_value metadata_dict key out_value)
    message(STATUS "Fetching metadata for key [${key}]")
    increment_log_indent()
 
    foreach(record ${metadata_dict})

        # Remove the square brackets from the record to simplify parsing. 
        # The record is expected to be in the format "key=[value1;value2;...]". 
        # By removing the square brackets, we can easily split the string 
        # into key and value parts using a regular expression.
        string(REGEX REPLACE "^\\[(.*)\\]$" "\\1" record "${record}")
        foreach(pair ${record})
            # Use a regular expression to split the record into key and value parts. 
            # The regular expression looks for a pattern where there is a key followed 
            # by an equals sign and then a value. 
            # The key is captured in the first group (CMAKE_MATCH_1) and the 
            # value is captured in the second group (CMAKE_MATCH_2).
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