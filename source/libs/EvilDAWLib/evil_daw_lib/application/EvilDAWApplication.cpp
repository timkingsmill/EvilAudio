#include <functional>
#include <utility>

#include "EvilDAWApplication.h"

#include "windows/main/MainAppWindowView.h"
#include "windows/main/MainAppController.h"
#include "windows/main/MainAppDataModel.h"

#include "plugins/edaw_plugin_graph.h"
#include "plugins/edaw_internal_plugins.h"
#include "plugins/edaw_plugin_scanner.h"
#include "plugins/edaw_plugins_list_window.h"
#include "settings/editors/edaw_audio_settings_editor.h"


namespace evil
{

    //==============================================================================

    /**
     * @brief Base offset used for dynamically-generated plugin menu item IDs.
     *
     * @details
     * Each plugin menu entry is assigned an ID of:
     * @code
     * menuIDBase + indexInAddedPlugins
     * @endcode
     * where @c indexInAddedPlugins is the index of the corresponding
     * `PluginDescriptionAndPreference` in `_pluginDescriptionsAndPreference`.
     *
     * This establishes a stable mapping from a menu selection back to the
     * originating plugin description for creation.
     */
    static constexpr int menuIDBase = 0x324503f4;
    static constexpr const char* kMainWindowStateKey = "MainWindowState";
    static constexpr const char* kPluginListWindowStateKey = "PluginListWindowState";
    static constexpr const char* kAudioSettingsWindowStateKey = "AudioSettingsWindowState";

    namespace
    {
        class AudioSettingsWindow final : public juce::DialogWindow
        {
        public:
            AudioSettingsWindow(const juce::String& title,
                                juce::Colour backgroundColour,
                                bool escapeKeyTriggersCloseButton,
                                bool useNativeTitleBar,
                                bool resizable,
                                std::function<void()> onCloseRequested)
                : juce::DialogWindow(title, backgroundColour, escapeKeyTriggersCloseButton, useNativeTitleBar),
                  onCloseRequested_(std::move(onCloseRequested))
            {
                setResizable(resizable, false);
            }

        private:
            void closeButtonPressed() override
            {
                setVisible(false);

                if (onCloseRequested_)
                    juce::MessageManager::callAsync(onCloseRequested_);
            }

        private:
            std::function<void()> onCloseRequested_;
        };
    }

    //==============================================================================

    /**
     * @brief Determines if a plugin list contains multiple entries with the same name.
     *
     * @details
     * Used when building the plugin menu to detect collisions so the plugin
     * format name can be appended (e.g., `"MyPlugin (VST3)"`).
     *
     * A simple linear scan counts matching names and returns @c true as soon
     * as more than one match is found.
     *
     * @param plugins List of plugin descriptions to search.
     * @param name    Name to test for duplicates.
     * @return @c true if the given name occurs more than once in @p plugins.
     */
    static bool containsDuplicateNames(const juce::Array<juce::PluginDescription>& plugins,
                                       const juce::String& name)
    {
        int matches = 0;

        for (auto& p : plugins)
            if (p.name == name && ++matches > 1)
                return true;

        return false;
    }

    //==============================================================================

