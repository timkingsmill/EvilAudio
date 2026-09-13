#pragma once

#include <juce_events.h>
#include <juce_audio_processors.h>

namespace evil
{
    class PluginScannerSubprocess : private juce::ChildProcessWorker,
                                    private juce::AsyncUpdater
    {
    public:
        PluginScannerSubprocess();
        ~PluginScannerSubprocess();

        using ChildProcessWorker::initialiseFromCommandLine;
    private:
        void handleMessageFromCoordinator(const juce::MemoryBlock& mb) override;
        void handleConnectionLost() override;
        void handleAsyncUpdate() override;
        juce::OwnedArray<juce::PluginDescription> doScan(const juce::MemoryBlock& block);
        void sendResults(const juce::OwnedArray<juce::PluginDescription>& results);

        std::mutex _mutex;
        std::queue<juce::MemoryBlock> _pendingBlocks;
        juce::AudioPluginFormatManager _formatManager;

    };
}


