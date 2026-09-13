include_guard(GLOBAL)

##\file EvilAudioPrerequisiteUtils.cmake
##\brief Helper utilities for managing prerequisites of the EvilAudio project.

##\brief Ensures a Python virtual environment exists and upgrades core tooling.
##
##\details
##Creates a Python virtual environment at the provided path when it does not
##already exist. If it already exists, the environment is reused.
##
##After creation/reuse, the function upgrades core packaging tools inside the
##environment (`pip`, `setuptools`, `wheel`) to keep dependency installation
##behavior current and deterministic across developer machines.
##
##\param[in] venv_directory
##Absolute or relative path to the virtual environment directory.
##
##\param[in] PYTHON_EXECUTABLE
##Optional Python interpreter used to create the virtual environment.
##If omitted, `find_package(Python3 COMPONENTS Interpreter REQUIRED)` is used.
##
##\exception FATAL_ERROR
##Raised when virtual environment creation fails or when package tool upgrades fail.
##
##\note
##On Windows this function uses `<venv>/Scripts/python.exe`; on POSIX systems
##it uses `<venv>/bin/python`.
##
##\par Example
##\code{.cmake}
##evilaudio_install_or_upgrade_python_venv("${CMAKE_SOURCE_DIR}/.venv")
##
##find_package(Python3 COMPONENTS Interpreter REQUIRED)
##evilaudio_install_or_upgrade_python_venv(
##    "${CMAKE_SOURCE_DIR}/.venv"
##    PYTHON_EXECUTABLE "${Python3_EXECUTABLE}"
##)
##\endcode
function(evilaudio_install_or_upgrade_python_venv venv_directory)
	set(_ea_options)
	set(_ea_one_value_args PYTHON_EXECUTABLE)
	set(_ea_multi_value_args)
	cmake_parse_arguments(
		EA_PY_VENV
		"${_ea_options}"
		"${_ea_one_value_args}"
		"${_ea_multi_value_args}"
		${ARGN}
	)

	if(NOT EA_PY_VENV_PYTHON_EXECUTABLE)
		find_package(Python3 COMPONENTS Interpreter REQUIRED)
		set(_ea_python_executable "${Python3_EXECUTABLE}")
	else()
		set(_ea_python_executable "${EA_PY_VENV_PYTHON_EXECUTABLE}")
	endif()

	if(WIN32)
		set(_ea_venv_python "${venv_directory}/Scripts/python.exe")
	else()
		set(_ea_venv_python "${venv_directory}/bin/python")
	endif()

	if(NOT EXISTS "${_ea_venv_python}")
		message(STATUS "Creating Python virtual environment at: ${venv_directory}")
		execute_process(
			COMMAND "${_ea_python_executable}" -m venv "${venv_directory}"
			RESULT_VARIABLE _ea_create_result
			OUTPUT_VARIABLE _ea_create_stdout
			ERROR_VARIABLE _ea_create_stderr
			OUTPUT_STRIP_TRAILING_WHITESPACE
			ERROR_STRIP_TRAILING_WHITESPACE
		)

		if(NOT _ea_create_result EQUAL 0)
			message(FATAL_ERROR
				"Failed to create Python virtual environment at '${venv_directory}'.\n"
				"Python executable: ${_ea_python_executable}\n"
				"stdout:\n${_ea_create_stdout}\n"
				"stderr:\n${_ea_create_stderr}"
			)
		endif()

		execute_process(
			COMMAND "${_ea_venv_python}" -m pip install --upgrade pip setuptools wheel
			RESULT_VARIABLE _ea_upgrade_result
			OUTPUT_VARIABLE _ea_upgrade_stdout
			ERROR_VARIABLE _ea_upgrade_stderr
			OUTPUT_STRIP_TRAILING_WHITESPACE
			ERROR_STRIP_TRAILING_WHITESPACE
		)

		if(NOT _ea_upgrade_result EQUAL 0)
			message(FATAL_ERROR
				"Failed to upgrade Python packaging tools in '${venv_directory}'.\n"
				"Command: ${_ea_venv_python} -m pip install --upgrade pip setuptools wheel\n"
				"stdout:\n${_ea_upgrade_stdout}\n"
				"stderr:\n${_ea_upgrade_stderr}"
			)
		endif()

		message(STATUS "Python virtual environment is ready: ${venv_directory}")
	else()
		message(STATUS "Reusing existing Python virtual environment at: ${venv_directory}")
	endif()

	message(STATUS "Python virtual environment is ready: ${venv_directory}")
endfunction()

