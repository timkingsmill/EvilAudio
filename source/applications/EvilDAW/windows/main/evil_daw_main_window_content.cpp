#include "evil_daw_main_window_content.h"
#include "evil_daw_main_window_menu.h"

namespace evil
{

    EvilDAWMainWindowContent::EvilDAWMainWindowContent() 
    {
        // Setup the command manager.
        setApplicationCommandManagerToWatch(&_commandManager);
        _commandManager.setFirstCommandTarget(this);
        // Register all commands that this target can perform.
        _commandManager.registerAllCommandsForTarget(this);
        addKeyListener(_commandManager.getKeyMappings());

        // Create the menu bar and add it to the main window.
        // Pass this as the model to the menu bar, so that it can get callbacks when menu items are selected.
        _mainWindowMenu.reset(new MainWindowMenuBar(this));
        addAndMakeVisible(_mainWindowMenu.get());
    
        setWantsKeyboardFocus(true);
        setOpaque(true);
        setSize(800, 400);

    };

    void EvilDAWMainWindowContent::EvilDAWMainWindowContent::paint(juce::Graphics& graphics) {
        // (Our component is opaque, so we must completely fill 
        // the background with a solid colour)
        graphics.fillAll(juce::Colours::aquamarine);//getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    }

    void EvilDAWMainWindowContent::resized() 
    {
        // This is called when the MainComponent is resized.
        // If you add any child components, this is where you should
        // update their positions.
        auto bounds = getLocalBounds();
        {
            _mainWindowMenu->setBounds(bounds.removeFromTop(
                juce::LookAndFeel::getDefaultLookAndFeel().getDefaultMenuBarHeight()));

            //_liveScrollingAudioVisualiser->setBounds(bounds.removeFromBottom(32));
            //_parametricEqualizerEditor->setBounds(bounds);
        }

    }

    juce::StringArray EvilDAWMainWindowContent::getMenuBarNames()
    {
        return { "File", "Options", "Help" };
    }

    juce::PopupMenu EvilDAWMainWindowContent::getMenuForIndex(int topLevelMenuIndex, const juce::String& menuName)
    {
        juce::PopupMenu menu;
        if (menuName == "File")
        {
            menu.addCommandItem(&_commandManager, CommandIDs::audioSetupCommandID);
            menu.addSeparator();
            menu.addCommandItem(&_commandManager, CommandIDs::exitAppCommandID);
        }
        else if (menuName == "Options")
        {
            juce::PopupMenu audioSubMenu;
            audioSubMenu.addCommandItem(&_commandManager, CommandIDs::audioSetupCommandID);
            audioSubMenu.addSeparator();
            audioSubMenu.addCommandItem(&_commandManager, CommandIDs::exitAppCommandID);

            menu.addSubMenu("Audio", audioSubMenu);

            // Add options menu items here.
            //menu.addCommandItem(&_commandManager, CommandIDs::audioSetupCommandID);
            //menu.addSeparator();
            //menu.addCommandItem(&_commandManager, CommandIDs::exitAppCommandID);
        }
        else if (menuName == "Help")
        {
            // Add help menu items here.
            menu.addCommandItem(&_commandManager, CommandIDs::audioSetupCommandID);
            menu.addSeparator();
            menu.addCommandItem(&_commandManager, CommandIDs::exitAppCommandID);
        }
        repaint();
        return menu;
    }

    void EvilDAWMainWindowContent::menuItemSelected(int menuItemID, int topLevelMenuIndex)
    {
    }

    #pragma region ApplicationCommandTarget implementation

    juce::ApplicationCommandTarget* EvilDAWMainWindowContent::getNextCommandTarget()
    {
        return nullptr;
    }

    void EvilDAWMainWindowContent::getAllCommands(juce::Array<juce::CommandID>& commands)
    {
        juce::Array<juce::CommandID> c { CommandIDs::audioSetupCommandID,
                                         CommandIDs::exitAppCommandID
        };
        commands.addArray(c);
    }

    void EvilDAWMainWindowContent::getCommandInfo(juce::CommandID commandID, juce::ApplicationCommandInfo& result)
    {
        switch (commandID)
        {
        case CommandIDs::audioSetupCommandID:
            result.setInfo("AudioSetup", "Displays the audio options window", "FileMenu", 0);
            result.addDefaultKeypress('a', juce::ModifierKeys::shiftModifier);
            result.setActive(false);
            break;
        case CommandIDs::exitAppCommandID:
            result.setInfo("Exit", "Exit the application", "FileMenu", 0);
            result.addDefaultKeypress('q', juce::ModifierKeys::shiftModifier);
            break;
        default:
            break;
        }
    }

    bool EvilDAWMainWindowContent::perform(const InvocationInfo& info)
    {
        repaint();
        return true;
    }

    #pragma endregion

} // namespace evil
