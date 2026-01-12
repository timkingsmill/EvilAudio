#pragma once

#include "JuceHeader.h"

class MainWindowContent final :
    public juce::ApplicationCommandTarget,
    public juce::Component,
    public juce::MenuBarModel
{
    // Inherited via ApplicationCommandTarget
    ApplicationCommandTarget* getNextCommandTarget() override;
    void getAllCommands(juce::Array<juce::CommandID>& commands) override;
    void getCommandInfo(juce::CommandID commandID, juce::ApplicationCommandInfo& result) override;
    bool perform(const InvocationInfo& info) override;

    // Inherited via MenuBarModel
    juce::StringArray getMenuBarNames() override;
    juce::PopupMenu getMenuForIndex(int topLevelMenuIndex, const juce::String& menuName) override;
    void menuItemSelected(int menuItemID, int topLevelMenuIndex) override;
};
