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
    /**
     * @brief Returns the application name.
     * @return A juce::String containing the application name.
     */
    const juce::String EvilDAWApplication::getApplicationName() { return "ProjectInfo::projectName"; }

    /**
     * @brief Returns the application version.
     * @return A juce::String containing the application version.
     */
    const juce::String EvilDAWApplication::getApplicationVersion() { return "ProjectInfo::versionString"; };

    /**
     * @brief Retrieves the singleton instance of the EvilDAWApplication.
     * 
     * This method provides access to the global application instance.
     * Asserts if the instance is not available.
     *
     * @return A reference to the EvilDAWApplication instance.
     * @see EvilDAWMainWindow
     */
    EvilDAWApplication& EvilDAWApplication::getApp()
    {
        EvilDAWApplication* const app = dynamic_cast<EvilDAWApplication*> (JUCEApplication::getInstance());
        jassert(app != nullptr);
        return *app;
    }

    /**
     * @brief Retrieves the main application window.
     * 
     * Returns a reference to the main EvilDAW window. Asserts if the window
     * is not available.
     *
     * @return A reference to the EvilDAWMainWindow instance.
     * @see EvilDAWMainWindow
     */
    EvilDAWMainWindow& EvilDAWApplication::getMainWindow()
    {
        auto* window = dynamic_cast<EvilDAWMainWindow*>(EvilDAWApplication::getApp()._mainWindow.get());
        jassert(window != nullptr);
        return *window;
    }

    /**
     * @brief Determines if multiple instances of the application can run simultaneously.
     * @return false, as only one instance of EvilDAW is allowed.
     */
    bool EvilDAWApplication::moreThanOneInstanceAllowed() { return false; }

    /**
     * @brief Initializes the application on startup.
     * 
     * Sets up logger, command manager, application settings, and audio device manager.
     * Triggers async update to complete initialization after the message loop starts.
     *
     * @param commandLine The command line arguments passed to the application.
     */
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

    /**
     * @brief Performs cleanup operations when the application is shutting down.
     * 
     * Resets the audio device manager and shuts down the logger.
     */
    void EvilDAWApplication::shutdown() {
        // Application's shutdown code here..
        //_menuModel.reset();
        //_commandManager.reset();
        //_applicationSettings.reset();

        _audioDeviceManager.reset();

        shutdownLogger();
    };

    /**
     * @brief Handles system-level requests to quit the application.
     * 
     * Called when the OS requests the application to quit. Allows the application
     * to quit cleanly.
     */
    void EvilDAWApplication::systemRequestedQuit() {
        // This is called when the app is being asked to quit: you can ignore this
        // request and let the app carry on running, or call quit() to allow the app to close.
        quit();
    };

    /**
     * @brief Handles the event when another instance of the application is started.
     * 
     * Called when another instance of EvilDAW is launched while this one is running.
     *
     * @param commandLine The command line arguments passed to the new instance.
     */
    void EvilDAWApplication::anotherInstanceStarted(const juce::String& commandLine) {
        // When another instance of the app is launched while this one is running,
        // this method is invoked, and the commandLine parameter tells you what
        // the other instance's command-line arguments were.
        juce::ignoreUnused(commandLine);
    }

    /**
     * @brief Generates property file options for settings persistence.
     * 
     * Creates configuration options for saving and loading application or project settings.
     *
     * @param filename The base filename for the settings file.
     * @param isProjectSettings If true, settings are treated as project-specific.
     * @return A juce::PropertiesFile::Options object configured for the given parameters.
     */
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

    /**
     * @brief Retrieves the application settings.
     * 
     * Provides access to the global application settings object.
     * Asserts if settings are not available.
     *
     * @return A reference to the EvilDawApplicationSettings instance.
     * @see EvilDawApplicationSettings
     */
    EvilDawApplicationSettings& EvilDAWApplication::getApplicationSettings()
    {
        auto* settings = EvilDAWApplication::getApp()._applicationSettings.get();
        jassert(settings != nullptr);
        return *settings;
    }

    /**
     * @brief Retrieves the application command manager.
     * 
     * Provides access to the command manager for handling application commands
     * and keyboard shortcuts. Asserts if the command manager is not available.
     *
     * @return A reference to the juce::ApplicationCommandManager instance.
     */
    juce::ApplicationCommandManager& EvilDAWApplication::getCommandManager()
    {
        auto* cm = EvilDAWApplication::getApp()._commandManager.get();
        jassert(cm != nullptr);
        return *cm;
    }

    /**
     * @brief Retrieves the audio device manager.
     * 
     * Provides access to the audio device manager for audio I/O configuration.
     * Asserts if the device manager is not available.
     *
     * @return A reference to the juce::AudioDeviceManager instance.
     */
    juce::AudioDeviceManager& EvilDAWApplication::getAudioDeviceManager()
    {
        auto* adm = EvilDAWApplication::getApp()._audioDeviceManager.get();
        jassert(adm != nullptr);
        return *adm;
    }

    /**
     * @brief Registers all available commands with the command manager.
     * 
     * Called by the ApplicationCommandManager to discover all commands
     * that the application can perform.
     *
     * @param commands Array to be populated with command IDs.
     */
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

    /**
     * @brief Retrieves information about a specific command.
     * 
     * Provides the command description, category, and default keyboard shortcut.
     * Used by the menu system and command manager.
     *
     * @param commandID The ID of the command to query.
     * @param commandInfo Output parameter to be filled with command information.
     */
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

    /**
     * @brief Retrieves the menu bar model.
     * 
     * Returns the model that defines the structure of the application's menu bar.
     *
     * @return A pointer to the juce::MenuBarModel instance, or nullptr if not available.
     */
    juce::MenuBarModel* EvilDAWApplication::getMenuBarModel()
    {
        return _menuModel.get();
    }

    /**
     * @brief Retrieves the names of all top-level menu items.
     * 
     * Called by the menu system to determine which menus to display.
     *
     * @return A juce::StringArray containing the menu bar names.
     */
    juce::StringArray EvilDAWApplication::getMenuBarNames()
    {
        return juce::StringArray{ "File", "Edit", "View", "Options", "Help" };
    }

    /**
     * @brief Generates a popup menu for the given menu bar index.
     * 
     * Creates the menu items for the specified menu based on the menu name.
     *
     * @param menuIndex The index of the menu bar item.
     * @param menuName The name of the menu (e.g., "File", "Edit").
     * @return A juce::PopupMenu containing the menu items.
     */
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

    /**
     * @brief Performs the action associated with a command.
     * 
     * Handles the execution of application commands, such as Copy and AudioSettings.
     *
     * @param invocationInfo Contains the command ID and invocation details.
     * @return true if the command was handled, false otherwise.
     */
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

    /**
     * @brief Handles deferred initialization after the message loop has started.
     * 
     * Creates the menu model and main window, then makes the window visible.
     */
    void EvilDAWApplication::handleAsyncUpdate()
    {
        _menuModel.reset(new EvilDAWApplicationMenuModel());
        _mainWindow.reset(new evil::EvilDAWMainWindow(getApplicationName(), *this));
        _mainWindow->setVisible(true);
    }

    /**
     * @brief Displays the audio settings dialog window.
     * 
     * Creates and shows a modal dialog for configuring audio device settings.
     * On close, saves the audio device state to application settings.
     * 
     * @see EvilDAWAudioSettingsEditor
     */
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
    
    /**
     * @brief Initializes the application settings object.
     * 
     * Creates and configures the EvilDawApplicationSettings instance.
     * 
     * @see EvilDawApplicationSettings
     */
    void EvilDAWApplication::initialiseApplicatiomSettings()
    {
        _applicationSettings = std::make_unique<EvilDawApplicationSettings>();
    }

    /**
     * @brief Initializes the file logger.
     * 
     * Creates a date-stamped logger that writes to a file in the application logs directory.
     * On Windows, logs are stored in AppData\Roaming; on macOS, in Library/Logs.
     *
     * @param filePrefix The prefix for the log filename.
     * @return true if the logger was successfully initialized, false otherwise.
     */
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

    /**
     * @brief Initializes the command manager.
     * 
     * Creates the ApplicationCommandManager and registers all available commands.
     */
    void EvilDAWApplication::initialiseCommandManager()
    {
        _commandManager.reset(new juce::ApplicationCommandManager());
        _commandManager->registerAllCommandsForTarget(this);
    }

    /**
    * @brief Initializes the audio device manager.
    *
    * Creates the AudioDeviceManager and loads the previously saved audio device state
    * from application settings, or initializes with default devices.
    * On Windows, this enables ASIO driver support if available.
    */
    void EvilDAWApplication::initialiseDeviceManager()
    {
        // Initialize audio device manager
        auto deviceSettings = getApp().getApplicationSettings().getGlobalProperties().getXmlValue("audioDeviceState");
        _audioDeviceManager = std::make_unique<juce::AudioDeviceManager>();
        // Create audio device types (includes ASIO on Windows if configured)
        //_audioDeviceManager->createAudioDeviceTypes();
        _audioDeviceManager->initialise(256, 256, deviceSettings.get(), true);
    }

    /**
     * @brief Shuts down the file logger.
     * 
     * Removes the current logger and resets the logger instance.
     */
    void EvilDAWApplication::shutdownLogger()
    {
        juce::Logger::setCurrentLogger(nullptr);
        _logger.reset();
    }

} // namespace evil