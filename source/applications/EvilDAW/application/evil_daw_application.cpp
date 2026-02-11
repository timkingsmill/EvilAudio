#include "evil_daw_application.h"
#include "main_window/evil_daw_main_window.h"


// Implemented in juce_core_CompilationTime.cpp
extern const char* evil_daw_compilationDate;
extern const char* evil_daw_compilationTime;

namespace evil
{
    const juce::String EvilDAWApplication::getApplicationName() { return "ProjectInfo::projectName"; }
    const juce::String EvilDAWApplication::getApplicationVersion() { return "ProjectInfo::versionString"; };

    bool EvilDAWApplication::moreThanOneInstanceAllowed() { return false; }

    void EvilDAWApplication::initialise(const juce::String& commandLine) {

        //auto compilationDateTime = juce::String(evil_daw_compilationDate) + " " + juce::String(evil_daw_compilationTime);   
        // This method is where you should put your application's initialisation code..
        juce::ignoreUnused(commandLine);
        _mainWindow.reset(new evil::EvilDAWMainWindow(getApplicationName(), *this));
    };

    void EvilDAWApplication::shutdown() {
        // Application's shutdown code here..
    };

    void EvilDAWApplication::systemRequestedQuit() {
        // This is called when the app is being asked to quit: you can ignore this
        // request and let the app carry on running, or call quit() to allow the app to close.
        quit();
    };

    void EvilDAWApplication::anotherInstanceStarted(const juce::String& commandLine) {
        // When another instance of the app is launched while this one is running,
        // this method is invoked, and the commandLine parameter tells you what
        // the other instance's command-line arguments were.
        juce::ignoreUnused(commandLine);
    };
} // namespace evil