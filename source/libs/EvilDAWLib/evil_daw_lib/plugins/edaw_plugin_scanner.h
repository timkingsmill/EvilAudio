#pragma once

#include <atomic>
#include <memory>

#include <juce_audio_processors.h>

namespace evil
{
    class Superprocess;

    // -------------------------------------------------------------------------------
    class CustomPluginScanner final : public juce::KnownPluginList::CustomScanner,
                                      private juce::ChangeListener
    {
    public:
        /**
         * @brief Constructs a new `CustomPluginScanner`.
         *
         * Registers the scanner as a change listener on the global user settings and
         * initializes the scan mode based on the `"pluginScanMode"` setting.
         */
        CustomPluginScanner();

        /**
         * @brief Destroys the `CustomPluginScanner` instance.
         *
         * Unregisters from the global user settings change notifications and releases
         * any managed resources associated with plugin scanning.
         */
        ~CustomPluginScanner() override;

    private:
        /**
         * @brief Finds all plugin types for a given file or identifier.
         *
         * Overrides `juce::KnownPluginList::CustomScanner::findPluginTypesFor`.
         * Depending on the current scan mode:
         * - If scanning in process, directly calls `format.findAllTypesForFile()`.
         * - Otherwise, delegates to a worker process via `addPluginDescriptions()`.
         *
         * @param format Reference to the plugin format implementation used for scanning.
         * @param result Destination array that receives discovered plugin descriptions.
         * @param fileOrIdentifier Path or identifier of the plugin being scanned.
         *
         * @return `true` if scanning completed or was intentionally aborted
         *         (including successful or empty results), `false` if the worker
         *         communication failed.
         */
        bool findPluginTypesFor(juce::AudioPluginFormat& format,
                                juce::OwnedArray<juce::PluginDescription>& result,
                                const juce::String& fileOrIdentifier) override;

        /**
         * @brief Notifies the scanner that a scan operation has finished.
         *
         * Overrides `juce::KnownPluginList::CustomScanner::scanFinished` and clears
         * any active `Superprocess` instance to ensure the worker process is released.
         */
        void scanFinished() override;

        /**
         * @brief Sends a scan request to the worker process and collects plugin descriptions.
         *
         * Serializes the format name and plugin file/identifier, sends this message to
         * the worker process managed by `Superprocess`, and then waits for XML-based
         * plugin description responses.
         *
         * @param formatName Name of the plugin format (e.g. `"VST3"`, `"AudioUnit"`).
         * @param fileOrIdentifier Path or unique identifier of the plugin to scan.
         * @param result Destination array that receives parsed `juce::PluginDescription`
         *               instances when valid XML is returned.
         *
         * @return `true` if a result (including an empty set) was successfully obtained
         *         from the worker, `false` if sending the request or communication
         *         with the worker process failed.
         */
        bool addPluginDescriptions(const juce::String& formatName,
                                   const juce::String& fileOrIdentifier,
                                   juce::OwnedArray<juce::PluginDescription>& result);

        /**
         * @brief Updates the scan mode flag based on the global user settings.
         *
         * Reads the integer value under the `"pluginScanMode"` key from the global
         * application properties and sets `_scanInProcess` accordingly:
         * - `0`  -> scan in process.
         * - != 0 -> scan using the worker process.
         */
        void handleChange();

        /**
         * @brief Called when a change is broadcast from the observed `ChangeBroadcaster`.
         *
         * Implements `juce::ChangeListener::changeListenerCallback` and delegates to
         * `handleChange()` to re-evaluate the current scan mode.
         *
         * @param source Pointer to the broadcaster that triggered the callback.
         *               This parameter is unused.
         */
        void changeListenerCallback(juce::ChangeBroadcaster* source) override;

        /// Manages the lifetime and communication with the worker process when
        /// out-of-process scanning is enabled. May be `nullptr` when not in use.
        std::unique_ptr<Superprocess> _superprocess;

        /// Indicates whether scanning should be performed in-process (`true`) or
        /// via the external worker process (`false`). Updated from global settings.
        std::atomic<bool> _scanInProcess{ true };

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomPluginScanner)
    };

} // namespace evil
