#pragma once

#include <juce_audio_processors.h>

namespace evil
{

    /**
     * @brief JUCE audio plugin format for in-process/internal plugins.
     *
     * This format does not scan disk locations. Instead, plugin instances are created
     * from constructor callbacks managed by an internal factory.
     */
    class InternalPluginFormat final : public juce::AudioPluginFormat
    {
    public:
        /** @brief Constructs the internal plugin format. */
        InternalPluginFormat();

        /**
         * @brief Returns all registered internal plugin descriptions.
         * @return Const reference to all available plugin types.
         */
        const std::vector<juce::PluginDescription>& getAllTypes() const;

        //==============================================================================

        static juce::String getIdentifier();
        juce::String getName() const override;
        bool fileMightContainThisPluginType(const juce::String&) override;
        juce::FileSearchPath getDefaultLocationsToSearch() override;
        bool canScanForPlugins() const override;
        bool isTrivialToScan() const override;
        void findAllTypesForFile(juce::OwnedArray<juce::PluginDescription>&, const juce::String&) override;
        bool doesPluginStillExist(const juce::PluginDescription& description) override;
        juce::String getNameOfPluginFromIdentifier(const juce::String& fileOrIdentifier) override;
        bool pluginNeedsRescanning(const juce::PluginDescription& description) override;
        juce::StringArray searchPathsForPlugins(const juce::FileSearchPath&, bool, bool) override;

    private:

        //==============================================================================
        /**
         * @brief Factory used to create internal plugin instances and cache descriptions.
         */
        class InternalPluginFactory
        {
        public:
            /** @brief Callback signature used to construct plugin instances. */
            using Constructor = std::function<std::unique_ptr<juce::AudioPluginInstance>()>;

            /**
             * @brief Constructs the factory from constructor callbacks.
             * @param constructorsIn List of plugin constructors.
             */
            explicit InternalPluginFactory(const std::initializer_list<Constructor>& constructorsIn);

            /**
             * @brief Gets cached plugin descriptions.
             * @return Const reference to descriptions.
             */
            const std::vector<juce::PluginDescription>& getDescriptions() const;

            /**
             * @brief Creates an instance by plugin name.
             * @param name Plugin name.
             * @return Plugin instance or `nullptr` if not found.
             */
            std::unique_ptr<juce::AudioPluginInstance> createInstance(const juce::String& name) const;

        private:
            /** @brief Constructor callbacks indexed by plugin description. */
            const std::vector<Constructor> constructors;

            /** @brief Cached plugin descriptions generated from constructors. */
            std::vector<juce::PluginDescription> descriptions;
        };
        //==============================================================================

        //==============================================================================
        void createPluginInstance(const juce::PluginDescription&,
            double initialSampleRate, int initialBufferSize,
            PluginCreationCallback) override;

        /**
         * @brief Creates a plugin instance by name.
         * @param name Plugin name.
         * @return Plugin instance or `nullptr` if unavailable.
         */
        std::unique_ptr<juce::AudioPluginInstance> createInstance(const juce::String& name);

        /**
         * @brief Indicates whether creation requires an unblocked message thread.
         * @return Always `false`.
         */
        bool requiresUnblockedMessageThreadDuringCreation(const juce::PluginDescription&) const override;

        /** @brief Internal factory owning constructors and description cache. */
        InternalPluginFactory factory;
    };

}