    /**
     * @brief Recursively flattens a JUCE plugin tree into a popup menu.
     *
     * @details
     * Builds a hierarchical popup menu from a `juce::KnownPluginList::PluginTree`,
     * storing the associated `PluginDescriptionAndPreference` entries in
     * @p addedPlugins. Each menu item ID is computed from its index within
     * @p addedPlugins plus `menuIDBase`, so callers can map the chosen menu
     * item back to the corresponding plugin.
     *
     * For each plugin:
     *  - A non-ARA entry is always added.
     *  - If ARA hosting is enabled and the plugin exposes ARA, an additional
     *    `" (ARA)"` entry is added.
     */
    static void addToMenu(const juce::KnownPluginList::PluginTree& tree,
                          juce::PopupMenu& m,
                          const juce::Array<juce::PluginDescription>& allPlugins,
                          juce::Array<PluginDescriptionAndPreference>& addedPlugins)
    {
        juce::ignoreUnused(allPlugins);

        for (auto* sub : tree.subFolders)
        {
            juce::PopupMenu subMenu;
            addToMenu(*sub, subMenu, allPlugins, addedPlugins);

            m.addSubMenu(sub->folder, subMenu, true, nullptr, false, 0);
        }

        auto addPlugin = [&] (const auto& descriptionAndPreference, const auto& pluginName)
        {
            addedPlugins.add(descriptionAndPreference);
            const auto menuID = addedPlugins.size() - 1 + menuIDBase;
            m.addItem(menuID, pluginName, true, false);
        };

        for (auto& plugin : tree.plugins)
        {
            auto name = plugin.name;

            if (containsDuplicateNames(tree.plugins, name))
                name << " (" << plugin.pluginFormatName << ')';

            addPlugin(PluginDescriptionAndPreference{ plugin, PluginDescriptionAndPreference::UseARA::no }, name);

       #if JUCE_PLUGINHOST_ARA && (JUCE_MAC || JUCE_WINDOWS || JUCE_LINUX)
            if (plugin.hasARAExtension)
            {
                auto araName = name;
                araName << " (ARA)";
                addPlugin(PluginDescriptionAndPreference{ plugin }, araName);
            }
       #endif
        }
    }

    //==============================================================================

    /**
     * @brief Constructs the `EvilDAWApplication` and sets default plugin sort method.
     *
     * @details
     * The plugin list itself is not created here; that is deferred to
     * `initialise()`. The default sort is by manufacturer.
     */
    EvilDAWApplication::EvilDAWApplication() :
        _pluginSortMethod(juce::KnownPluginList::sortByManufacturer),
        JUCEApplication()
    {
    }

    /**
     * @brief Destroys the `EvilDAWApplication` instance.
     *
     * @details
     * Most cleanup is performed in `shutdown()`. The destructor is defaulted.
     */
    EvilDAWApplication::~EvilDAWApplication() = default;

    //==============================================================================

    /**
     * @brief Returns the human-readable application name.
     *
     * @details
     * Used in window titles, logs, and various platform-specific locations.
     */
    const juce::String EvilDAWApplication::getApplicationName() { return "ProjectInfo::projectName"; }

    /**
     * @brief Returns the application version string.
     *
     * @details
     * Typically of the form `"major.minor.patch"` as defined by the build system.
     */
    const juce::String EvilDAWApplication::getApplicationVersion() { return "ProjectInfo::versionString"; };

    //==============================================================================

    /**
     * @brief Returns the global `EvilDAWApplication` instance.
     *
     * @details
     * Wraps `JUCEApplication::getInstance()` and performs a `dynamic_cast`
     * to the concrete type, asserting success.
     *
     * @return Reference to the singleton `EvilDAWApplication`.
     */
    EvilDAWApplication& EvilDAWApplication::getApplicationInstance()
    {
        EvilDAWApplication* const app = dynamic_cast<EvilDAWApplication*> (JUCEApplication::getInstance());
        jassert(app != nullptr);
        return *app;
    }

    /**
     * @brief Returns a reference to the main DAW window.
     *
     * @details
        * Returns the stored `MainWindowView` instance and asserts that it has
        * already been created.
     *
        * @return Reference to the `MainWindowView` instance.
     */
    daw::MainAppWindow& EvilDAWApplication::getMainWindow()
    {
        auto* window = EvilDAWApplication::getApplicationInstance()._mainAppWindow.get();
        jassert(window != nullptr);
        return *window;
    }

    /**
     * @brief Indicates whether multiple instances of the app are allowed.
     *
     * @return Always returns @c false; EvilDAW is single-instance only.
     */
    bool EvilDAWApplication::moreThanOneInstanceAllowed() { return false; }

    //==============================================================================

