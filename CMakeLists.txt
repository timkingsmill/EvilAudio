# ------------------------------------------------------------------------------------------------

# Minimum required version of CMake
cmake_minimum_required(VERSION 4.2.0)

# Prevent in-source builds to keep source directory clean
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "In-source builds not allowed. Please create a separate build directory and run CMake from there.")
endif()

# ------------------------------------------------------------------------------------------------
# Include CMake modules and custom utilities
include(CMakeDependentOption)
include(CMakePrintHelpers)

# ------------------------------------------------------------------------------------------------

set(EVILAUDIO_CMAKE_MODULE_DIR "${CMAKE_SOURCE_DIR}/extras/cmake")
# Load the command dispatcher before project() so configure-time commands can
# run before the rest of the project is evaluated.
include("${EVILAUDIO_CMAKE_MODULE_DIR}/EvilAudioCommandHandlers.cmake")

# ------------------------------------------------------------------------------------------------

# Suppress third-party deprecation warnings emitted during dependency configure.
set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "Disable CMake deprecation warnings" FORCE)
set(CMAKE_ERROR_DEPRECATED OFF CACHE BOOL "Do not elevate deprecations to errors" FORCE)

# JUCE options
option(JUCE_ENABLE_MODULE_SOURCE_GROUPS "Enable Module Source Groups" OFF)

# Options to add documentation files to Visual Studio Solution Items
option(EVILAUDIO_ADD_API_DOCS_TO_SOLUTION_ITEMS
    "Add docs/evil_audio_api files to Visual Studio Solution Items during generation" ON)

option(EVILAUDIO_ADD_TODO_DOCS_TO_SOLUTION_ITEMS
    "Add docs/todo files to Visual Studio Solution Items during generation" ON)

option(EVILAUDIO_DOXYGEN_EXCLUDE_ASIO "Exclude ASIO sources from Doxygen" ON)

# Option to enable PlantUML CMake target graph generation
option(EVILAUDIO_ENABLE_CMAKE_TARGET_GRAPH_TARGET
    "Enable generation of docs/tools/plantUML/cmake-targets.puml via docs/tools/plantUML/generate-cmake-targets.ps1" ON)

# MSVC multiprocessor compilation options
option(EVILAUDIO_MSVC_ENABLE_MULTIPROCESSOR
    "Enable MSVC multiprocessor compilation (/MP) for Visual Studio builds" ON)

# =================================================================================
# Optional configure-time command dispatcher.
# Example: cmake -S . -B build -DEVIL_AUDIO_COMMAND=configure
# Read the cache value once, then remove it so the command does not run again
# on a later reconfigure unless the user explicitly requests it.

set(EVIL_AUDIO_COMMAND "none")
if(DEFINED CACHE{EVIL_AUDIO_COMMAND})
    set(EVIL_AUDIO_COMMAND "$CACHE{EVIL_AUDIO_COMMAND}")
    unset(EVIL_AUDIO_COMMAND CACHE)
endif()
evilaudio_handle_configure_time_command("${EVIL_AUDIO_COMMAND}")

# =================================================================================

# Establish the project and the language/toolchain settings shared by all
# targets created by the subdirectories below.
# Project definition
project(EvilAudio VERSION 0.1.0 LANGUAGES C CXX)

# Use folders in IDEs for better organization
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
# Set C++23 as the default for all targets
set(CMAKE_CXX_STANDARD 23)
# Ensure the compiler supports the specified standard
set(CMAKE_CXX_STANDARD_REQUIRED ON) 
# Disable compiler-specific extensions by default
set(CMAKE_CXX_EXTENSIONS OFF) 

# Set the default build type to Debug if not specified by the user
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build (Debug, Release, RelWithDebInfo, MinSizeRel)" FORCE)
endif()

# ------------------------------------------------------------------------------------------------

enable_testing()

# ------------------------------------------------------------------------------------------------

# These modules provide the project-specific functions used during
# configuration: logging, dependency detection, test setup, documentation,
# and general CMake utilities.


