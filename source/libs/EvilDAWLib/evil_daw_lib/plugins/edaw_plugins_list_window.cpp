#include "edaw_plugins_list_window.h"
#include "edaw_plugins_list_component.h"
#include "application/EvilDAWApplication.h"

namespace evil
{
    // ------------------------------------------------------------------------------

    PluginListWindow::PluginListWindow(juce::AudioPluginFormatManager& pluginFormatManager,
                                       juce::KnownPluginList& knownPluginList)
        : DocumentWindow("Available Plugins",
            juce::LookAndFeel::getDefaultLookAndFeel().findColour(ResizableWindow::backgroundColourId),
            DocumentWindow::minimiseButton | DocumentWindow::closeButton)
    {
        auto deadMansPedalFile = getAppProperties().getUserSettings()->getFile().getSiblingFile("PluginListWindowSettings");
        
        setContentOwned(new EvilDAWPluginsListComponent(
                pluginFormatManager,
                knownPluginList,
                deadMansPedalFile,
                getAppProperties().getUserSettings(),
                true), 
            true);
            
        setResizable(true, false);
        setResizeLimits(300, 400, 800, 1500);
        setTopLeftPosition(60, 60);
        setVisible(true);
    }

    PluginListWindow::~PluginListWindow()
    {
    }

    void PluginListWindow::setOnCloseRequested(std::function<void()> onCloseRequested)
    {
        onCloseRequested_ = std::move(onCloseRequested);
    }

    // ------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------

    void PluginListWindow::closeButtonPressed()
    {
        setVisible(false);

        if (onCloseRequested_)
        {
            juce::MessageManager::callAsync(onCloseRequested_);
        }
    }

    // ------------------------------------------------------------------------------

}