    void EvilDAWApplication::initialise(const juce::String& commandLine)
    {
        juce::PropertiesFile::Options options;
        options.applicationName = "EvilDAW";
        options.filenameSuffix = "settings";
        options.osxLibrarySubFolder = "Preferences";

        _applicationProperties = std::make_unique<juce::ApplicationProperties>();
        _applicationProperties->setStorageParameters(options);

        initialiseLogger("evil_daw_log");
        initialiseCommandManager();
        initialiseDeviceManager();

        //-------------------------------------------------------------------------
        // Check if this process is running as a dedicated plugin scanner subprocess.
        // In that case, normal UI initialization is skipped and the subprocess
        // instance is retained instead.
        auto scannerSubprocess = std::make_unique<PluginScannerSubprocess>();
        if (scannerSubprocess->initialiseFromCommandLine(commandLine, processUID))
        {
            _storedScannerSubprocess = std::move(scannerSubprocess);
            return;
        }

        
        // NOTE: Controller creation is deferred until after main window is created
        // See ensureMainWindow() for the initialization sequence
        
        _pluginListWindow = nullptr;

        _knownPluginList = std::make_unique<juce::KnownPluginList>();
        _knownPluginList->setCustomScanner(std::make_unique<CustomPluginScanner>());

        _pluginFormatManager = std::make_unique<juce::AudioPluginFormatManager>();

        _graphHolder.reset(new GraphDocumentComponent(*_pluginFormatManager, *_audioDeviceManager, *_knownPluginList));

        InternalPluginFormat internalFormat;
        auto& internalTypes = internalFormat.getAllTypes();

        if (auto savedPluginList = getAppProperties().getUserSettings()->getXmlValue("pluginList"))
        {
            _knownPluginList->recreateFromXml(*savedPluginList);
        }

        for (auto& t : internalTypes)
        {
            _knownPluginList->addType(t);
        }

        _pluginSortMethod = (juce::KnownPluginList::SortMethod)getAppProperties().getUserSettings()
            ->getIntValue("pluginSortMethod", juce::KnownPluginList::sortByManufacturer);

        _knownPluginList->addChangeListener(this);

        //-------------------------------------------------------------------------
        // Defer main window creation until after the message loop has started.
        triggerAsyncUpdate();
    }

    //==============================================================================

    /**
     * @brief Performs orderly shutdown of the application.
     *
     * @details
     * Called when the JUCE application terminates. It:
        *  - Closes and releases the owned top-level windows.
        *  - Unregisters from the known plugin list.
        *  - Releases the known plugin list, audio device manager, and application
        *    properties.
        *  - Shuts down the logger.
     */
    void EvilDAWApplication::shutdown()
    {
        closeAllWindows();
        
        // Reset main window before controller
        _mainAppWindow.reset();
        //_mainAppDataModel.reset();
        _mainAppController.reset();

        if (_knownPluginList != nullptr)
            _knownPluginList->removeChangeListener(this);

        _knownPluginList.reset();
        _audioDeviceManager.reset();
        _applicationProperties = nullptr;

        shutdownLogger();
    };

    //==============================================================================

    /**
     * @brief Handles OS-level quit requests.
     *
     * @details
     * Forwards directly to JUCE's `quit()` to trigger normal shutdown.
     */
    void EvilDAWApplication::systemRequestedQuit()
    {
        quit();
    };

    //==============================================================================

    void EvilDAWApplication::anotherInstanceStarted(const juce::String& commandLine)
    {
        juce::ignoreUnused(commandLine);
    }

    //==============================================================================

    juce::PropertiesFile::Options EvilDAWApplication::getPropertyFileOptionsFor(const juce::String& filename,
                                                                                bool isProjectSettings)
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

    //==============================================================================

    juce::ApplicationProperties& EvilDAWApplication::getApplicationProperties()
    {
        auto& app = EvilDAWApplication::getApplicationInstance();
        jassert(app._applicationProperties != nullptr);
        return *app._applicationProperties;
    }

    //==============================================================================
    void EvilDAWApplication::changeListenerCallback(juce::ChangeBroadcaster* changedItem)
    {
        if (changedItem == _knownPluginList.get())
        {
            // Persist plugin list changes eagerly to avoid losing scanned results
            // if the host crashes during plugin discovery.
            if (auto savedPluginList = std::unique_ptr<juce::XmlElement>(_knownPluginList->createXml()))
            {
                getAppProperties().getUserSettings()->setValue("pluginList", savedPluginList.get());
                getAppProperties().saveIfNeeded();
            }
        }
    }

