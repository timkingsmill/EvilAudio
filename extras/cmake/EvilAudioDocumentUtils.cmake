include_guard(GLOBAL)

# =================================================================================================

# Runs hpp2plantuml through the selected Python interpreter.
#
# Usage:
#   evil_audio_run_hpp2plantuml(
#       INPUT_GLOB "*.hpp"
#       OUTPUT_FILE "out.puml"
#       [WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"]
#       [PYTHON_EXECUTABLE "..."]
#       [VENV_PATH "..."]
#       [EXTRA_ARGS ...]
#       [FAIL_ON_ERROR]
#       [QUIET]
#   )
#
# Example:
#   evil_audio_run_hpp2plantuml(
#       INPUT_GLOB "source/libs/**/*.hpp"
#       OUTPUT_FILE "docs/puml/generated/headers.puml"
#       WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
#       VENV_PATH "${CMAKE_SOURCE_DIR}/.venv"
#   )

function(evil_audio_run_hpp2plantuml)
	set(options QUIET FAIL_ON_ERROR)
	set(oneValueArgs INPUT_GLOB OUTPUT_FILE WORKING_DIRECTORY PYTHON_EXECUTABLE VENV_PATH)
	set(multiValueArgs EXTRA_ARGS)
	cmake_parse_arguments(EA_RUN "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	if(NOT EA_RUN_INPUT_GLOB)
		message(FATAL_ERROR "evil_audio_run_hpp2plantuml(): INPUT_GLOB is required")
	endif()

	if(NOT EA_RUN_OUTPUT_FILE)
		message(FATAL_ERROR "evil_audio_run_hpp2plantuml(): OUTPUT_FILE is required")
	endif()

	set(_working_directory "${EA_RUN_WORKING_DIRECTORY}")
	if(NOT _working_directory)
		set(_working_directory "${CMAKE_SOURCE_DIR}")
	endif()

	_evil_audio_resolve_python_interpreter(
		_python
		_python_source
		PYTHON_EXECUTABLE "${EA_RUN_PYTHON_EXECUTABLE}"
		VENV_PATH "${EA_RUN_VENV_PATH}"
		CALLER "evil_audio_run_hpp2plantuml"
	)

	evil_audio_ensure_hpp2plantuml_installed(
		PYTHON_EXECUTABLE "${_python}"
		VENV_PATH "${EA_RUN_VENV_PATH}"
		QUIET
	)

	if(NOT EA_RUN_QUIET)
		message(STATUS "Running hpp2plantuml with ${_python} (${_python_source})")
	endif()

	execute_process(
		COMMAND ${CMAKE_COMMAND} -E env PYTHONUTF8=1 PYTHONIOENCODING=UTF-8 "${_python}" -m hpp2plantuml.hpp2plantuml -i "${EA_RUN_INPUT_GLOB}" -o "${EA_RUN_OUTPUT_FILE}" ${EA_RUN_EXTRA_ARGS}
		WORKING_DIRECTORY "${_working_directory}"
		RESULT_VARIABLE _hpp2plantuml_result
		OUTPUT_VARIABLE _hpp2plantuml_stdout
		ERROR_VARIABLE _hpp2plantuml_stderr
		OUTPUT_STRIP_TRAILING_WHITESPACE
		ERROR_STRIP_TRAILING_WHITESPACE
	)

	if(NOT _hpp2plantuml_result EQUAL 0)
		set(_hpp2plantuml_error
			"hpp2plantuml execution failed using interpreter: ${_python}\n"
			"stdout:\n${_hpp2plantuml_stdout}\n"
			"stderr:\n${_hpp2plantuml_stderr}")

		if(EA_RUN_FAIL_ON_ERROR)
			message(FATAL_ERROR "${_hpp2plantuml_error}")
		else()
			message(STATUS "hpp2plantuml skipped due to parser/runtime issue")
			return()
		endif()
	endif()

	if(NOT EA_RUN_QUIET)
		message(STATUS "Generated PlantUML: ${EA_RUN_OUTPUT_FILE}")
	endif()
endfunction()

# =================================================================================================

# Resolves a Python interpreter, preferring virtual environments when available.
function(_evil_audio_resolve_python_interpreter out_python out_python_source)
	set(options)
	set(oneValueArgs PYTHON_EXECUTABLE VENV_PATH CALLER)
	cmake_parse_arguments(EA_PY "${options}" "${oneValueArgs}" "" ${ARGN})

	set(_python "${EA_PY_PYTHON_EXECUTABLE}")
	set(_python_source "PYTHON_EXECUTABLE argument")

	if(NOT _python)
		set(_venv_root "")
		if(EA_PY_VENV_PATH)
			set(_venv_root "${EA_PY_VENV_PATH}")
		elseif(DEFINED ENV{VIRTUAL_ENV} AND NOT "$ENV{VIRTUAL_ENV}" STREQUAL "")
			set(_venv_root "$ENV{VIRTUAL_ENV}")
		elseif(EXISTS "${CMAKE_SOURCE_DIR}/.venv")
			set(_venv_root "${CMAKE_SOURCE_DIR}/.venv")
		endif()

		if(_venv_root)
			if(WIN32)
				if(EXISTS "${_venv_root}/Scripts/python.exe")
					set(_python "${_venv_root}/Scripts/python.exe")
				endif()
			else()
				if(EXISTS "${_venv_root}/bin/python3")
					set(_python "${_venv_root}/bin/python3")
				elseif(EXISTS "${_venv_root}/bin/python")
					set(_python "${_venv_root}/bin/python")
				endif()
			endif()

			if(_python)
				set(_python_source "virtual environment")
			endif()
		endif()
	endif()

	if(NOT _python)
		find_package(Python3 COMPONENTS Interpreter QUIET)
		if(Python3_Interpreter_FOUND)
			set(_python "${Python3_EXECUTABLE}")
			set(_python_source "find_package(Python3)")
		endif()
	endif()

	if(NOT _python)
		set(_caller "_evil_audio_resolve_python_interpreter")
		if(EA_PY_CALLER)
			set(_caller "${EA_PY_CALLER}")
		endif()

		message(FATAL_ERROR
			"${_caller}(): Python interpreter not found. "
			"Install Python 3, activate a venv, or pass PYTHON_EXECUTABLE.")
	endif()

	set(${out_python} "${_python}" PARENT_SCOPE)
	set(${out_python_source} "${_python_source}" PARENT_SCOPE)
endfunction()

# =================================================================================================

# Ensures the Python package hpp2plantuml is installed.
#
# Usage:
#   evil_audio_ensure_hpp2plantuml_installed()
#   evil_audio_ensure_hpp2plantuml_installed(PYTHON_EXECUTABLE "C:/path/to/python.exe")
#
function(evil_audio_ensure_hpp2plantuml_installed)
	set(options QUIET)
	set(oneValueArgs PYTHON_EXECUTABLE VENV_PATH)
	cmake_parse_arguments(EA_H2P "${options}" "${oneValueArgs}" "" ${ARGN})

	if(EA_H2P_VENV_PATH)
		if(NOT COMMAND evilaudio_ensure_hpp2plantuml_installed_in_venv)
			message(FATAL_ERROR
				"evil_audio_ensure_hpp2plantuml_installed(): VENV_PATH requires "
				"evilaudio_ensure_hpp2plantuml_installed_in_venv() from EvilAudioPrerequisiteUtils.cmake")
		endif()

		if(EA_H2P_QUIET)
			evilaudio_ensure_hpp2plantuml_installed_in_venv("${EA_H2P_VENV_PATH}" QUIET)
		else()
			evilaudio_ensure_hpp2plantuml_installed_in_venv("${EA_H2P_VENV_PATH}")
		endif()
		return()
	endif()

	_evil_audio_resolve_python_interpreter(
		_python
		_python_source
		PYTHON_EXECUTABLE "${EA_H2P_PYTHON_EXECUTABLE}"
		VENV_PATH "${EA_H2P_VENV_PATH}"
		CALLER "evil_audio_ensure_hpp2plantuml_installed"
	)

	execute_process(
		COMMAND "${_python}" -m pip show hpp2plantuml
		RESULT_VARIABLE _hpp2plantuml_installed
		OUTPUT_QUIET
		ERROR_QUIET
	)

	if(_hpp2plantuml_installed EQUAL 0)
		if(NOT EA_H2P_QUIET)
			message(STATUS "hpp2plantuml is already installed")
		endif()
		return()
	endif()

	if(NOT EA_H2P_QUIET)
		message(STATUS "Installing hpp2plantuml with ${_python} -m pip (${_python_source})")
	endif()

	execute_process(
		COMMAND "${_python}" -m pip install hpp2plantuml
		RESULT_VARIABLE _install_result
		OUTPUT_VARIABLE _install_stdout
		ERROR_VARIABLE _install_stderr
		OUTPUT_STRIP_TRAILING_WHITESPACE
		ERROR_STRIP_TRAILING_WHITESPACE
	)

	if(NOT _install_result EQUAL 0)
		message(FATAL_ERROR
			"Failed to install hpp2plantuml using interpreter: ${_python}\n"
			"pip install output:\n${_install_stdout}\n${_install_stderr}")
	endif()

	if(NOT EA_H2P_QUIET)
		message(STATUS "hpp2plantuml installed successfully")
	endif()
endfunction()

# Backward-compatible alias for older internal call sites.
function(_evil_audio_ensure_hpp2plantuml_installed)
	evil_audio_ensure_hpp2plantuml_installed(${ARGN})
endfunction()

# =================================================================================================

function(evil_audio_get_all_puml_files_in_directory out_files)
    set(options)
    set(oneValueArgs)
    set(multiValueArgs SOURCE_DIR_LIST)
    cmake_parse_arguments(EA_PUML "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT EA_PUML_SOURCE_DIR_LIST)
        message(FATAL_ERROR
            "evil_audio_get_all_puml_files_in_directory: SOURCE_DIR_LIST is required")
    endif()

    set(_all_puml_files)
    foreach(_source IN LISTS EA_PUML_SOURCE_DIR_LIST)
        if(NOT IS_DIRECTORY "${_source}")
            message(FATAL_ERROR
                "evil_audio_get_all_puml_files_in_directory: ${_source} is not a valid directory")
        endif()

        file(GLOB_RECURSE _puml_files "${_source}/*.puml")
        list(APPEND _all_puml_files ${_puml_files})
    endforeach()

    set(${out_files} "${_all_puml_files}" PARENT_SCOPE)
endfunction()

# =================================================================================================

# Ensures .vscode/settings.json contains the PlantUML include path used by local preview.
#
# Usage:
#   evil_audio_add_vscode_plantuml_include()
#   evil_audio_add_vscode_plantuml_include(
#       INCLUDE_PATH "${CMAKE_SOURCE_DIR}/docs/puml/themes"
#       SETTINGS_FILE "${CMAKE_SOURCE_DIR}/.vscode/settings.json"
#       [PYTHON_EXECUTABLE "..."]
#       [VENV_PATH "..."]
#       [QUIET]
#   )
function(evil_audio_add_vscode_plantuml_include)
	set(options QUIET)
	set(oneValueArgs INCLUDE_PATH SETTINGS_FILE PYTHON_EXECUTABLE VENV_PATH)
	cmake_parse_arguments(EA_VSCODE "${options}" "${oneValueArgs}" "" ${ARGN})

	set(_settings_file "${EA_VSCODE_SETTINGS_FILE}")
	if(NOT _settings_file)
		set(_settings_file "${CMAKE_SOURCE_DIR}/.vscode/settings.json")
	endif()

	set(_include_path "${EA_VSCODE_INCLUDE_PATH}")
	if(NOT _include_path)
		set(_include_path "${CMAKE_SOURCE_DIR}/docs/puml")
	endif()

	if(IS_ABSOLUTE "${_include_path}")
		file(RELATIVE_PATH _relative_include_path "${CMAKE_SOURCE_DIR}" "${_include_path}")
		if(NOT _relative_include_path MATCHES "^\\.\\.")
			set(_include_path "${_relative_include_path}")
		endif()
	endif()
	string(REPLACE "\\" "/" _include_path "${_include_path}")

	get_filename_component(_settings_dir "${_settings_file}" DIRECTORY)
	if(NOT IS_DIRECTORY "${_settings_dir}")
		file(MAKE_DIRECTORY "${_settings_dir}")
	endif()

	_evil_audio_resolve_python_interpreter(
		_python
		_python_source
		PYTHON_EXECUTABLE "${EA_VSCODE_PYTHON_EXECUTABLE}"
		VENV_PATH "${EA_VSCODE_VENV_PATH}"
		CALLER "evil_audio_add_vscode_plantuml_include"
	)

	set(_update_script [=[
import json
import pathlib
import sys

settings_file = pathlib.Path(sys.argv[1])
include_path = sys.argv[2].replace("\\", "/")

if settings_file.exists():
    raw = settings_file.read_text(encoding="utf-8").strip()
    data = json.loads(raw) if raw else {}
else:
    data = {}

if not isinstance(data, dict):
    raise SystemExit("settings.json root must be a JSON object")

def ensure_list_setting(key: str) -> list:
	value = data.get(key)
	if value is None:
		return []
	if not isinstance(value, list):
		raise SystemExit(f"{key} must be an array")
	return value

includes = ensure_list_setting("plantuml.includes")
includepaths = ensure_list_setting("plantuml.includepaths")

added_includes = False
if include_path not in includes:
	includes.append(include_path)
	added_includes = True

added_includepaths = False
if include_path not in includepaths:
	includepaths.append(include_path)
	added_includepaths = True

data["plantuml.includes"] = includes
data["plantuml.includepaths"] = includepaths

settings_file.parent.mkdir(parents=True, exist_ok=True)
settings_file.write_text(json.dumps(data, indent=4, ensure_ascii=True) + "\n", encoding="utf-8")
if added_includes and added_includepaths:
	print("added-both")
elif added_includes:
	print("added-includes")
elif added_includepaths:
	print("added-includepaths")
else:
	print("exists")
]=])

	execute_process(
		COMMAND "${_python}" -c "${_update_script}" "${_settings_file}" "${_include_path}"
		RESULT_VARIABLE _settings_update_result
		OUTPUT_VARIABLE _settings_update_stdout
		ERROR_VARIABLE _settings_update_stderr
		OUTPUT_STRIP_TRAILING_WHITESPACE
		ERROR_STRIP_TRAILING_WHITESPACE
	)

	if(NOT _settings_update_result EQUAL 0)
		message(FATAL_ERROR
			"evil_audio_add_vscode_plantuml_include(): failed to update ${_settings_file}\n"
			"Python: ${_python} (${_python_source})\n"
			"stderr:\n${_settings_update_stderr}")
	endif()

	if(NOT EA_VSCODE_QUIET)
		if(_settings_update_stdout STREQUAL "added-both")
			message(STATUS "Added PlantUML include path to VS Code settings keys plantuml.includes and plantuml.includepaths: ${_include_path}")
		elseif(_settings_update_stdout STREQUAL "added-includes")
			message(STATUS "Added PlantUML include path to VS Code settings key plantuml.includes: ${_include_path}")
		elseif(_settings_update_stdout STREQUAL "added-includepaths")
			message(STATUS "Added PlantUML include path to VS Code settings key plantuml.includepaths: ${_include_path}")
		else()
			message(STATUS "PlantUML include path already present in VS Code settings keys plantuml.includes and plantuml.includepaths: ${_include_path}")
		endif()
	endif()
endfunction()


