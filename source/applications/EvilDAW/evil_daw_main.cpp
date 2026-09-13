/**
 * @file evil_daw_main.cpp
 * @brief Application entry point for EvilDAW.
 *
 * This file defines the JUCE application startup using the
 * START_JUCE_APPLICATION macro, which generates the program's main()
 * function and launches `evil::EvilDAWApplication`.
 */
#include <evil_daw_lib.h>

// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (evil::EvilDAWApplication)


