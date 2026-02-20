#include "evil_daw_application_menu_model.h"
#include "evil_daw_application.h"

namespace evil
{

    void EvilDAWApplicationMenuModel::menuItemSelected(int menuItemID, int topLevelMenuIndex)
    { 
    }

    juce::PopupMenu EvilDAWApplicationMenuModel::getMenuForIndex(int menuIndex, const juce::String& menuName)
    {
        // Use a pointer to avoid copying the singleton application instance
        auto& app = EvilDAWApplication::getApp();
        // Call the friend function to get the menu for the specified index and name
        return app.getMenuForIndex(menuIndex, menuName); 
    }

    juce::StringArray EvilDAWApplicationMenuModel::getMenuBarNames()
    {
        // Use a pointer to avoid copying the singleton application instance
        auto& app = EvilDAWApplication::getApp();
        return app.getMenuBarNames();
    }

}