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
        juce::ignoreUnused(commandLine);

        initialiseLogger("evil_daw_log");
        initCommandManager();

        _applicationSettings = std::make_unique<EvilDawApplicationSettings>();

        _mainWindow.reset(new evil::EvilDAWMainWindow(getApplicationName(), *this));
        _mainWindow->setVisible(true);
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
    }

    juce::PropertiesFile::Options EvilDAWApplication::getPropertyFileOptionsFor(const juce::String& filename, bool isProjectSettings)
    {
        juce::PropertiesFile::Options options;
        options.applicationName = filename;
        options.filenameSuffix = "settings";
        options.osxLibrarySubFolder = "Application Support";
        options.folderName = "EvilDAW";
        if (isProjectSettings)
            options.folderName += "/ProjectSettings";
        return options;
    }


    EvilDawApplicationSettings& EvilDAWApplication::getApplicationSettings()
    {
        return *_applicationSettings;
    }
    
    juce::ApplicationCommandManager& EvilDAWApplication::getCommandManager()
    {
        auto* cm = EvilDAWApplication::getApp()._commandManager.get();
        jassert(cm != nullptr);
        return *cm;
    }

    bool EvilDAWApplication::initialiseLogger(const char* filePrefix)
    {
        if (_logger == nullptr)
        {
            // Create a date-stamped logger that writes to a file in 
            // the "evil.audio.logs" folder, with the specified file prefix and a ".txt" extension.
            // On windows, this will typically be located in the user's "AppData\Roaming" folder, 
            // while on macOS it will be in the user's Library/Logs folder.
            juce::String folder = "evil.audio.logs";
            _logger.reset(juce::FileLogger::createDateStampedLogger(folder, filePrefix, ".txt",
                                            getApplicationName() + " " + getApplicationVersion()
                                            + "  ---  Build date: " __DATE__));
            juce::Logger::setCurrentLogger(_logger.get());
        }
        return _logger != nullptr;
    }

    void EvilDAWApplication::initCommandManager()
    {
        _commandManager.reset(new juce::ApplicationCommandManager());
        _commandManager->registerAllCommandsForTarget(this);
    }

    void EvilDAWApplication::shutdownLogger()
    {
        juce::Logger::setCurrentLogger(nullptr);
        _logger.reset();
    }

} // namespace evil