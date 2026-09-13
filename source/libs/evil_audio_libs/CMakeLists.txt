include_guard(GLOBAL)

cmake_minimum_required(VERSION 4.2.0)

project(EvilAudioStaticLibs VERSION 0.1.0 LANGUAGES C CXX)

set(libraries_folder "EvilAudio/libs/evil_audio_libs") 

add_subdirectory(evil_core_lib)
set_target_properties(evil_core_lib PROPERTIES
	FOLDER "${libraries_folder}"
)

add_subdirectory(evil_plugins_lib)
set_target_properties(evil_plugins_lib PROPERTIES
	FOLDER "${libraries_folder}"
)

add_subdirectory(evil_eq_editors_lib)
set_target_properties(evil_eq_editors_lib PROPERTIES
	FOLDER "${libraries_folder}"
)

add_subdirectory(evil_patterns_lib)
set_target_properties(evil_patterns_lib PROPERTIES
	FOLDER "${libraries_folder}"
)
