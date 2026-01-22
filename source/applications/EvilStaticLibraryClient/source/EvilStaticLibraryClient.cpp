#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>


//==============================================================================
class GuiAppApplication final : public juce::JUCEApplication
{

public:
    //==============================================================================
    GuiAppApplication() {}

    //==============================================================================
    void initialise(const juce::String& commandLine) override
    {
    }

    void shutdown() override
    {
    }

    //==============================================================================
    void systemRequestedQuit() override
    {
    }

    void anotherInstanceStarted(const juce::String& commandLine) override
    {
    }

    //==============================================================================
    /*
        This class implements the desktop window that contains an instance of
        our MainComponent class.
    */
private:
    //JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
};


//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION(GuiAppApplication);



//int main(int argc, char *argv[])
//{
//    auto debuggerPresent = juce::juce_isRunningUnderDebugger();
//    auto lib = juce::File();
//    return 0;
//}