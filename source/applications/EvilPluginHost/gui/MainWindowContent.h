//==============================================================================
/**
 * @file MainWindowContent.h
 * @brief Main content component for the EvilPluginHost application window
 */
//==============================================================================

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
 * @class MainWindowContent
 * @brief Primary content component that manages the main window UI, menu bar, and application commands
 * 
 * This class serves as the central GUI component for the EvilPluginHost application.
 * It integrates three key JUCE interfaces to provide comprehensive window management:
 * 
 * - ApplicationCommandTarget: Handles application-level commands and keyboard shortcuts
 * - Component: Provides the visual representation and UI rendering
 * - MenuBarModel: Manages the application's menu bar structure and behavior
 * 
 * The class is marked as final to prevent further inheritance and ensure a stable
 * interface for the main window content.
 */
class MainWindowContent final :
    public juce::ApplicationCommandTarget,
    public juce::Component,
    public juce::MenuBarModel
{
    //==============================================================================
    // Inherited via ApplicationCommandTarget
    
    /** @brief Returns the next command target in the command chain */
    ApplicationCommandTarget* getNextCommandTarget() override;
    
    /** @brief Populates the array with all available command IDs */
    void getAllCommands(juce::Array<juce::CommandID>& commands) override;
    
    /** @brief Retrieves detailed information about a specific command */
    void getCommandInfo(juce::CommandID commandID, juce::ApplicationCommandInfo& result) override;
    
    /** @brief Executes the specified command */
    bool perform(const InvocationInfo& info) override;

    //==============================================================================
    // Inherited via MenuBarModel
    
    /** @brief Returns the names of all top-level menu items */
    juce::StringArray getMenuBarNames() override;
    
    /** @brief Constructs the popup menu for a specific top-level menu item */
    juce::PopupMenu getMenuForIndex(int topLevelMenuIndex, const juce::String& menuName) override;
    
    /** @brief Handles selection of a menu item */
    void menuItemSelected(int menuItemID, int topLevelMenuIndex) override;
};
