#include "evil_daw_main_window.h"
#include "evil_daw_main_window_content.h"

namespace evil
{


EvilDAWMainWindow::EvilDAWMainWindow(const juce::String& name, juce::JUCEApplication& application)
    : DocumentWindow(name,
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
    setVisible(true);
}

EvilDAWMainWindow::~EvilDAWMainWindow() {

};

void EvilDAWMainWindow::closeButtonPressed()
{
    // This is called when the user tries to close this window. Here, we'll just
    // ask the app to quit when this happens, but you can change this to do
    // whatever you need.
    _application.systemRequestedQuit();
}


} // namespace evil
