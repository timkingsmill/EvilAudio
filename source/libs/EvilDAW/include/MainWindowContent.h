#pragma once

#include "JuceHeader.h"
#include "AudioSettingsComponent.h"
#include "LiveScrollingAudioVisualiser.h"

//==============================================================================

/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/

/*
Pseudocode / Plan (detailed):
1. Add Doxygen-style documentation comments to the `MainComponent` class and all
   publicly visible methods and important private members in the selected range.
2. For each method override (paint, resized, lookAndFeelChanged, etc.) provide:
   - Brief description of purpose.
   - Notes about behavior/ownership where relevant.
3. For the `CommandIDs` enum, document what each command does and any numeric
   considerations.
4. For ApplicationCommandTarget and MenuBarModel overrides, document their role
   in command dispatch and menu construction.
5. For private members exposed in the selected range, document their role and
   ownership semantics (e.g., `std::unique_ptr` usage).
6. Ensure comments are concise, consistent, and suitable for Doxygen generation.
*/

/**
 * @brief Top-level main component for the equalizer example application.
 *
 * This component hosts the UI controls and handles application-level services such
 * as menu construction, command routing and device management. It implements
 * `juce::ApplicationCommandTarget` to integrate with the JUCE command system,
 * `juce::MenuBarModel` to provide the application menu, and `juce::Component` to
 * provide the visible UI.
 */