    //==============================================================================

    /**
     * @brief Returns the global command manager.
     *
     * @details
     * Asserts `_commandManager` is non-null and returns it by reference.
     */
    juce::ApplicationCommandManager& EvilDAWApplication::getCommandManager()
    {
        auto* cm = EvilDAWApplication::getApplicationInstance()._commandManager.get();
        jassert(cm != nullptr);
        return *cm;
    }

    /**
     * @brief Returns the global audio device manager.
     *
     * @details
     * Asserts `_audioDeviceManager` is non-null and returns it by reference.
     */
    juce::AudioDeviceManager& EvilDAWApplication::getAudioDeviceManager()
    {
        auto* adm = EvilDAWApplication::getApplicationInstance()._audioDeviceManager.get();
        jassert(adm != nullptr);
        return *adm;
    }

    void EvilDAWApplication::handleAsyncUpdate()
    {
        ensureMainWindow();
    }

    /**
     * @brief Ensures the main application window exists and is visible.
     */
    void EvilDAWApplication::ensureMainWindow()
    {
        if (_mainAppWindow == nullptr)
        {
            _mainAppDataModel = std::make_shared<daw::MainAppDataModel>();
            _mainAppWindow = std::make_shared<daw::MainAppWindow>(*this, getApplicationName());

            _mainAppController = std::make_unique<daw::MainAppController>();

            _mainAppController->addView(_mainAppWindow);
            _mainAppController->setDataModel(_mainAppDataModel);

            restoreWindowState(*_mainAppWindow, kMainWindowStateKey);
        }
        _mainAppWindow->setVisible(true);
        _mainAppWindow->toFront(true);
    }

    //==============================================================================

    /**
        * @brief Shows or focuses the audio settings window.
     *
     * @details
        * Validates prerequisites, lazily creates an `AudioSettingsWindow`
        * containing `EvilDAWAudioSettingsEditor`, restores its saved geometry,
        * and reuses it on subsequent calls. When the window closes, the current
        * audio device state is saved back to the user settings.
     *
        * Runtime checks are performed using `juce::WeakReference` to avoid
        * dereferencing destroyed application state during asynchronous close
        * handling.
     *
        * @par Window lifecycle
     * ```mermaid
     * graph TD
     *   A["showAudioSettings()"] --> B{"_audioDeviceManager != nullptr?"}
     *   B -- "no" --> C["Log and return"]
     *   B -- "yes" --> D{"settingsFile (userSettings) != nullptr?"}
     *   D -- "no" --> E["Log and return"]
        *   D -- "yes" --> F{"_audioSettingsWindow == nullptr?"}
        *   F -- "yes" --> G["Create AudioSettingsWindow + editor"]
        *   G --> H["Restore window state"]
        *   H --> I["Install close callback for save/reset"]
        *   F -- "no" --> J["Reuse existing window"]
        *   I --> K["Show, restore if minimized, bring to front"]
        *   J --> K
     * ```
     */
    void EvilDAWApplication::showAudioSettings()
    {
        ensureAudioSettingsWindow();
    }

