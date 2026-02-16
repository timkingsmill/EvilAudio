#include "evil_daw_main_window.h"
#include "evil_daw_main_window_content.h"
#include "application/evil_daw_application.h"

namespace evil
{


EvilDAWMainWindow::EvilDAWMainWindow(const juce::String& name, juce::JUCEApplication& application) :
    DocumentWindow(name,
                   juce::Desktop::getInstance().getDefaultLookAndFeel()
                                               .findColour(ResizableWindow::backgroundColourId),
                   DocumentWindow::allButtons),
    _application(application)
{
    setUsingNativeTitleBar(true);

    _mainWindowContent = std::make_unique<EvilDAWMainWindowContent>();
    setContentOwned(_mainWindowContent.get(), true);

    #if JUCE_IOS || JUCE_ANDROID
        setFullScreen(true);
    #else
        setResizable(true, true);
        setResizeLimits(300, 250, 10000, 10000);
        centreWithSize(getWidth(), getHeight());
    #endif

    #if ! JUCE_MAC
        setMenuBar(EvilDAWApplication::getApp().getMenuBarModel());
    #endif

    auto& commandManager = EvilDAWApplication::getCommandManager();
}

EvilDAWMainWindow::~EvilDAWMainWindow() 
{
    storeWindowPosition();

}

// -----------------------------------------------------------------

void EvilDAWMainWindow::closeButtonPressed()
{
    // This is called when the user tries to close this window. Here, we'll just
    // ask the app to quit when this happens, but you can change this to do
    // whatever you need.
    _application.systemRequestedQuit();
}

void EvilDAWMainWindow::restoreWindowPosition()
{
    juce::String windowState;

    //if (currentProject != nullptr)
    //    windowState = currentProject->getStoredProperties().getValue(getProjectWindowPosName());

    auto& p = getGlobalProperties();
    if (windowState.isEmpty()) {
        windowState = getGlobalProperties().getValue("lastMainWindowPos");
    }
    restoreWindowStateFromString(windowState);
}

void EvilDAWMainWindow::storeWindowPosition()
{
    // save the current size and position to our settings file..
    getGlobalProperties().setValue("lastMainWindowPos", getWindowStateAsString());
}

// ToDo - we should probably also save the window position when the app is closed by the OS, or when the user logs out, etc. We can do this by overriding the appropriate methods in the JUCEApplication class, and calling storeWindowPosition() from there.
// ToDo - we should also consider saving the window position more frequently, such as when the user moves or resizes the window, to ensure that we always have the most up-to-date position saved. We can do this by overriding the appropriate methods in the DocumentWindow class, and calling storeWindowPosition() from there.
// ToDo - we should also consider saving the window position for each project separately, so that when the user opens a project, the window is restored to the position it was in when that project was last closed. We can do this by storing the window position in the project's properties, and restoring it from there when the project is opened.
void EvilDAWMainWindow::visibilityChanged()
{
    DocumentWindow::visibilityChanged();
    if (isVisible()) {
        //addToDesktop();
        if (!isMinimised())
            restoreWindowPosition();
    }
}

} // namespace evil
