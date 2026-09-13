#include "edaw_plugin_scanner_subprocess.h"

namespace evil
{

    PluginScannerSubprocess::PluginScannerSubprocess()
    {
        {
            addDefaultFormatsToManager(_formatManager);
        }
    }

    PluginScannerSubprocess::~PluginScannerSubprocess()
    {
    }

    void PluginScannerSubprocess::handleMessageFromCoordinator(const juce::MemoryBlock& mb)
    {
        if (mb.isEmpty())
            return;

        const std::lock_guard<std::mutex> lock(_mutex);

        if (const auto results = doScan(mb); !results.isEmpty())
        {
            sendResults(results);
        }
        else
        {
            _pendingBlocks.emplace(mb);
            triggerAsyncUpdate();
        }
    }

    /**
     * @brief Handles loss of connection to the coordinator process.
     * 
     * Terminates the subprocess when the connection to the coordinator is lost.
     */
    void PluginScannerSubprocess::handleConnectionLost()
    {
        juce::JUCEApplicationBase::quit();
    }

    /**
     * @brief Processes pending scan requests asynchronously.
     * 
     * Retrieves and processes the next pending scan request from the queue.
     * If scanning produces no results, the request is re-queued and the async
     * update is triggered again.
     */
    void PluginScannerSubprocess::handleAsyncUpdate()
    {
        juce::MemoryBlock block;
        {
            const std::lock_guard<std::mutex> lock(_mutex);
            if (_pendingBlocks.empty())
                return;
            block = _pendingBlocks.front();
            _pendingBlocks.pop();
        }
        if (const auto results = doScan(block); !results.isEmpty())
        {
            sendResults(results);
        }
        else
        {
            const std::lock_guard<std::mutex> lock(_mutex);
            _pendingBlocks.emplace(block);
            triggerAsyncUpdate();
        }
    }

    /**
     * @brief Performs the actual plugin scanning operation.
     * 
     * Deserializes the scan request from the memory block, identifies the target
     * plugin format, and scans for all plugin types at the specified location.
     * Scanning only occurs if called from the message thread or if the format
     * doesn't require an unblocked message thread.
     * 
     * @param block The memory block containing format name and plugin identifier.
     * @return An array of plugin descriptions found during the scan, or empty if
     *         scanning cannot be performed at this time.
     */
    juce::OwnedArray<juce::PluginDescription> PluginScannerSubprocess::doScan(const juce::MemoryBlock& block)
    {
        juce::MemoryInputStream stream{ block, false };
        const auto formatName = stream.readString();
        const auto identifier = stream.readString();

        juce::PluginDescription pd;
        pd.fileOrIdentifier = identifier;
        pd.uniqueId = pd.deprecatedUid = 0;

        const auto matchingFormat = [&]() -> juce::AudioPluginFormat*
            {
                for (auto* format : _formatManager.getFormats())
                {
                    if (format->getName() == formatName)
                        return format;
                }
                return nullptr;
            }();

        juce::OwnedArray<juce::PluginDescription> results;

        if (matchingFormat != nullptr
            && (juce::MessageManager::getInstance()->isThisTheMessageThread()
                || !matchingFormat->requiresUnblockedMessageThreadDuringCreation(pd)))
        {
            matchingFormat->findAllTypesForFile(results, identifier);
        }

        return results;
    }

    /**
     * @brief Sends scan results back to the coordinator process.
     * 
     * Serializes the plugin descriptions to XML format and transmits them
     * to the coordinator as a message.
     * 
     * @param results The array of plugin descriptions to send.
     */
    void PluginScannerSubprocess::sendResults(const juce::OwnedArray<juce::PluginDescription>& results)
    {
        juce::XmlElement xml("LIST");
        for (const auto& desc : results)
            xml.addChildElement(desc->createXml().release());
        const auto str = xml.toString();
        sendMessageToCoordinator({ str.toRawUTF8(), str.getNumBytesAsUTF8() });
    }

}