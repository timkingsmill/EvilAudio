#include <JuceHeader.h>
#include "content/MainWindowContent.h"

class EvilLookAndFeelApplication final : public juce::JUCEApplication
{
    public:
        EvilLookAndFeelApplication() {
            juce::PropertiesFile::Options options;
            options.applicationName = getApplicationName();
            options.filenameSuffix = ".settings";
            options.osxLibrarySubFolder = "Application Support";
            #if JUCE_LINUX || JUCE_BSD
                options.folderName = "~/.config";
            #else
                options.folderName = "";
            #endif
            _appProperties.setStorageParameters(options);
        };

        // We inject these as compile definitions from the CMakeLists.txt
        // If you've enabled the juce header with `juce_generate_juce_header(<thisTarget>)`
        // you could `#include <JuceHeader.h>` and use `ProjectInfo::projectName` etc. instead.
        const juce::String getApplicationName() override       { return ProjectInfo::projectName; }
        const juce::String getApplicationVersion() override    { return ProjectInfo::versionString; }
        bool moreThanOneInstanceAllowed() override             { return false; }

        void initialise(const juce::String& commandLine) override {
            // This method is where you should put your application's initialisation code..
            juce::ignoreUnused(commandLine);
            _mainWindow.reset(new MainWindow(
                getApplicationName(),
                *this));
            #if JUCE_STANDALONE_FILTER_WINDOW_USE_KIOSK_MODE
                juce::Desktop::getInstance().setKioskModeComponent(_mainWindow.get(), false);
            #endif
            _mainWindow->setVisible(true);
        };

        void shutdown() override {
            _mainWindow = nullptr;
            _appProperties.saveIfNeeded();
        };

        void systemRequestedQuit() override {
            if (juce::ModalComponentManager::getInstance()->cancelAllModalComponents())
            {
                juce::Timer::callAfterDelay(100, []()
                    {
                        if (auto app = JUCEApplicationBase::getInstance())
                            app->systemRequestedQuit();
                    });
            }
            else
            {
                quit();
            }
        };

        void anotherInstanceStarted (const juce::String& commandLine) override {
            // When another instance of the app is launched while this one is running,
            // this method is invoked, and the commandLine parameter tells you what
            // the other instance's command-line arguments were.
            juce::ignoreUnused (commandLine);
        };
    
    private:
        std::shared_ptr<juce::DocumentWindow> _mainWindow;
        juce::ApplicationProperties _appProperties;

        class MainWindow final : public juce::DocumentWindow
        {
        public:
            explicit MainWindow(const juce::String& name, JUCEApplication& application) :
                DocumentWindow(name, juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(ResizableWindow::backgroundColourId),
                DocumentWindow::allButtons),
                _application(application)
            {
                setUsingNativeTitleBar(true);
                _mainWindowContent.reset(new LookAndFeelMainWindowContent());
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

};

// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION(EvilLookAndFeelApplication)



