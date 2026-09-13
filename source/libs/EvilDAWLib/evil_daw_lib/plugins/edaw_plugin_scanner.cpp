#include <juce_data_structures.h>
#include "application/EvilDAWApplication.h"
#include "edaw_plugin_scanner.h"

namespace evil
{
    /// Name of the global settings key that controls plugin scan mode.
    constexpr const char* scanModeKey = "pluginScanMode";

    class Superprocess final : private juce::ChildProcessCoordinator
    {
    public:
        /**
         * @brief Constructs a `Superprocess` and launches the worker process.
         *
         * Uses the current executable file to start a child process with the
         * configured process UID and default timeout settings.
         */
        Superprocess()
        {
            launchWorkerProcess(juce::File::getSpecialLocation(juce::File::currentExecutableFile),
                                processUID,
                                0,
                                0);
        }

        /**
         * @enum State
         * @brief Possible states of a worker process response.
         */
        enum class State
        {
            timeout,        //!< Response timeout occurred while waiting for a message.
            gotResult,      //!< A valid result was successfully received from the worker.
            connectionLost, //!< Connection to the worker process was lost.
        };

        /**
         * @struct Response
         * @brief Aggregates the state and payload of a worker process response.
         */
        struct Response
        {
            State state;                           //!< The current outcome of the receive operation.
            std::unique_ptr<juce::XmlElement> xml; //!< Parsed XML payload from the worker, or `nullptr` if none.
        };

        /**
         * @brief Retrieves the next response from the worker process with a timeout.
         *
         * Blocks for up to 50 milliseconds while waiting for either:
         * - A complete result from the worker process, or
         * - Notification that the connection to the worker has been lost.
         *
         * Synchronization is handled via a mutex and condition variable to ensure
         * thread-safe access to the internal XML payload and state flags.
         *
         * @return A `Response` value describing the outcome:
         *         - `State::timeout` with `xml == nullptr` if no update arrived in time.
         *         - `State::gotResult` and a non-null `xml` pointer when a result is available.
         *         - `State::connectionLost` if the worker process has disconnected.
         */
        Response getResponse()
        {
            std::unique_lock<std::mutex> lock{ mutex };

            if (!condvar.wait_for(lock, std::chrono::milliseconds{ 50 },
                                  [&] { return gotResult || connectionLost; }))
                return { State::timeout, nullptr };

            const auto state = connectionLost ? State::connectionLost : State::gotResult;
            connectionLost = false;
            gotResult = false;

            return { state, std::move(pluginDescription) };
        }

        /// Expose `sendMessageToWorker` from the `juce::ChildProcessCoordinator` base.
        using ChildProcessCoordinator::sendMessageToWorker;

    private:
        /**
         * @brief Handles incoming messages from the worker process.
         *
         * Called by the `juce::ChildProcessCoordinator` base when a new message is
         * received from the worker. Interprets the payload as UTF-8 XML, parses it
         * into a `juce::XmlElement`, and notifies any waiting threads.
         *
         * @param mb Memory block containing the XML-formatted response from the worker.
         */
        void handleMessageFromWorker(const juce::MemoryBlock& mb) override
        {
            const std::lock_guard<std::mutex> lock{ mutex };
            pluginDescription = juce::parseXML(mb.toString());
            gotResult = true;
            condvar.notify_one();
        }

        /**
         * @brief Handles loss of connection to the worker process.
         *
         * Invoked when the connection to the worker process is unexpectedly lost.
         * Sets the `connectionLost` flag and wakes any threads waiting in
         * `getResponse()`, allowing callers to react appropriately.
         */
        void handleConnectionLost() override
        {
            const std::lock_guard<std::mutex> lock{ mutex };
            connectionLost = true;
            condvar.notify_one();
        }

        /// Mutex protecting shared response state.
        std::mutex mutex;

        /// Condition variable used to signal changes in response state.
        std::condition_variable condvar;

        /// Most recent XML response received from the worker, or `nullptr` if none.
        std::unique_ptr<juce::XmlElement> pluginDescription;

        /// Indicates whether the connection to the worker process was lost.
        bool connectionLost{ false };

        /// Indicates whether a new result has been received from the worker.
        bool gotResult{ false };

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Superprocess)
    };

    // ===============================================================================

    CustomPluginScanner::CustomPluginScanner()
    {
        auto* file = getAppProperties().getUserSettings();
        if (file != nullptr)
            file->addChangeListener(this);

        handleChange();
    }

    CustomPluginScanner::~CustomPluginScanner()
    {
        auto* file = getAppProperties().getUserSettings();
        if (file != nullptr)
            file->removeChangeListener(this);
    }

    bool CustomPluginScanner::findPluginTypesFor(juce::AudioPluginFormat& format,
                                                 juce::OwnedArray<juce::PluginDescription>& result,
                                                 const juce::String& fileOrIdentifier)
    {
        if (_scanInProcess)
        {
            _superprocess = nullptr;
            format.findAllTypesForFile(result, fileOrIdentifier);
            return true;
        }

        if (addPluginDescriptions(format.getName(), fileOrIdentifier, result))
        {
            return true;
        }

        _superprocess = nullptr;
        return false;
    }

    void CustomPluginScanner::scanFinished()
    {
        _superprocess = nullptr;
    }

    bool CustomPluginScanner::addPluginDescriptions(const juce::String& formatName,
                                                    const juce::String& fileOrIdentifier,
                                                    juce::OwnedArray<juce::PluginDescription>& result)
    {
        if (_superprocess == nullptr)
            _superprocess = std::make_unique<Superprocess>();

        juce::MemoryBlock block;
        juce::MemoryOutputStream stream{ block, true };
        stream.writeString(formatName);
        stream.writeString(fileOrIdentifier);

        if (!_superprocess->sendMessageToWorker(block))
        {
            return false;
        }

        for (;;)
        {
            if (shouldExit())
                return true;

            const auto response = _superprocess->getResponse();

            if (response.state == Superprocess::State::timeout)
                continue;

            if (response.xml != nullptr)
            {
                for (const auto* item : response.xml->getChildIterator())
                {
                    auto desc = std::make_unique<juce::PluginDescription>();
                    if (desc->loadFromXml(*item))
                    {
                        result.add(std::move(desc));
                    }
                }
            }

            return (response.state == Superprocess::State::gotResult);
        }
    }

    void CustomPluginScanner::handleChange()
    {
        auto* file = getAppProperties().getUserSettings();
        if (file == nullptr)
        {
            _scanInProcess = true;
            return;
        }

        _scanInProcess = (file->getIntValue(scanModeKey) == 0);
    }

    void CustomPluginScanner::changeListenerCallback(juce::ChangeBroadcaster*)
    {
        handleChange();
    }

} // namespace evil