    void EvilDAWApplication::ensureAudioSettingsWindow()
    {
        if (_audioDeviceManager == nullptr)
        {
            juce::Logger::writeToLog("showAudioSettings: audio device manager is null");
            return;
        }

        auto* settingsFile = (_applicationProperties != nullptr) ? _applicationProperties->getUserSettings() : nullptr;
        if (settingsFile == nullptr)
        {
            juce::Logger::writeToLog("showAudioSettings: application settings file is null");
            return;
        }

        if (_audioSettingsWindow == nullptr)
        {
            juce::WeakReference<EvilDAWApplication> safeThis(this);
            auto window = std::make_unique<AudioSettingsWindow>(
                "Audio Settings",
                juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId),
                true,
                true,
                false,
                [safeThis]
                {
                    if (safeThis == nullptr || safeThis->_audioSettingsWindow == nullptr)
                        return;

                    safeThis->saveWindowState(*safeThis->_audioSettingsWindow, kAudioSettingsWindowStateKey);

                    if (safeThis->_audioDeviceManager != nullptr && safeThis->_applicationProperties != nullptr)
                    {
                        auto* userSettings = safeThis->_applicationProperties->getUserSettings();
                        if (userSettings != nullptr)
                        {
                            auto audioState = safeThis->_audioDeviceManager->createStateXml();
                            userSettings->setValue("audioDeviceState", audioState.get());
                            userSettings->saveIfNeeded();
                        }
                    }

                    safeThis->_audioSettingsWindow.reset();
                });

            auto* settingsComponent = new EvilDAWAudioSettingsEditor(*_audioDeviceManager);
            settingsComponent->setSize(600, 440);
            window->setContentOwned(settingsComponent, true);

            _audioSettingsWindow = std::move(window);
            restoreWindowState(*_audioSettingsWindow, kAudioSettingsWindowStateKey);
        }

