include_guard(GLOBAL)

add_subdirectory(juce_static_libs)
add_subdirectory(evil_audio_libs)

evil_audio_setup_google_test()

add_subdirectory(EvilDAWLib/evil_daw_lib)
