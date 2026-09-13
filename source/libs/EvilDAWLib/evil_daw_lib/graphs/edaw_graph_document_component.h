#pragma once            

#include <juce_audio_formats.h>
#include <juce_audio_processors.h>
#include <juce_gui_basics.h>

namespace evil
{

class GraphDocumentComponent final : public juce::Component,
                                     public juce::DragAndDropTarget,
                                     public juce::DragAndDropContainer,
                                     private juce::ChangeListener
{
public:
    GraphDocumentComponent(juce::AudioPluginFormatManager& formatManager,
                           juce::AudioDeviceManager& deviceManager,
                           juce::KnownPluginList& knownPluginList)
        : _formatManager(formatManager),
          _deviceManager(deviceManager),
          _knownPluginList(knownPluginList)
    {
    }

    //==============================================================================
    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::black);
    }

    void resized() override
    {
    }

    // DragAndDropTarget interface
    bool isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails&) override { return false; }
    void itemDragEnter(const juce::DragAndDropTarget::SourceDetails&) override {}
    void itemDragMove(const juce::DragAndDropTarget::SourceDetails&) override {}
    void itemDragExit(const juce::DragAndDropTarget::SourceDetails&) override {}
    void itemDropped(const juce::DragAndDropTarget::SourceDetails&) override {}

private:
    void changeListenerCallback(juce::ChangeBroadcaster*) override {}

    juce::AudioPluginFormatManager& _formatManager;
    juce::AudioDeviceManager& _deviceManager;
    juce::KnownPluginList& _knownPluginList;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GraphDocumentComponent)
};

}
