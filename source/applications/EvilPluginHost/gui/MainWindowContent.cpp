#include "MainWindowContent.h"

juce::ApplicationCommandTarget* MainWindowContent::getNextCommandTarget()
{
    return nullptr;
}

void MainWindowContent::getAllCommands(juce::Array<juce::CommandID>& commands)
{
}

void MainWindowContent::getCommandInfo(juce::CommandID commandID, juce::ApplicationCommandInfo& result)
{
}

bool MainWindowContent::perform(const InvocationInfo& info)
{
    return false;
}

juce::StringArray MainWindowContent::getMenuBarNames()
{
    return juce::StringArray();
}

juce::PopupMenu MainWindowContent::getMenuForIndex(int topLevelMenuIndex, const juce::String& menuName)
{
    return juce::PopupMenu();
}

void MainWindowContent::menuItemSelected(int menuItemID, int topLevelMenuIndex)
{
}
