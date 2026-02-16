#include <juce_core/juce_core.h>

#include "evil_daw_application.h"
#include "windows/main/evil_daw_main_window.h"
#include "commands/evil_daw_commandIDs.h"
#include "settings/editors/evil_daw_audio_settings_editor.h"


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

    EvilDAWMainWindow& EvilDAWApplication::getMainWindow()
    {
        auto* window = dynamic_cast<EvilDAWMainWindow*>(EvilDAWApplication::getApp()._mainWindow.get());
        jassert(window != nullptr);
        return *window;
    }

    bool EvilDAWApplication::moreThanOneInstanceAllowed() { return false; }

    void EvilDAWApplication::initialise(const juce::String& commandLine)
    {
        juce::ignoreUnused(commandLine);


        initialiseApplicatiomSettings();
        initialiseLogger("evil_daw_log");
        initialiseCommandManager();
        initialiseDeviceManager();

        // Do further initialisation in a moment 
        // when the message loop has started
        triggerAsyncUpdate();
    };

    void EvilDAWApplication::shutdown() {
        // Application's shutdown code here..
        //_menuModel.reset();
        //_commandManager.reset();
        //_applicationSettings.reset();

        _audioDeviceManager.reset();

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

    juce::AudioDeviceManager& EvilDAWApplication::getAudioDeviceManager()
    {
        auto* adm = EvilDAWApplication::getApp()._audioDeviceManager.get();
        jassert(adm != nullptr);
        return *adm;
    }

    // Here you would add the command IDs of all the commands that your application can perform.
    // This is used by the ApplicationCommandManager to know which commands are available, and to
    // enable/disable menu items and buttons based on the state of these commands.
    //
    // For example, if you have a command with ID 55, you would add it like this:
    // commands.add(55);
    void EvilDAWApplication::getAllCommands(juce::Array<juce::CommandID>& commands)
    {
        JUCEApplication::getAllCommands(commands);
        const juce::CommandID ids[] = {
            (int)CommandID::Copy,
            (int)CommandID::AudioSettings
        };
        size_t length = std::size(ids);
        commands.addArray(ids, length);
    }

    void EvilDAWApplication::getCommandInfo(juce::CommandID commandID, juce::ApplicationCommandInfo& commandInfo)
    {
        if (commandID == (int)CommandID::Copy)
        {
            commandInfo.setInfo("Hello World", "Prints Hello World to the console", "General", false);  // juce::ApplicationCommandInfo::enabled);
            commandInfo.addDefaultKeypress('h', juce::ModifierKeys::commandModifier);
        }
        else if (commandID == (int)CommandID::AudioSettings)
        {
            commandInfo.setInfo("Audio Settings...", "Configure audio device settings", "Options", 0);
            commandInfo.addDefaultKeypress(',', juce::ModifierKeys::commandModifier);
        }
        else
        {
            JUCEApplication::getCommandInfo(commandID, commandInfo);
        }
    }

    juce::MenuBarModel* EvilDAWApplication::getMenuBarModel()
    {
        return _menuModel.get();
    }

    juce::StringArray EvilDAWApplication::getMenuBarNames()
    {
        return juce::StringArray{ "File", "Edit", "View", "Options", "Help" };
    }

    juce::PopupMenu EvilDAWApplication::getMenuForIndex(int menuIndex, const juce::String& menuName)
    {
        auto* registeredInfo = _commandManager->getCommandForID(55);

        juce::PopupMenu popupMenu;

        if (menuName == "File")
        {
            popupMenu.addItem(1, "New Project");
            popupMenu.addItem(2, "Open Project...");
            popupMenu.addItem(3, "Save Project");
            popupMenu.addItem(4, "Save Project As...");
            popupMenu.addSeparator();

            // Commands must be added after the regular items, otherwise they won't be enabled/disabled correctly
            // The command manager will automatically enable/disable the menu item based on the command's state, 
            // and will also display the shortcut key if one is assigned to the command.
            // Note that the command manager will only manage the state of the menu item if the command is 
            // registered with the command manager, and if the menu item is added with a valid command ID 
            // that matches the registered command.
            popupMenu.addCommandItem(_commandManager.get(), (int)CommandID::Exit); // , "Hello World");

        }
        else if (menuName == "Edit")
        {
            popupMenu.addItem(6, "Undo");
            popupMenu.addItem(7, "Redo");
            popupMenu.addSeparator();
            popupMenu.addItem(8, "Cut");
            popupMenu.addCommandItem(_commandManager.get(), (int)CommandID::Copy); // , "Hello World");
            //popupMenu.addItem(9, "Copy");
            //popupMenu.addItem(10, "Paste");
        }
        else if (menuName == "View")
        {
            popupMenu.addItem(11, "Toggle Mixer");
            popupMenu.addItem(12, "Toggle Piano Roll");
        }
        else if (menuName == "Options")
        {
            popupMenu.addCommandItem(_commandManager.get(), (int)CommandID::AudioSettings);
        }
        else if (menuName == "Help")
        {
            popupMenu.addItem(13, "Documentation");
            popupMenu.addItem(14, "About EvilDAW");
        }

        return popupMenu;
    }

    bool EvilDAWApplication::perform(const InvocationInfo& invocationInfo)
    {
        if (invocationInfo.commandID == (int)CommandID::Copy)
        {
            juce::Logger::writeToLog("Copy Command Invocked");
            return true;
        }

        if (invocationInfo.commandID == (int)CommandID::AudioSettings)
        {
            showAudioSettings();
            return true;
        }


        return JUCEApplication::perform(invocationInfo);
    }

    // This method is called by the message thread at the next convenient time
    // after the triggerAsyncUpdate() method has been called.
    void EvilDAWApplication::handleAsyncUpdate()
    {
        _menuModel.reset(new EvilDAWApplicationMenuModel());
        _mainWindow.reset(new evil::EvilDAWMainWindow(getApplicationName(), *this));
        _mainWindow->setVisible(true);
    }

    /*** Moved from EvilDAWMainWindow to here because it needs to access the audio device manager, and it's more logical to have it in the application class. 
    ***/
    

    void EvilDAWApplication::showAudioSettings()
    {
        auto* settingsComponent = new EvilDAWAudioSettingsEditor(*_audioDeviceManager);
        settingsComponent->setSize(600, 440);

        juce::DialogWindow::LaunchOptions options;
        options.content.setOwned(settingsComponent);
        options.dialogTitle = "Audio Settings";
        options.dialogBackgroundColour = juce::Desktop::getInstance().getDefaultLookAndFeel()
                                             .findColour(juce::ResizableWindow::backgroundColourId);
        options.escapeKeyTriggersCloseButton = true;
        options.useNativeTitleBar = true;
        options.resizable = false;
        
        auto* window = options.create();

        window->enterModalState(true,
            juce::ModalCallbackFunction::create(
                [this](int)
                {
                    auto audioState = _audioDeviceManager->createStateXml();
                    _applicationSettings->getGlobalProperties().setValue("audioDeviceState", audioState.get());
                    _applicationSettings->getGlobalProperties().saveIfNeeded();

                    //if (safeThis->graphHolder != nullptr)
                    //    if (safeThis->graphHolder->graph != nullptr)
                    //        safeThis->graphHolder->graph->graph.removeIllegalConnections();
                }
            ), true);
    }
    

    void EvilDAWApplication::initialiseApplicatiomSettings()
    {
        _applicationSettings = std::make_unique<EvilDawApplicationSettings>();

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

    void EvilDAWApplication::initialiseCommandManager()
    {
        _commandManager.reset(new juce::ApplicationCommandManager());
        _commandManager->registerAllCommandsForTarget(this);
    }

    void EvilDAWApplication::initialiseDeviceManager()
    {
        // Initialize audio device manager
        auto deviceSettings = getApp().getApplicationSettings().getGlobalProperties().getXmlValue("audioDeviceState");
        _audioDeviceManager = std::make_unique<juce::AudioDeviceManager>();
        _audioDeviceManager->initialise(256, 256, deviceSettings.get(), true);
    }

    void EvilDAWApplication::shutdownLogger()
    {
        juce::Logger::setCurrentLogger(nullptr);
        _logger.reset();
    }

} // namespace evil