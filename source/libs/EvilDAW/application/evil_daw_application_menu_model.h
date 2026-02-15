#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace evil
{
    class EvilDAWApplication; // Forward declaration

    /**
     * @class EvilDAWApplicationMenuModel
     * @brief Menu model for the EvilDAW application.
     *
     * This class defines the structure and behavior of the application's menu system.
     * It inherits from juce::MenuBarModel and implements the necessary methods to
     * provide menu items and handle menu actions.
     */
    class EvilDAWApplicationMenuModel : public juce::MenuBarModel
    {
    public:
        /**
         * @brief Default constructor for the menu model.
         */
        EvilDAWApplicationMenuModel() = default;
        /**
         * @brief Destructor for the menu model.
         */
        ~EvilDAWApplicationMenuModel() override = default;

        /**
         * @brief Handles the selection of a menu item.
         * @param menuItemID The ID of the selected menu item.
         * @param topLevelMenuIndex The index of the top-level menu that contains the selected item.
         */
        void menuItemSelected(int menuItemID, int topLevelMenuIndex) override;

        /**
         * @brief Retrieves the menu bar names.
         * @return An array of juce::String containing the names of the menu bars.
         */
        juce::StringArray getMenuBarNames() override;
        /**
         * @brief Retrieves the menu for a given menu bar name.
         * @param menuName The name of the menu bar for which to retrieve the menu.
         * @return A pointer to a juce::PopupMenu representing the requested menu, or nullptr if not found.
         */
        juce::PopupMenu getMenuForIndex(int menuIndex, const juce::String& menuName) override;
    };
} // namespace evil


