#include "evil_daw_application_menu_model.h"
#include "evil_daw_application.h"

namespace evil
{

    void EvilDAWApplicationMenuModel::menuItemSelected(int menuItemID, int topLevelMenuIndex)
    { 
    }

    juce::StringArray EvilDAWApplicationMenuModel::getMenuBarNames()
    {
        return { "File", "Edit", "View", "Window", "Document", "Tools", "Help" };
    }

    juce::PopupMenu EvilDAWApplicationMenuModel::getMenuForIndex(int menuIndex, const juce::String& menuName)
    {
        // Use a pointer to avoid copying the singleton application instance
        auto& app = EvilDAWApplication::getApp();
        GFG_Function(app); // Call the friend function with the application instance
        return juce::PopupMenu();
    }
}