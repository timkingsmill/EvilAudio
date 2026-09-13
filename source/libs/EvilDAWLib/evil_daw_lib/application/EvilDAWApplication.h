#pragma once

#include <juce_audio_devices.h>

#include "graphs/edaw_graph_document_component.h"
#include "plugins/edaw_plugin_graph.h"
#include "plugins/edaw_plugin_scanner_subprocess.h"
#include "plugins/edaw_plugins_list_window.h"

/**
 * @defgroup evil_daw_application Evil DAW Application
 * @brief JUCE application entry point and process-level service host for EvilDAW.
 *
 * @details
 * This module implements the concrete application runtime that boots and
 * coordinates EvilDAW. It owns process-wide services (settings, logging,
 * command routing, audio devices, plugin management) and orchestrates the
 * main-window MVC triad used by the DAW shell.
 *
 * Startup behavior is split into two phases:
 * - `initialise()` creates core services, restores persisted plugin state,
 *   and registers plugin-list listeners.
 * - UI creation is deferred through `triggerAsyncUpdate()` so window setup
 *   runs on the message thread via `handleAsyncUpdate()`.
 *
 * Main-window MVC wiring occurs in `ensureMainWindow()`:
 * - create `daw::MainAppDataModel` and `daw::MainAppWindow`;
 * - create `daw::MainAppController`;
 * - bind controller to model (`setDataModelShared`) and view
 *   (`addViewShared`), which attaches lifecycle callbacks and model observer
 *   links through the base MVC layer.
 *
 * The same class also supports plugin scanner subprocess mode; when requested
 * by command-line flags, normal UI startup is skipped.
 *
 * @section evil_daw_application_diagrams Diagrams
 * - Sequence diagram: 
 * 
 * @plantumlfile docs/puml/diagrams/EvilDAWLib/application_mvc_sequence.puml
 * 
 * The sequence diagram is organized into four phases:
 * - Compose MVC triad:
 * 
 *   `EvilDAWApplication::ensureMainWindow()` constructs
 *   `MainAppDataModel`, `MainAppWindow`, and `MainAppController`, then binds
 *   them with `setDataModelShared()` and `addViewShared()`.
 * 
 * - View lifecycle routing:
 * 
 *   when the window is shown, `BaseWindowView::visibilityChanged()` triggers
 *   `onInit` (first show) and `onShow`, which are forwarded by
 *   `BaseController::attachViewCallbacks()` into controller hooks.
 * 
 * - Runtime MVC flow:
 * 
 *   user actions are forwarded from view to controller (`handleRequest()`),
 *   controller logic interacts with the model, and model updates notify the
 *   view via observer callbacks.
 * 
 * - Window close path:
 * 
 *   `MainAppWindow::closeButtonPressed()` delegates to
 *   `EvilDAWApplication::systemRequestedQuit()`.
 *
 * The diagram is intended to document interaction wiring and control flow,
 * not object ownership semantics or threading guarantees beyond the visible
 * async update callbacks.
 * @section evil_daw_application_usage Usage Notes
 * - Use `getApplicationInstance()` to access shared services.
 * - Use `getCommandManager()` and `getAudioDeviceManager()` instead of
 *   constructing per-feature managers.
 * - Treat window creation as lazy/deferred (`ensureMainWindow()`), not as an
 *   immediate side effect of object construction.
 */

namespace evil
{
    namespace daw
    {
        class MainAppController;
        class MainAppDataModel;
        class MainAppWindow;
    }

    /**
     * @brief Unique process identifier used for the EvilDAW application.
     *
     * This constant is passed to helper processes (such as the plugin scanner
     * subprocess) to identify the parent EvilDAW process instance.
     */
    constexpr const char* processUID = "evildawapplication";

