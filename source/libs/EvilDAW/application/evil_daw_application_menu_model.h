#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
//#include "evil_daw_application.h"

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

        // Change to static so it can be a friend in EvilDAWApplication
        static void GFG_Function(EvilDAWApplication& app);

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


