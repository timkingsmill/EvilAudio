#include "evil_daw_application.h"
#include "windows/main/evil_daw_main_window.h"
#include <juce_core/juce_core.h>


// Implemented in juce_core_CompilationTime.cpp
extern const char* evil_daw_compilationDate;
extern const char* evil_daw_compilationTime;

namespace evil
{
    const juce::String EvilDAWApplication::getApplicationName() { return "ProjectInfo::projectName"; }
    const juce::String EvilDAWApplication::getApplicationVersion() { return "ProjectInfo::versionString"; };

    EvilDAWApplication& EvilDAWApplication::getApp()
    {
        EvilDAWApplication* const app = dynamic_cast<EvilDAWApplication*> (JUCEApplication::getInstance());
        jassert(app != nullptr);
        return *app;
    }

    bool EvilDAWApplication::moreThanOneInstanceAllowed() { return false; }

    void EvilDAWApplication::initialise(const juce::String& commandLine) 
    {
        initialiseLogger("evil_daw_log");
        juce::Logger::writeToLog(juce::SystemStats::getOperatingSystemName());

        //auto compilationDateTime = juce::String(evil_daw_compilationDate) + " " + juce::String(evil_daw_compilationTime);   
        // This method is where you should put your application's initialisation code..
        juce::ignoreUnused(commandLine);
        _mainWindow.reset(new evil::EvilDAWMainWindow(getApplicationName(), *this));
    };

    void EvilDAWApplication::shutdown() {
        // Application's shutdown code here..



        shutdownLogger();
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

    bool EvilDAWApplication::initialiseLogger(const char* filePrefix)
    {
        if (logger == nullptr)
        {
            // Create a date-stamped logger that writes to a file in 
            // the "evil.audio.logs" folder, with the specified file prefix and a ".txt" extension.
            // On windows, this will typically be located in the user's "AppData\Roaming" folder, 
            // while on macOS it will be in the user's Library/Logs folder.
            juce::String folder = "evil.audio.logs";
            logger.reset(juce::FileLogger::createDateStampedLogger(folder, filePrefix, ".txt",
                                            getApplicationName() + " " + getApplicationVersion()
                                            + "  ---  Build date: " __DATE__));
            juce::Logger::setCurrentLogger(logger.get());
        }
        return logger != nullptr;
    }

    void EvilDAWApplication::shutdownLogger()
    {
        juce::Logger::setCurrentLogger(nullptr);
        logger.reset();
    }

} // namespace evil