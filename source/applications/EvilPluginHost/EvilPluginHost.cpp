#include <JuceHeader.h>
#include "gui/MainWindowContent.h"

//==============================================================================
/*
    This class implements the main application object that is started up when
    the app launches.
*/
class EvilPluginHostApplication final : public juce::JUCEApplication
{
public:
    EvilPluginHostApplication() {};

    const juce::String getApplicationName() override { return ProjectInfo::projectName; }
    const juce::String getApplicationVersion() override { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override { return false; }

    void initialise(const juce::String& commandLine) override {
        // This method is where you should put your application's initialisation code..
        juce::ignoreUnused(commandLine);
        _mainWindow.reset(new MainWindow(
            getApplicationName(),
            *this));
    };

    void shutdown() override {
        // Application's shutdown code here..
    };

    void systemRequestedQuit() override {
        // This is called when the app is being asked to quit: you can ignore this
        // request and let the app carry on running, or call quit() to allow the app to close.
        quit();
    };

    void anotherInstanceStarted(const juce::String& commandLine) override {
        // When another instance of the app is launched while this one is running,
        // this method is invoked, and the commandLine parameter tells you what
        // the other instance's command-line arguments were.
        juce::ignoreUnused(commandLine);
    };

    //==============================================================================
    /*
        This class implements the desktop window that contains an instance of
        our MainComponent class.

        Note:
            Be careful if you override any DocumentWindow methods - the base
            class uses a lot of them, so by overriding you might break its functionality.
            It's best to do all your work in your content component instead, but if
            you really have to override any DocumentWindow methods, make sure your
            subclass also calls the superclass's method.
    */
    class MainWindow final : public juce::DocumentWindow
    {
    public:
        explicit MainWindow(const juce::String& name,
            JUCEApplication& application)
            : DocumentWindow(name,
                juce::Desktop::getInstance().getDefaultLookAndFeel()
                .findColour(ResizableWindow::backgroundColourId),
                DocumentWindow::allButtons),
            _application(application)
        {
            setUsingNativeTitleBar(true);
            _mainWindowContent.reset(new MainWindowContent());
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

        void closeButtonPressed() override {
            // This is called when the user tries to close this window. Here, we'll just
            // ask the app to quit when this happens, but you can change this to do
            // whatever you need.
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

    private:
        const JUCEApplication& _application;
        std::unique_ptr<juce::Component> _mainWindowContent;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
    };

private:
    std::shared_ptr<juce::DocumentWindow> _mainWindow;
};

// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION(EvilPluginHostApplication)