        if (_audioSettingsWindow != nullptr)
        {
            _audioSettingsWindow->setVisible(true);

            if (_audioSettingsWindow->isMinimised())
                _audioSettingsWindow->setMinimised(false);

            _audioSettingsWindow->toFront(true);
        }
    }

    //==============================================================================

    /**
     * @brief Shows or focuses the plugin list window.
     *
     * @details
     * Lazily constructs `PluginListWindow` when needed, restores its saved
        * window state, then installs an asynchronous close callback that persists
     * updated window state and destroys the window on close. If the window
     * already exists, it is simply brought to the front.
     *
     * ```mermaid
     * graph TD
     *   A["showPluginListSettings()"] --> B{"_pluginListWindow == nullptr?"}
     *   B -- "no" --> C["_pluginListWindow->toFront(true)"]
     *   B -- "yes" --> D{"_pluginFormatManager && _knownPluginList ?"}
     *   D -- "no" --> E["Log and return"]
     *   D -- "yes" --> F{"_applicationProperties && userSettings ?"}
     *   F -- "no" --> G["Log and return"]
     *   F -- "yes" --> H["Create PluginListWindow"]
     *   H --> I["Restore window state from settings"]
     *   I --> J["Create WeakReference<EvilDAWApplication>"]
        *   J --> K["Install close callback\nsaving window state\nand resetting _pluginListWindow"]
     * ```
     */
    void EvilDAWApplication::showPluginListSettings()
    {
        ensurePluginListWindow();
    }

    void EvilDAWApplication::ensurePluginListWindow()
    {
        if (_pluginListWindow == nullptr)
        {
            if (_pluginFormatManager == nullptr || _knownPluginList == nullptr)
            {
                juce::Logger::writeToLog("showPluginListSettings: plugin manager/list not initialized");
                return;
            }

            auto* settingsFile = (_applicationProperties != nullptr) ? _applicationProperties->getUserSettings() : nullptr;
            if (settingsFile == nullptr)
            {
                juce::Logger::writeToLog("showPluginListSettings: application settings file is null");
                return;
            }

            if (_pluginListWindow == nullptr)
            {
                _pluginListWindow = std::make_unique<PluginListWindow>(
                    *_pluginFormatManager,
                    *_knownPluginList);

                restoreWindowState(*_pluginListWindow, kPluginListWindowStateKey);

                juce::WeakReference<EvilDAWApplication> safeThis(this);
                _pluginListWindow->setOnCloseRequested([safeThis]
                {
                    if (safeThis == nullptr || safeThis->_pluginListWindow == nullptr || safeThis->_applicationProperties == nullptr)
                        return;

                    safeThis->saveWindowState(*safeThis->_pluginListWindow, kPluginListWindowStateKey);
                    safeThis->_pluginListWindow.reset();

                    juce::Logger::writeToLog("Plugin List Window closed");
                });
            }
        }
        if (_pluginListWindow != nullptr)
        {
            _pluginListWindow->setVisible(true);

            if (_pluginListWindow->isMinimised())
                _pluginListWindow->setMinimised(false);

            _pluginListWindow->toFront(true);
        }
    }

    void EvilDAWApplication::restoreWindowState(juce::DocumentWindow& window, const juce::String& stateKey)
    {
        if (_applicationProperties == nullptr)
            return;

        auto* userSettings = _applicationProperties->getUserSettings();
        if (userSettings == nullptr)
            return;

        const auto state = userSettings->getValue(stateKey);
        if (state.isNotEmpty())
            window.restoreWindowStateFromString(state);
    }

    void EvilDAWApplication::saveWindowState(juce::DocumentWindow& window, const juce::String& stateKey) const
    {
        if (_applicationProperties == nullptr)
            return;

        auto* userSettings = _applicationProperties->getUserSettings();
        if (userSettings == nullptr)
            return;

        userSettings->setValue(stateKey, window.getWindowStateAsString());
        userSettings->saveIfNeeded();
    }

    void EvilDAWApplication::closeAllWindows()
    {
        if (_audioSettingsWindow != nullptr)
        {
            saveWindowState(*_audioSettingsWindow, kAudioSettingsWindowStateKey);
            _audioSettingsWindow.reset();
        }

        if (_pluginListWindow != nullptr)
        {
            saveWindowState(*_pluginListWindow, kPluginListWindowStateKey);
            _pluginListWindow.reset();
        }

        if (_mainAppWindow != nullptr)
        {
            saveWindowState(*_mainAppWindow, kMainWindowStateKey);
            _mainAppWindow.reset();
        }
    }

    //==============================================================================
    bool EvilDAWApplication::initialiseLogger(const char* filePrefix)
    {
        if (_logger == nullptr)
        {
            juce::String folder = "evil.audio.logs";
            _logger.reset(juce::FileLogger::createDateStampedLogger(folder,
                                                                    filePrefix,
                                                                    ".txt",
                                                                    getApplicationName() + " " + getApplicationVersion()
                                                                    + "  ---  Build date: " __DATE__));
            juce::Logger::setCurrentLogger(_logger.get());
        }
        return _logger != nullptr;
    }

    //==============================================================================

    /**
     * @brief Creates and registers the application command manager.
     *
     * @details
     * Allocates `juce::ApplicationCommandManager` and registers all commands
     * provided by `EvilDAWApplication` as a target.
     */
    void EvilDAWApplication::initialiseCommandManager()
    {
        _commandManager.reset(new juce::ApplicationCommandManager());
        _commandManager->registerAllCommandsForTarget(this);
    }

    //==============================================================================

    /**
     * @brief Initializes the audio device manager from persisted state.
     *
     * @details
     * Loads `"audioDeviceState"` from user settings (if present) and calls
     * `juce::AudioDeviceManager::initialise()` with 256 input/output channels.
     * Any returned error string is logged.
     *
     * If properties or user settings are unavailable, logs and returns
     * without creating a device manager.
     */
    void EvilDAWApplication::initialiseDeviceManager()
    {
        auto* settingsFile = (_applicationProperties != nullptr) ? _applicationProperties->getUserSettings() : nullptr;
        if (settingsFile == nullptr)
        {
            juce::Logger::writeToLog("initialiseDeviceManager: application settings file is null");
            return;
        }

        auto deviceSettings = settingsFile->getXmlValue("audioDeviceState");

        _audioDeviceManager = std::make_unique<juce::AudioDeviceManager>();
        const auto error = _audioDeviceManager->initialise(256, 256, deviceSettings.get(), true);

        if (error.isNotEmpty())
            juce::Logger::writeToLog("initialiseDeviceManager error: " + error);
    }

    //==============================================================================

    /**
     * @brief Shuts down and detaches the logger.
     *
     * @details
     * Clears JUCE's current logger and resets `_logger`, flushing and
     * closing the underlying log file.
     */
    void EvilDAWApplication::shutdownLogger()
    {
        juce::Logger::setCurrentLogger(nullptr);
        _logger.reset();
    }

} // namespace evil