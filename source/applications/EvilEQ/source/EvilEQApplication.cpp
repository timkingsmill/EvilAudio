#include <JuceHeader.h>
#include <juce_audio_plugin_client/Standalone/juce_StandaloneFilterWindow.h>

// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ParametricEqualiserProcessor();
}

class EvilEQApplication final : public juce::JUCEApplication
{
    public:
        EvilEQApplication() {
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
            _mainWindow.reset(createWindow());
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
    
        juce::StandaloneFilterWindow* createWindow()
        {
            #ifdef JucePlugin_PreferredChannelConfigurations
                StandalonePluginHolder::PluginInOuts channels[] = { JucePlugin_PreferredChannelConfigurations };
            #endif
            return new juce::StandaloneFilterWindow(
                getApplicationName(),
                juce::LookAndFeel::getDefaultLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId),
                _appProperties.getUserSettings(),
                false, {}, nullptr
                #ifdef JucePlugin_PreferredChannelConfigurations
                    , juce::Array<StandalonePluginHolder::PluginInOuts>(channels, juce::numElementsInArray(channels))
                #else
                    , {}
                #endif
                    #if JUCE_DONT_AUTO_OPEN_MIDI_DEVICES_ON_MOBILE
                        , false
                #endif
            );
        }

    private:
      std::shared_ptr<juce::DocumentWindow> _mainWindow;
      juce::ApplicationProperties _appProperties;

};

// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION(EvilEQApplication)