        /**
         * @brief Concrete JUCE application class that implements EvilDAW runtime behavior.
         *
         * @details
         * `EvilDAWApplication` is the process singleton created by JUCE. The
         * implementation is responsible for:
         * - bootstrapping settings, logger, command manager, and audio device manager;
         * - initializing plugin infrastructure (known list, format manager,
         *   graph holder, custom scanner, persisted plugin sort/state);
         * - handling plugin-list change notifications and persisting updates;
         * - lazily creating and wiring the main MVC window stack via
         *   `ensureMainWindow()`;
         * - managing secondary windows (plugin list and audio settings) with
         *   persistent window state restore/save;
         * - executing orderly shutdown and release of process-owned resources.
         *
         * UI creation is intentionally deferred: `initialise()` schedules
         * `handleAsyncUpdate()`, and `handleAsyncUpdate()` calls
         * `ensureMainWindow()` once the message loop is active.
         *
         * plantumlfile docs/puml/diagrams/application/EvilDAWApplicationComponent.puml "EvilDAW Application Component View"
         * plantumlfile docs/puml/diagrams/application/EvilDAWApplicationLifecycle.puml "EvilDAW Application Lifecycle"
         * plantumlfile docs/puml/diagrams/application/EvilDAWApplicationSequence.puml "EvilDAW Startup Sequence"
         */
    class EvilDAWApplication final : public juce::JUCEApplication,
                                     public juce::ChangeListener,
                                     private juce::AsyncUpdater
    {
    public:
        //==========================================================================
        /**
         * @brief Returns the singleton EvilDAW application instance.
         *
         * @details
         * This helper wraps `juce::JUCEApplication::getInstance()` and performs
         * a `dynamic_cast` to ensure that the current JUCE application object
         * really is an `EvilDAWApplication`. A failed cast will trigger a
         * debug-time assertion.
         *
         * The returned reference can be used to access application-wide services
         * such as the command manager, audio device manager, plugin list, and
         * main window.
         *
         * @return Reference to the global `EvilDAWApplication` instance.
         *
         * @see getMainWindow(), getCommandManager(), getAudioDeviceManager()
         */
        static EvilDAWApplication& getApplicationInstance();

        /**
         * @brief Returns the application's main window.
         *
         */
        static daw::MainAppWindow& getMainWindow();

        /**
         * @brief Returns the shared application command manager.
         *
         * @details
         * Provides access to the process-wide `juce::ApplicationCommandManager`
         * instance owned by `EvilDAWApplication`. The manager is created and
         * configured in `initialiseCommandManager()` during application startup,
         * and is used to register commands, build menus/toolbars, and manage
         * keyboard shortcuts.
         *
         * This static helper is the preferred way for UI components and other
         * subsystems to obtain the shared command manager rather than keeping
         * their own instances.
         *
         * @return Reference to the shared `juce::ApplicationCommandManager`.
         *
         * @see initialiseCommandManager(), getApplicationInstance(), getMainWindow()
         */
        static juce::ApplicationCommandManager& getCommandManager();

        /**
         * @brief Returns the shared audio device manager for the application.
         *
         * @details
         * Provides access to the process-wide `juce::AudioDeviceManager`
         * instance owned by `EvilDAWApplication`. The manager is created and
         * configured in `initialiseDeviceManager()`, typically restoring its
         * state from the persisted `"audioDeviceState"` stored in the user
         * settings.
         *
         * All audio-related components (graph, editors, meters, etc.) should
         * obtain their device manager via this helper to ensure they operate
         * on a single, consistent audio configuration.
         *
         * @return Reference to the global `juce::AudioDeviceManager` instance.
         *
         * @see initialiseDeviceManager(), showAudioSettings(), getApplicationInstance()
         */
        static juce::AudioDeviceManager& getAudioDeviceManager();

    public:
        /**
         * @brief Constructs the EvilDAW application object.
         *
         * @details
         * The constructor performs minimal setup and is intentionally light‑weight.
         * It does not create JUCE subsystems, windows, or devices; those are
         * initialized later in `initialise()`, once the JUCE runtime and message
         * loop are ready.
         *
         * The only notable side effect is selecting the default plugin sort
         * method (`juce::KnownPluginList::sortByManufacturer`), which will be
         * used when building plugin trees and menus until (or unless) the value
         * is overridden from persisted user settings.
         */
        EvilDAWApplication();

        /**
         * @brief Destroys the EvilDAW application object.
         *
         * @details
         * The destructor itself performs no explicit cleanup beyond what is
         * handled by the member smart pointers and JUCE utilities. All
         * subsystems owned by the application (logger, device manager,
         * windows, plugin list, etc.) are expected to be torn down in
         * `shutdown()`, which JUCE calls before the destructor runs.
         *
         * This separation keeps the destructor trivial and ensures that
         * shutdown ordering is driven by the JUCE application lifecycle
         * rather than by object destruction alone.
         */
        ~EvilDAWApplication() override;

        /**
         * @brief Returns the user-visible name of the application.
         *
         * Used by JUCE for window titles, settings file names, and logging.
         *
         * @return A juce::String containing the application name.
         */
        const juce::String getApplicationName() override;

        /**
         * @brief Returns the current application version string.
         *
         * Used primarily for About dialogs and log file headers.
         *
         * @return A juce::String containing the application version.
         */
        const juce::String getApplicationVersion() override;

        /**
         * @brief Indicates whether multiple EvilDAW instances can run at once.
         *
         * EvilDAW currently only supports a single instance, so this returns false.
         *
         * @return true if multiple instances are allowed; false otherwise.
         */
        bool moreThanOneInstanceAllowed() override;

        /**
         * @brief Initializes the EvilDAW application on startup.
         *
         * @details
         * Called once by the JUCE framework after the application object has
         * been constructed. This is the main entry point for setting up all
         * process-wide services and UI state. In order, this method:
         *
         *  - Configures and creates the `juce::ApplicationProperties` store
         *    backing the app/user settings.
         *  - Initializes:
         *      - the file logger (`initialiseLogger()`),
         *      - the command manager (`initialiseCommandManager()`),
         *      - the audio device manager (`initialiseDeviceManager()`).
         *  - Attempts to interpret @p commandLine as a request to run in
         *    dedicated plugin-scanner subprocess mode; if successful,
         *    stores the `PluginScannerSubprocess` and returns early
         *    without creating any UI.
         *  - Creates the `juce::KnownPluginList` and attaches a custom
         *    `CustomPluginScanner`.
         *  - Creates the `juce::AudioPluginFormatManager` and the main
         *    processing graph holder (`GraphDocumentComponent`).
         *  - Loads previously saved plugin list state from user settings
         *    (if available), and registers built‑in `InternalPluginFormat`
         *    types into the known plugin list.
         *  - Restores the plugin sort method from settings and registers
         *    `EvilDAWApplication` as a change listener on the plugin list
         *    so that changes can be persisted eagerly.
         *  - Schedules deferred UI creation by calling `triggerAsyncUpdate()`,
         *    which will later invoke `handleAsyncUpdate()` on the message thread
         *    to create the menu model and main window.
         *
         * This function must return quickly and avoid blocking operations; any
         * long‑running tasks such as plugin scanning are delegated to background
         * mechanisms (e.g. the scanner subprocess).
         *
         * @param commandLine Raw command line string passed to the process.
         *
         * @see shutdown(), handleAsyncUpdate(), initialiseDeviceManager(),
         *      initialiseCommandManager(), initialiseLogger()
         */
        void initialise(const juce::String& commandLine) override;

        /**
         * @brief Performs application-wide teardown during shutdown.
         *
         * @details
         * Called once by JUCE when the message loop is about to terminate.
         * This method is responsible for releasing resources owned by
         * `EvilDAWApplication` in a controlled order. In the current
         * implementation it:
         *
         *  - closes and releases the owned top-level windows;
         *  - removes this object as a change listener from `_knownPluginList`;
         *  - resets `_knownPluginList` and `_audioDeviceManager`;
         *  - releases `_applicationProperties`;
         *  - shuts down logging via `shutdownLogger()`.
         *
         * After `shutdown()` completes, member smart pointers should be cleared
         * so that the subsequent destruction performed by JUCE is trivial and
         * free of ordering dependencies.
         *
         * @see initialise(), shutdownLogger(), changeListenerCallback()
         */
        void shutdown() override;

        /**
         * @brief Handles system-level requests to quit the application.
         *
         * @details
         * Called by the JUCE framework when the operating system or window
         * manager asks the application to terminate (for example, when the
         * user closes the last top‑level window or selects the platform‑specific
         * “Quit” command).
         *
         * The default implementation forwards to JUCE’s `quit()` helper, which
         * initiates the normal shutdown sequence and ultimately leads to
         * `shutdown()` being called. Override or extend this method if the
         * application ever needs to confirm quit requests or perform custom
         * pre‑shutdown logic.
         *
         * @see quit(), shutdown()
         */
        void systemRequestedQuit() override;

        /**
         * @brief Handles notification that another instance of EvilDAW was started.
         *
         * @details
         * Invoked by the JUCE framework when a second instance of the application
         * is launched while an existing instance is already running. The raw
         * @p commandLine of the new instance is provided so the primary instance
         * can decide how to react (e.g. open a project file, bring a window to
         * front, etc.).
         *
         * In the current implementation this method simply ignores the argument
         * and performs no action, effectively preventing multiple concurrent
         * instances without any additional behavior.
         *
         * This is the natural extension point for future enhancements such as:
         *  - focusing or restoring the main window in the primary instance;
         *  - handling custom URI schemes or file open requests;
         *  - implementing simple inter‑process command routing.
         *
         * @param commandLine Command-line string passed to the newly-started
         *                    (secondary) process instance.
         *
         * @see moreThanOneInstanceAllowed()
         */
        void anotherInstanceStarted(const juce::String& commandLine) override;

        /**
         * @brief Builds a configured `juce::PropertiesFile::Options` for a settings file.
         *
         * @details
         * This helper centralizes how EvilDAW chooses the location and naming
         * convention for its properties files. It fills out a
         * `juce::PropertiesFile::Options` struct based on whether the caller
         * is requesting per-project settings or global application settings:
         *
         *  - `applicationName` is set to @p filename.
         *  - `filenameSuffix` is set to `"settings"`.
         *  - `osxLibrarySubFolder` is set to `"Application Support"`.
         *  - `folderName` is:
         *      - `"EvilDAW"` for global settings, or
         *      - `"EvilDAW/ProjectSettings"` when @p isProjectSettings is true.
         *
         * The returned options can then be passed directly to the
         * `juce::PropertiesFile` constructor or to `juce::ApplicationProperties`
         * to create or open the corresponding settings file.
         *
         * @param filename          Logical base name for the properties file
         *                          (without suffix or extension).
         * @param isProjectSettings If true, builds options for a per-project
         *                          settings file; otherwise, for global settings.
         * @return A fully-populated `juce::PropertiesFile::Options` describing
         *         where and how the properties should be stored.
         */
        juce::PropertiesFile::Options getPropertyFileOptionsFor(const juce::String& filename,
                                                                bool isProjectSettings);

    private:
        //==========================================================================
        /**
         * @brief Populates a popup menu with all discoverable plugin types.
         *
         * @details
         * Builds the contents of the "Create Plug-in" submenu using the
         * application’s `juce::KnownPluginList`. The method:
         *
         *  - Inserts a separator at the top of @p m (reserved for potential
         *    internal/graph-related items).
         *  - Copies the known plugin types from `_knownPluginList`.
         *  - Filters out internal plugin types (identified by
         *    `InternalPluginFormat::getIdentifier()`) to avoid showing them
         *    twice.
         *  - Uses `juce::KnownPluginList::createTree()` with the current
         *    `_pluginSortMethod` to build a hierarchical tree of plugins.
         *  - Clears `_pluginDescriptionsAndPreference` and calls `addToMenu()`
         *    to recursively flatten the tree into @p m while filling
         *    `_pluginDescriptionsAndPreference` so that each menu item can be
         *    mapped back to its `PluginDescriptionAndPreference` entry.
         *
         * The menu item IDs created by `addToMenu()` are based on the index
         * within `_pluginDescriptionsAndPreference`, offset by a fixed base,
         * allowing later command handlers to resolve which plugin to instantiate
         * from the selected menu item.
         *
         * @param m Popup menu that will receive plugin items and any submenus.
         *
         * @see addToMenu(), _pluginDescriptionsAndPreference, _knownPluginList
         */
        void addPluginsToMenu(juce::PopupMenu& m);

        /**
         * @brief Handles deferred initialization once the message loop is running.
         *
         * Called automatically by juce::AsyncUpdater after triggerAsyncUpdate()
         * is invoked in initialise(). Responsible for creating:
         *  - Menu model.
         *  - Main window instance.
         */
        void handleAsyncUpdate() override;

        /**
         * @brief Initializes the global application command manager.
         *
         * @details
         * Allocates the process-wide `juce::ApplicationCommandManager` and stores
         * it in `_commandManager`, then registers `EvilDAWApplication` as a
         * command target by calling `registerAllCommandsForTarget(this)`.
         *
         * This:
         *  - exposes all IDs returned by `getAllCommands()` to JUCE, and
         *  - associates their metadata via `getCommandInfo()`,
         *
         *  - enabling them to be used in menus, toolbars and keymaps, and
         *  - to be invoked via `juce::ApplicationCommandManager::invoke()`.
         *
         * The method is intended to be called once from `initialise()`, before
         * any menus, keymaps, or UI components that depend on the command
         * manager are created.
         *
         * @see getCommandManager(), getAllCommands(), getCommandInfo(), perform()
         */
        void initialiseCommandManager();

        /**
         * @brief Initializes the audio device manager.
         *
         * Creates the juce::AudioDeviceManager and initializes it with saved
         * state from application settings (key "audioDeviceState"), or with
         * default devices if no saved state exists. On Windows, this may include
         * ASIO support if available.
         *
         * @note This function performs runtime null checks on the application
         *       properties and settings file before accessing them.
         */
        void initialiseDeviceManager();

        /**
         * @brief Initializes the process-wide file logger.
         *
         * @details
         * Creates a dated log file in the per-user log directory and registers
         * it as JUCE’s current logger. The method:
         *
         *  - Builds a log file name using @p filePrefix and the current date
         *    (e.g. `"EvilDAW_2025-01-01.log"`).
         *  - Ensures the log directory exists (typically inside the user’s
         *    application data folder for EvilDAW).
         *  - Creates a `juce::FileLogger` and stores it in `_logger`.
         *  - Calls `juce::Logger::setCurrentLogger(_logger.get())` so that all
         *    subsequent JUCE `Logger::writeToLog()` calls go to this file.
         *
         * If the logger cannot be created, the function leaves `_logger` as
         * `nullptr`, does not install a logger, and returns `false`.
         *
         * This should be called early in `initialise()` so that startup
         * diagnostics are captured.
         *
         * @param filePrefix Prefix for the log filename, typically the
         *                   application name (without extension).
         * @return true if the logger was created and installed successfully;
         *         false otherwise.
         *
         * @see shutdownLogger(), juce::FileLogger, juce::Logger
         */
        bool initialiseLogger(const char* filePrefix);


        /**
         * @brief Shuts down and detaches the file-based logger.
         *
         * @details
         * Cleans up the logging subsystem by first clearing JUCE’s global
         * logger pointer via `juce::Logger::setCurrentLogger(nullptr)`, and
         * then resetting the `_logger` smart pointer. This flushes and closes
         * the underlying log file created in `initialiseLogger()`, ensuring
         * that no further log messages are written after shutdown.
         *
         * Typically invoked from `shutdown()` as part of the overall
         * application teardown sequence.
         *
         * @see initialiseLogger(), shutdown()
         */
        void shutdownLogger();

        /**
         * @brief Shows or focuses the audio settings window and persists changes.
         *
         * @details
         * Lazily creates an `EvilDAWAudioSettingsEditor` hosted inside a
         * dedicated top-level window bound to the shared `_audioDeviceManager`,
         * allowing the user to inspect and adjust the current audio
         * configuration (device type, sample rate, buffer size, active
         * channels, etc.). The method:
         *
         *  - Verifies that `_audioDeviceManager` and `_applicationProperties`
         *    (and its user settings) are valid; if not, logs a diagnostic
         *    message and returns.
         *  - Creates the window on first use, restores its saved geometry,
         *    and reuses the same window on subsequent calls.
         *  - On window close, re-validates the application instance via a
         *    `juce::WeakReference<EvilDAWApplication>`, serializes the current
         *    device state to XML, writes it to the `"audioDeviceState"` key in
         *    the user settings, saves the settings file, and releases the
         *    window instance.
         *  - If the window is already open, makes it visible, restores it from
         *    a minimized state if necessary, and brings it to the front.
         *
         * All state persistence in the close callback is guarded by weak
         * references and null checks so that no invalid objects are accessed if
         * the application is shutting down while the window is open.
         *
         * @see initialiseDeviceManager(), getAudioDeviceManager()
         */
        void showAudioSettings();

        /**
         * @brief Shows or focuses the plugin list management window.
         *
         * @details
         * Lazily creates and displays the `PluginListWindow` used to manage the
         * contents of `_knownPluginList` (scan paths, enabled/disabled plugins,
         * rescanning, etc.), or brings the existing window to the front if it is
         * already open. The method:
         *
         *  - Verifies that `_pluginFormatManager`, `_knownPluginList`, and
         *    `_applicationProperties` (and its user settings) are valid; if not,
         *    logs a diagnostic message and returns.
         *  - When `_pluginListWindow` is `nullptr`, constructs a new
         *    `PluginListWindow` bound to the shared format manager and known
         *    plugin list.
         *  - Restores the window’s geometry/state from the
         *    `"PluginListWindowState"` key in the user settings, if present.
         *  - Installs a close callback that, on close:
         *      - validates the `EvilDAWApplication` instance using a
         *        `juce::WeakReference<EvilDAWApplication>`,
         *      - persists the updated window state back to
         *        `"PluginListWindowState"`,
         *      - resets `_pluginListWindow` to release the window.
         *  - If `_pluginListWindow` already exists, makes it visible,
         *    restores it from a minimized state if necessary, and brings it
         *    to the front.
         *
         * All access to application and settings state in the close callback is
         * guarded with null checks to avoid dereferencing invalid objects if the
         * application is shutting down while the window is open.
         *
         * @see _knownPluginList, _pluginFormatManager, addPluginsToMenu()
         */
        void showPluginListSettings();

        /** @brief Ensures the main window exists and is visible. */
        void ensureMainWindow();

        /** @brief Ensures the plugin list window exists and is focused. */
        void ensurePluginListWindow();

        /** @brief Ensures the audio settings window exists and is focused. */
        void ensureAudioSettingsWindow();

        /** @brief Closes and releases all top-level windows owned by the app. */
        void closeAllWindows();

        /** @brief Restores a window state string from persistent settings. */
        void restoreWindowState(juce::DocumentWindow& window, const juce::String& stateKey);

        /** @brief Saves a window state string into persistent settings. */
        void saveWindowState(juce::DocumentWindow& window, const juce::String& stateKey) const;

        //==========================================================================
        // Core application state

        /// Cached list of internal plugin types (if any are used separately).
        std::vector<juce::PluginDescription>                    _internalTypes;

        /// Shared list of all known plugin types (internal and external).
        std::unique_ptr<juce::KnownPluginList>                  _knownPluginList;

        /// Current plugin sorting method used for menu/tree construction.
        juce::KnownPluginList::SortMethod                       _pluginSortMethod;

        /// Primary graph holder component for the audio processing graph.
        std::unique_ptr<GraphDocumentComponent>                 _graphHolder;

        /// Flattened mapping of menu entries to plugin descriptions and preferences.
        juce::Array<PluginDescriptionAndPreference>             _pluginDescriptionsAndPreference;

        /// File-based logger instance used for all logging output.
        std::unique_ptr<juce::FileLogger>                       _logger;

        /// Global application properties for storing persistent settings.
        std::unique_ptr<juce::ApplicationProperties>            _applicationProperties;

        /// Application-wide command manager.
        std::unique_ptr<juce::ApplicationCommandManager>        _commandManager;

        /// Application-wide undo/redo command manager.
        ///std::unique_ptr<mvc::ControllerCommandManager>       _undoableCommandManager;

        /// Main window menu bar model.
//        std::unique_ptr<juce::MenuBarModel>                     _menuModel;

        /// Shared audio device manager for the entire application.
        std::unique_ptr<juce::AudioDeviceManager>               _audioDeviceManager;

        /// Manager for all supported audio plugin formats.
        std::unique_ptr<juce::AudioPluginFormatManager>         _pluginFormatManager;

        /// Plugin list editor window (created on demand, kept alive while open).
        std::unique_ptr<PluginListWindow>                       _pluginListWindow;

        /// Audio settings window (created on demand, kept alive while open).
        std::unique_ptr<juce::DocumentWindow>                   _audioSettingsWindow;

        /// Optional plugin scanner subprocess used when launched in scanner mode.
        std::unique_ptr<PluginScannerSubprocess>                _storedScannerSubprocess;

        /// Main JUCE window used for the EvilDAW application UI.
        
        std::shared_ptr<daw::MainAppWindow> _mainAppWindow;
        std::unique_ptr<daw::MainAppController> _mainAppController;
        std::shared_ptr<daw::MainAppDataModel> _mainAppDataModel;

        static juce::ApplicationProperties& getApplicationProperties();

        /// Grant access to the namespace-level helper free function.
        friend juce::ApplicationProperties& getAppProperties();

    private:
        //==========================================================================
        /**
         * @brief Responds to changes in registered ChangeBroadcasters.
         *
         * Currently used to:
         *  - Detect changes in _knownPluginList.
         *  - Persist plugin list changes to application settings on each update.
         *
         * @param changedItem Pointer to the broadcaster that triggered the callback.
         */
        void changeListenerCallback(juce::ChangeBroadcaster* changedItem) override;

        //==========================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EvilDAWApplication)
        JUCE_DECLARE_WEAK_REFERENCEABLE(EvilDAWApplication)
    };

    /**
     * @brief Convenience accessor for the current EvilDAWApplication instance.
     *
     * This function performs a dynamic_cast from the JUCEApplication singleton
     * and returns a reference to the EvilDAWApplication.
     *
     * @return Reference to the global EvilDAWApplication.
     */
    static EvilDAWApplication& getApplication()
    {
        return *dynamic_cast<EvilDAWApplication*> (juce::JUCEApplication::getInstance());
    }

    /**
     * @brief Returns the global application properties object.
     *
     * Shorthand for accessing EvilDAWApplication::_applicationProperties.
     *
     * @return Reference to juce::ApplicationProperties.
     */
    inline juce::ApplicationProperties& getAppProperties()
    {
        return *getApplication()._applicationProperties.get();
    }

} // namespace evil