class MainWindowContent final :
    public juce::ApplicationCommandTarget,
    public juce::Component,
    public juce::MenuBarModel
{
public:
    //==============================================================================
    /**
     * @brief Constructs the main component.
     *
     * Responsible for initializing member objects (audio device manager, command
     * manager, debug UI, and menu bar) and registering application commands.
     */
    MainWindowContent() {
        _audioDeviceManager.reset(new juce::AudioDeviceManager());

        // Setup the command manager.
        setApplicationCommandManagerToWatch(&_commandManager);

        // Set this component to be the target for all commands.
        _commandManager.setFirstCommandTarget(this);

        // Register all commands that this target can perform.
        _commandManager.registerAllCommandsForTarget(this);
        addKeyListener(_commandManager.getKeyMappings());

        // Create and display the main menu.
        _menuBarComponent.reset(new juce::MenuBarComponent(this));
        addAndMakeVisible(_menuBarComponent.get());

        _audioProcessor.reset(new ParametricEqualiserProcessor());
        _parametricEqualizerEditor.reset(_audioProcessor->createEditorIfNeeded());
        addAndMakeVisible(_parametricEqualizerEditor.get());

        _liveScrollingAudioVisualiser.reset(new LiveScrollingAudioVisualiser());
        _audioDeviceManager->addAudioCallback(_liveScrollingAudioVisualiser.get());
        addAndMakeVisible(_liveScrollingAudioVisualiser.get());

        _audioProcessorPlayer.reset(new juce::AudioProcessorPlayer());
        _audioProcessorPlayer->setProcessor(_audioProcessor.get());
        _audioDeviceManager->addAudioCallback(_audioProcessorPlayer.get());

        // Setup the application properties storage.
        juce::PropertiesFile::Options options;
        options.applicationName = "equalizer";
        options.folderName = "EvilAudio";
        options.filenameSuffix = "settings";
        options.osxLibrarySubFolder = "Application Support";
        _applicationProperties.setStorageParameters(options);

        auto userSettings = _applicationProperties.getUserSettings();
        if (userSettings != nullptr)
        {
            // Load the audio device settings from user settings.
            /**
             * @brief Load and apply previously saved audio device configuration.
             *
             * Reads the string value stored under the user-settings key
             * "AudioDeviceSettings", attempts to parse it into an XML element via
             * `juce::XmlDocument::parse`, and forwards the resulting `XmlElement*`
             * to `juce::AudioDeviceManager::initialise`.
             *
             * Notes:
             * - `juce::XmlDocument::parse` returns a `std::unique_ptr<juce::XmlElement>`.
             *   The pointer may be `nullptr` if the stored string is empty or malformed.
             * - The call `_audioDeviceManager->initialise(2, 2, xmlState.get(), false)`
             *   will receive `nullptr` when parsing failed; `initialise` will then
             *   use default device settings. If different behaviour is required,
             *   add explicit null handling and error logging here.
             * - Parameters `2, 2` represent the desired number of input and output
             *   channels for the manager initialisation.
             *
             * TODO:
             * - Validate the parsed XML contents before calling `initialise`.
             * - Add logging for parse failures and consider a non-blocking initialisation
             *   strategy if platform/device drivers can block during construction.
             */
            auto audioDeviceSettings = userSettings->getValue("AudioDeviceSettings");
            {
                auto xmlState = juce::XmlDocument::parse(audioDeviceSettings);
                {
                    // Initialize the audio device manager with the saved settings.
                    // Note: xmlState.get() may be nullptr (parse failure or empty value).
                    _audioDeviceManager->initialise(2, 2, xmlState.get(), false);
                }
            }

            // Load the audio processor state using the processors setStateInformation() method.
            auto equalizerSettings = userSettings->getValue("AudioProcessorState");
            {
                const auto& xmlState = juce::XmlDocument::parse(equalizerSettings);
                if (xmlState != nullptr)
                {
                    auto memoryBlock = juce::MemoryBlock();
                    {
                        _audioProcessor->copyXmlToBinary(*xmlState, memoryBlock);
                        _audioProcessor->setStateInformation(memoryBlock.getData(),
                                                                static_cast<int>(memoryBlock.getSize()));
                    }
                }
            }
        }

        setWantsKeyboardFocus(true);
        setOpaque(true);
        setSize(800, 400);
    };

    /**
     * @brief Virtual destructor.
     *
     * Ensures proper teardown of JUCE objects and owned pointers. Marked `override`
     * to reflect override of `juce::Component` destructor semantics.
     */
    ~MainWindowContent() override {
        juce::PropertiesFile* userSettings = _applicationProperties.getUserSettings();
        if (userSettings != nullptr)
        {
            // Save the audio device settings to user properties.
            auto xmlState = _audioDeviceManager.get()->createStateXml();
            if (xmlState != nullptr)
            {
                userSettings->setValue("AudioDeviceSettings", xmlState->toString());
            }

            // Save the audio processor state using the processors getStateInformation() method.
            // This is the preferred way to save plugin state.
            auto memoryBlock = juce::MemoryBlock();
            {
                _audioProcessor->getStateInformation(memoryBlock);
                if (memoryBlock.getSize() != 0)
                {
                    auto xmlElement = _audioProcessor->getXmlFromBinary(static_cast<const char*>(memoryBlock.getData()), size_t(memoryBlock.getSize()));
                    userSettings->setValue("AudioProcessorState", xmlElement->toString());
                }
            }
            userSettings->saveIfNeeded();
        }

        _commandManager.setFirstCommandTarget(nullptr);
        _menuBarComponent->setModel(nullptr);
        
        _audioDeviceManager->removeAudioCallback(_audioProcessorPlayer.get());
        _audioDeviceManager->removeAudioCallback(_liveScrollingAudioVisualiser.get());

        _audioProcessorPlayer = nullptr;

        // Notify the processor that its editor is being deleted.
        _audioProcessor->editorBeingDeleted(_parametricEqualizerEditor.get());
        // Delete the editor component.
        _parametricEqualizerEditor = nullptr;
        
        _audioProcessor = nullptr;
    };

    /**
     * @brief Paints this component.
     *
     * Called by JUCE when the component needs to be repainted. Implementations
     * should draw the component background and any custom graphics here.
     *
     * @param graphics Graphics context used for drawing.
     */
    void paint(juce::Graphics& graphics) override {
        // (Our component is opaque, so we must completely fill 
        // the background with a solid colour)
        graphics.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    };

    /**
     * @brief Called when this component is resized.
     *
     * Use this to lay out child components and respond to size changes.
     */
    void resized() override {
        // This is called when the MainComponent is resized.
        // If you add any child components, this is where you should
        // update their positions.
        auto bounds = getLocalBounds();
        {
            _menuBarComponent->setBounds(bounds.removeFromTop(juce::LookAndFeel::getDefaultLookAndFeel()
                .getDefaultMenuBarHeight()));

            _liveScrollingAudioVisualiser->setBounds(bounds.removeFromBottom(32));
            _parametricEqualizerEditor->setBounds(bounds);
        }
    };

    /**
     * @brief Identifiers for application commands exposed by this component.
     *
     * These IDs are used with the `juce::ApplicationCommandManager` to register
     * and invoke commands from menus, keyboard shortcuts, and other UI sources.
     */
    enum CommandIDs
    {
        /** Show the audio device settings / setup dialog. */
        audioSetupCommandID = 1,

        /** Exit the application. */
        exitAppCommandID
    };

    //==============================================================================
private:
    /**
     * @brief Show a dialog window used by the application (e.g., audio settings).
     *
     * Creates and displays a `juce::DialogWindow` instance, storing a `SafePointer`
     * so the dialog can be tracked and safely accessed without creating ownership
     * cycles.
     */
    void showDialogWindow() {
        juce::String m;

        m << "Dialog Windows can be used to quickly show a component, usually blocking mouse input to other windows."
            << juce::newLine
            << juce::newLine
            << "They can also be quickly closed with the escape key, try it now.";

        juce::DialogWindow::LaunchOptions options;

        // Create the content component to go inside the dialog window.
        AudioSettingsComponent* audioSettingsComponent = new AudioSettingsComponent(*_audioDeviceManager);
        // The dialog takes ownership of the content component, so use setOwned().
        options.content.setOwned(audioSettingsComponent);
        options.dialogTitle = "Audio Settings";
        //options.dialogBackgroundColour = juce::Colour(0xff0e345a);
        options.escapeKeyTriggersCloseButton = true;
        //options.useNativeTitleBar = false;
        options.resizable = true;

        _dialogWindow = options.launchAsync();

        if (_dialogWindow != nullptr)
        {
            juce::Rectangle bounds = audioSettingsComponent->getBounds();
            _dialogWindow->centreAroundComponent(this, bounds.getWidth(), bounds.getHeight());
        }
    };


    // Inherited via ApplicationCommandTarget
    /**
     * @brief Returns the next command target in the command routing chain.
     *
     * Used by JUCE to propagate commands to parent components or application-level
     * handlers if this component does not handle them.
     *
     * @return Pointer to the next `ApplicationCommandTarget`, or nullptr.
     */
    ApplicationCommandTarget* getNextCommandTarget() override {
        return nullptr;
    };

    /**
     * @brief Populates `commands` with the set of command IDs this target supports.
     *
     * JUCE calls this to determine which commands to query for `getCommandInfo`.
     *
     * @param commands Array to be filled with supported `juce::CommandID` values.
     */
    void getAllCommands(juce::Array<juce::CommandID>& commands) override {
        juce::Array<juce::CommandID> c {
            CommandIDs::audioSetupCommandID,
            CommandIDs::exitAppCommandID
        };
        commands.addArray(c);
    };

    /**
     * @brief Provides metadata (name, description, default keypress) for a command.
     *
     * Used by application menus and key mappings to show human-readable information
     * about a command.
     *
     * @param commandID The command identifier to describe.
     * @param result Output parameter to be populated with command information.
     */
    void getCommandInfo(juce::CommandID commandID, juce::ApplicationCommandInfo& result) override {
        switch (commandID)
        {
            case CommandIDs::audioSetupCommandID:
                result.setInfo("AudioSetup", "Displays the audio options window", "FileMenu", 0);
                result.addDefaultKeypress('a', juce::ModifierKeys::shiftModifier);
                break;
            case CommandIDs::exitAppCommandID:
                result.setInfo("Exit", "Exit the application", "FileMenu", 0);
                result.addDefaultKeypress('q', juce::ModifierKeys::shiftModifier);
                break;
            default:
                break;
        }
    };

    /**
     * @brief Execute a command identified by `info`.
     *
     * Performs the action associated with the requested command and returns whether
     * it was successfully handled.
     *
     * @param info Invocation information containing the command id and any parameters.
     * @return true if the command was handled, false otherwise.
     */
    bool perform(const InvocationInfo& info) override {
        switch (info.commandID)
        {
        case CommandIDs::audioSetupCommandID:
            showDialogWindow();
            //_audioSettingsComponent.showAudioSettingsDialog();
            break;
        case CommandIDs::exitAppCommandID:
            juce::JUCEApplication::getInstance()->systemRequestedQuit();
            break;
        default:
            break;
        }
        return true;
    };

    // Inherited via MenuBarModel
    /**
     * @brief Returns the array of top-level menu names for the menu bar.
     *
     * The returned list will be used by `juce::MenuBarComponent` to construct menus.
     *
     * @return A `juce::StringArray` of menu names (e.g., "File", "Edit", ...).
     */
    juce::StringArray getMenuBarNames() override {
        return { "File", "Options", "Help" };
    };

    /**
     * @brief Returns the menu contents for the menu at `menuIndex`.
     *
     * Build and return a `juce::PopupMenu` for the requested index. The `menuName`
     * can be used to disambiguate or localize content.
     *
     * @param menuIndex Index of the menu requested.
     * @param menuName Localized menu name for reference.
     * @return A `juce::PopupMenu` instance describing the menu items.
     */
    juce::PopupMenu getMenuForIndex(int menuIndex, const juce::String& menuName) override {
        juce::PopupMenu menu;
        if (menuName == "File")
        {
            menu.addCommandItem(&_commandManager, CommandIDs::audioSetupCommandID);
            menu.addSeparator();
            menu.addCommandItem(&_commandManager, CommandIDs::exitAppCommandID);
        }
        else if (menuName == "Options")
        {
            // Add options menu items here.
        }
        else if (menuName == "Help")
        {
            // Add help menu items here.
        }
        return menu;
    };

    /**
     * @brief Called when a menu item is selected.
     *
     * Receives the selected `menuItemID` and the index of the menu that produced it.
     *
     * @param menuItemID Identifier for the selected menu item (typically CommandID).
     * @param menuIndex Index of the menu that contained the item.
     */
    void menuItemSelected(int menuItemID, int menuIndex) override {};

    // Inherited via Component
    /**
     * @brief Called when the look-and-feel for the component changes.
     *
     * Override this to update any cached fonts, colours or to re-parent child
     * components that depend on the current look-and-feel.
     */
    void lookAndFeelChanged() override {};

private:

    //==============================================================================
    /**
     * @brief Stores application-wide properties (persisted settings).
     *
     * `juce::ApplicationProperties` provides access to a properties file and
     * allows the app to save/recall user preferences across runs.
     */
    juce::ApplicationProperties _applicationProperties;

    std::unique_ptr<LiveScrollingAudioVisualiser> _liveScrollingAudioVisualiser;
    std::unique_ptr<juce::AudioProcessorEditor> _parametricEqualizerEditor;
    std::unique_ptr<ParametricEqualiserProcessor> _audioProcessor;

    /**
     * @brief Manages audio device configuration and callbacks.
     *
     * Responsible for opening/closing audio devices and providing audio I/O to
     * the rest of the application.
     * `std::unique_ptr` expresses exclusive ownership.
     * The `AudioDeviceManager` is created and destroyed along with the `MainComponent`.
     */
    std::unique_ptr<juce::AudioDeviceManager> _audioDeviceManager;

    std::unique_ptr<juce::AudioProcessorPlayer> _audioProcessorPlayer;

    /**
    * @brief The menu bar component shown at the top of the application window.
    *
    * Owned by this component and constructed from the `MenuBarModel` implemented
    * by this class.
    */
    std::unique_ptr<juce::MenuBarComponent> _menuBarComponent;

    /**
        * @brief Manages application commands and keyboard mappings.
        *
        * Registers commands exposed by `MainComponent` and routes invocations coming
        * from the menu bar, keypresses or other UI elements.
        */
    juce::ApplicationCommandManager _commandManager;

    /**
        * @brief Safe pointer to an optional dialog window created by this component.
        *
        * `SafePointer` allows the code to refer to the dialog without creating a
        * strong ownership link; it becomes `nullptr` automatically if the dialog is destroyed.
        */
    juce::Component::SafePointer<juce::DialogWindow> _dialogWindow;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindowContent)

};