include("${EVILAUDIO_CMAKE_MODULE_DIR}/EvilAudioLogging.cmake")
include("${EVILAUDIO_CMAKE_MODULE_DIR}/EvilAudioCMakeUtils.cmake")
include("${EVILAUDIO_CMAKE_MODULE_DIR}/EvilAudioUtils.cmake")
include("${EVILAUDIO_CMAKE_MODULE_DIR}/EvilAudioASIO.cmake")
include("${EVILAUDIO_CMAKE_MODULE_DIR}/EvilAudioTestSettings.cmake")
set(_doc_utils "${EVILAUDIO_CMAKE_MODULE_DIR}/EvilAudioDocumentUtils.cmake")
if(NOT EXISTS "${_doc_utils}")
    message(FATAL_ERROR "Missing required module: ${_doc_utils}")
endif()
include("${_doc_utils}")

include("${EVILAUDIO_CMAKE_MODULE_DIR}/EvilAudioDocumentUtils.cmake")
include("${EVILAUDIO_CMAKE_MODULE_DIR}/EvilAudioPrerequisiteUtils.cmake")

evil_audio_add_vscode_plantuml_include(
    INCLUDE_PATH "${CMAKE_SOURCE_DIR}/docs/puml/themes"
    SETTINGS_FILE "${CMAKE_SOURCE_DIR}/.vscode/settings.json"
    VENV_PATH "${CMAKE_SOURCE_DIR}/.venv"
)

# Documentation generation uses a project-local Python environment so its
# tools and versions are reproducible across developer machines.
# ------------------------------------------------------------------------------------------------
# Install or upgrade the Python virtual environment used for 
# documentation generation and other Python-based tooling.
evilaudio_install_or_upgrade_python_venv("${CMAKE_SOURCE_DIR}/.venv")
evilaudio_ensure_hpp2plantuml_installed_in_venv("${CMAKE_SOURCE_DIR}/.venv")

evil_audio_add_runsettings_to_solution_items("${CMAKE_SOURCE_DIR}/settings/EvilAudio.runsettings")

# ------------------------------------------------------------------------------------------------

message(STATUS "EvilAudio CMake configuration searching for PUML files:")
increment_log_indent()

evil_audio_get_all_puml_files_in_directory(ALL_PUML_FILES
    SOURCE_DIR_LIST
        "${CMAKE_SOURCE_DIR}/source"
        "${CMAKE_SOURCE_DIR}/docs/puml"
)

foreach(_puml_file IN LISTS ALL_PUML_FILES)
    message(STATUS "Found PUML file: ${_puml_file}")
endforeach()

decrement_log_indent()

# ------------------------------------------------------------------------------------------------

# Configure compiler-specific options and discover optional audio SDKs before
# defining targets that may depend on them.
set(EVILAUDIO_MSVC_MAX_PARALLEL_COMPILE 2 CACHE STRING
    "Optional processor cap for MSVC /MP<n>. Leave empty to let MSVC choose automatically")
# Path to JUCE examples
set(JUCE_EXAMPLES_DIR "${CMAKE_SOURCE_DIR}/external/JUCE/examples" CACHE PATH "Path to the JUCE examples directory" )

# Enable multiprocessor compilation for MSVC if requested
evil_audio_set_msvc_multiprocessor_options()
evil_detect_asio_sdk()

# ------------------------------------------------------------------------------------------------
# Doxygen documentation options and exclude paths
# TIM. Don't like how this is currently structured, 
# but it needs to be defined before the main Doxygen CMakeLists.txt is included. Will refactor later.
#evil_audio_configure_doxygen_exclude_paths()

# ------------------------------------------------------------------------------------------------
# Process dependencies first, then project targets, and finally documentation
# targets. Each directory adds its own libraries, applications, tests, or docs.
add_subdirectory(external)
add_subdirectory(source)

add_subdirectory(docs)

# Add documentation and todo files to Visual Studio Solution Items (Windows/Visual Studio only)
#evil_audio_add_docs_to_solution_items()

# ------------------------------------------------------------------------------------------------

# Set EvilDAW as the startup project in Visual Studio (after all targets are defined)
if (TARGET EvilDAW)
    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT EvilDAW)
endif()

# ------------------------------------------------------------------------------------------------


# ------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------
