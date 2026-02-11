#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace evil
{
    class EvilDAWMainWindowContent final :
        public juce::ApplicationCommandTarget,
        public juce::Component,
        public juce::MenuBarModel
    {
    public:

        EvilDAWMainWindowContent();

        enum CommandIDs
        {
            /** Show the audio device settings / setup dialog. */
            audioSetupCommandID = 1,
            /** Exit the application. */
            exitAppCommandID
        };

        void paint(juce::Graphics& graphics) override;
        void resized() override;

    private:
        std::unique_ptr<juce::Component> _mainWindowMenu;

        juce::ApplicationCommandManager _commandManager;

        // Inherited via MenuBarModel
        juce::StringArray getMenuBarNames() override;
        juce::PopupMenu getMenuForIndex(int topLevelMenuIndex, const juce::String& menuName) override;
        void menuItemSelected(int menuItemID, int topLevelMenuIndex) override;

#pragma region Inherited via ApplicationCommandTarget

        ApplicationCommandTarget* getNextCommandTarget() override;
        void getAllCommands(juce::Array<juce::CommandID>& commands) override;
        void getCommandInfo(juce::CommandID commandID, juce::ApplicationCommandInfo& result) override;
        bool perform(const InvocationInfo& info) override;

#pragma endregion

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EvilDAWMainWindowContent)

    };
}