##\brief Ensures the \c hpp2plantuml Python package is installed in a virtual environment.
##
##\details
##Resolves the Python interpreter inside the provided virtual environment and
##uses it to check for the \c hpp2plantuml package. When the package is not
##present, it is installed with \c pip into that same environment.
##
##\param[in] venv_directory
##Absolute or relative path to the virtual environment directory.
##
##\param[in] QUIET
##Optional flag that suppresses status messages while checking and installing
##the package.
##
##\exception FATAL_ERROR
##Raised when the virtual environment interpreter cannot be resolved or when
##the package installation fails.
##
##\par Example
##\code{.cmake}
##evilaudio_install_or_upgrade_python_venv("${CMAKE_SOURCE_DIR}/.venv")
##evilaudio_ensure_hpp2plantuml_installed_in_venv("${CMAKE_SOURCE_DIR}/.venv")
##\endcode
function(evilaudio_ensure_hpp2plantuml_installed_in_venv venv_directory)
    set(_ea_options QUIET)
    set(_ea_one_value_args VERSION)
    set(_ea_multi_value_args)
    cmake_parse_arguments(
        EA_H2P_VENV
        "${_ea_options}"
        "${_ea_one_value_args}"
        "${_ea_multi_value_args}"
        ${ARGN}
    )

    evilaudio_resolve_venv_python("${venv_directory}" _ea_venv_python)

    if(EA_H2P_VENV_VERSION)
        set(_ea_hpp2plantuml_spec "hpp2plantuml==${EA_H2P_VENV_VERSION}")
    else()
        set(_ea_hpp2plantuml_spec "hpp2plantuml")
    endif()

    execute_process(
        COMMAND "${_ea_venv_python}" -m pip install "${_ea_hpp2plantuml_spec}"
        RESULT_VARIABLE _ea_install_result
        OUTPUT_VARIABLE _ea_install_stdout
        ERROR_VARIABLE _ea_install_stderr
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_STRIP_TRAILING_WHITESPACE
    )

    if(NOT _ea_install_result EQUAL 0)
        message(FATAL_ERROR
            "Failed to install hpp2plantuml in virtual environment '${venv_directory}'.\n"
            "Command: ${_ea_venv_python} -m pip install hpp2plantuml\n"
            "stdout:\n${_ea_install_stdout}\n"
            "stderr:\n${_ea_install_stderr}"
        )
    endif()

    if(NOT EA_H2P_VENV_QUIET)
        message(STATUS "hpp2plantuml installed successfully in: ${venv_directory}")
    endif()
endfunction()

##\brief Removes a Python virtual environment directory.
##
##\details
##Deletes the provided virtual environment directory recursively when it exists.
##If the directory is already absent, the function logs a status message and
##returns without failing.
##
##\param[in] venv_directory
##Absolute or relative path to the virtual environment directory to remove.
##
##\par Example
##\code{.cmake}
##evilaudio_uninstall_python_venv("${CMAKE_SOURCE_DIR}/.venv")
##\endcode
function(evilaudio_uninstall_python_venv venv_directory)
	if(EXISTS "${venv_directory}")
		message(STATUS "Removing Python virtual environment at: ${venv_directory}")
		file(REMOVE_RECURSE "${venv_directory}")
		message(STATUS "Python virtual environment removed: ${venv_directory}")
	else()
		message(STATUS "Python virtual environment not found, skipping removal: ${venv_directory}")
	endif()
endfunction()

##\brief Resolves the Python interpreter executable from a virtual environment.
##
##\details
##Derives the platform-correct path to the Python interpreter inside the given
##virtual environment directory and verifies that the file exists:
##- Windows: `<venv_directory>/Scripts/python.exe`
##- POSIX:   `<venv_directory>/bin/python`
##
##The resolved path is returned via the caller-supplied output variable.
##
##\param[in]  venv_directory   Absolute or relative path to the virtual environment.
##\param[out] python_executable Variable name to receive the resolved interpreter path.
##This variable is set in the parent scope.
##
##\exception FATAL_ERROR
##Raised if the expected Python interpreter does not exist inside the virtual
##environment, which typically means the venv has not been created yet.
##
##\par Example
##\code{.cmake}
##evilaudio_install_or_upgrade_python_venv("${CMAKE_SOURCE_DIR}/.venv")
##evilaudio_resolve_venv_python("${CMAKE_SOURCE_DIR}/.venv" VENV_PYTHON)
##message(STATUS "Using Python: ${VENV_PYTHON}")
##\endcode
function(evilaudio_resolve_venv_python venv_directory python_executable)
	if(WIN32)
		set(_ea_venv_python "${venv_directory}/Scripts/python.exe")
	else()
		set(_ea_venv_python "${venv_directory}/bin/python")
	endif()

	if(NOT EXISTS "${_ea_venv_python}")
		message(FATAL_ERROR
			"Python interpreter not found in virtual environment '${venv_directory}'.\n"
			"Expected: ${_ea_venv_python}\n"
			"The virtual environment may not have been created yet."
		)
	endif()
	set(${python_executable} "${_ea_venv_python}" PARENT_SCOPE)
endfunction()


