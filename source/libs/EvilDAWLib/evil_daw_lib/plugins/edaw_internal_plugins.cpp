#include "edaw_internal_plugins.h"
#include <evil_plugins_lib.h>

namespace evil
{
    /**************
    static std::unique_ptr<juce::InputStream> createAssetInputStream(const char* resourcePath)
    {
        for (int i = 0; i < juce::BinaryData::namedResourceListSize; ++i)
        {
            if (String(BinaryData::originalFilenames[i]) == String(resourcePath))
            {
                int dataSizeInBytes;
                auto* resource = BinaryData::getNamedResource(BinaryData::namedResourceList[i], dataSizeInBytes);
                return std::make_unique<MemoryInputStream>(resource, dataSizeInBytes, false);
            }
        }

        return {};
    }
    ***************/

    //==============================================================================
    //==============================================================================

    /**
     * @brief Wraps an `AudioProcessor` and exposes it as an `AudioPluginInstance`.
     *
     * This adapter allows internal processors to be registered and handled through
     * JUCE's plugin format abstraction used by the host.
     */
    class InternalPlugin final : public juce::AudioPluginInstance
    {
    public:
        /**
         * @brief Constructs an internal plugin wrapper.
         * @param innerIn Owned processor instance to wrap.
         */
        explicit InternalPlugin(std::unique_ptr<AudioProcessor> innerIn)
            : inner(std::move(innerIn))
        {
            jassert(inner != nullptr);

            for (auto isInput : { true, false })
                matchChannels(isInput);

            setBusesLayout(inner->getBusesLayout());
        }

        //==============================================================================

        const juce::String getName() const override { return inner->getName(); }
        juce::StringArray getAlternateDisplayNames() const override { return inner->getAlternateDisplayNames(); }
        double getTailLengthSeconds() const override { return inner->getTailLengthSeconds(); }
        bool acceptsMidi() const override { return inner->acceptsMidi(); }
        bool producesMidi() const override { return inner->producesMidi(); }
        juce::AudioProcessorEditor* createEditor() override { return inner->createEditor(); }
        bool hasEditor() const override { return inner->hasEditor(); }
        int getNumPrograms() override { return inner->getNumPrograms(); }
        int getCurrentProgram() override { return inner->getCurrentProgram(); }
        void setCurrentProgram(int i) override { inner->setCurrentProgram(i); }
        const juce::String getProgramName(int i) override { return inner->getProgramName(i); }
        void changeProgramName(int i, const juce::String& n) override { inner->changeProgramName(i, n); }
        void getStateInformation(juce::MemoryBlock& b) override { inner->getStateInformation(b); }
        void setStateInformation(const void* d, int s) override { inner->setStateInformation(d, s); }
        void getCurrentProgramStateInformation(juce::MemoryBlock& b) override { inner->getCurrentProgramStateInformation(b); }
        void setCurrentProgramStateInformation(const void* d, int s) override { inner->setCurrentProgramStateInformation(d, s); }

        /**
         * @brief Prepares the wrapped processor for playback.
         * @param sr Initial sample rate.
         * @param bs Initial block size.
         *
         * Synchronizes precision and rate/buffer metadata before forwarding
         * to the wrapped processor.
         */
        void prepareToPlay(double sr, int bs) override
        {
            inner->setProcessingPrecision(getProcessingPrecision());
            inner->setRateAndBufferSizeDetails(sr, bs);
            inner->prepareToPlay(sr, bs);
        }

        void releaseResources() override { inner->releaseResources(); }
        void memoryWarningReceived() override { inner->memoryWarningReceived(); }
        void processBlock(juce::AudioBuffer<float>& a, juce::MidiBuffer& m) override { inner->processBlock(a, m); }
        void processBlock(juce::AudioBuffer<double>& a, juce::MidiBuffer& m) override { inner->processBlock(a, m); }
        void processBlockBypassed(juce::AudioBuffer<float>& a, juce::MidiBuffer& m) override { inner->processBlockBypassed(a, m); }
        void processBlockBypassed(juce::AudioBuffer<double>& a, juce::MidiBuffer& m) override { inner->processBlockBypassed(a, m); }
        bool supportsDoublePrecisionProcessing() const override { return inner->supportsDoublePrecisionProcessing(); }
        bool supportsMPE() const override { return inner->supportsMPE(); }
        bool isMidiEffect() const override { return inner->isMidiEffect(); }
        void reset() override { inner->reset(); }
        void setNonRealtime(bool b) noexcept override { inner->setNonRealtime(b); }
        void refreshParameterList() override { inner->refreshParameterList(); }
        void numChannelsChanged() override { inner->numChannelsChanged(); }
        void numBusesChanged() override { inner->numBusesChanged(); }
        void processorLayoutsChanged() override { inner->processorLayoutsChanged(); }
        void setPlayHead(juce::AudioPlayHead* p) override { inner->setPlayHead(p); }
        void updateTrackProperties(const TrackProperties& p) override { inner->updateTrackProperties(p); }
        bool isBusesLayoutSupported(const BusesLayout& layout) const override { return inner->checkBusesLayoutSupported(layout); }
        bool applyBusLayouts(const BusesLayout& layouts) override { return inner->setBusesLayout(layouts) && AudioPluginInstance::applyBusLayouts(layouts); }

        bool canAddBus(bool) const override { return true; }
        bool canRemoveBus(bool) const override { return true; }

        //==============================================================================
        /**
         * @brief Populates a plugin description for host discovery and display.
         * @param description Output description object.
         */
        void fillInPluginDescription(juce::PluginDescription& description) const override
        {
            description = getPluginDescription(*inner);
        }

    private:
        /**
         * @brief Builds a `PluginDescription` from a processor instance.
         * @param proc Source processor.
         * @return `juce::PluginDescription` derived from processor capabilities.
         */
        static juce::PluginDescription getPluginDescription(const juce::AudioProcessor& proc)
        {
            const auto ins = proc.getTotalNumInputChannels();
            const auto outs = proc.getTotalNumOutputChannels();
            const auto identifier = proc.getName();
            const auto registerAsGenerator = ins == 0;
            const auto acceptsMidi = proc.acceptsMidi();

            juce::PluginDescription descr;

            descr.name = identifier;
            descr.descriptiveName = identifier;
            descr.pluginFormatName = InternalPluginFormat::getIdentifier();
            descr.category = (registerAsGenerator ? (acceptsMidi ? "Synth" : "Generator") : "Effect");
            descr.manufacturerName = "JUCE";
            descr.version = "ProjectInfo::versionString";
            descr.fileOrIdentifier = identifier;
            descr.isInstrument = (acceptsMidi && registerAsGenerator);
            descr.numInputChannels = ins;
            descr.numOutputChannels = outs;

            descr.uniqueId = descr.deprecatedUid = identifier.hashCode();

            return descr;
        }

        /**
         * @brief Aligns this wrapper's bus count with the wrapped processor.
         * @param isInput `true` for input buses, `false` for output buses.
         */
        void matchChannels(bool isInput)
        {
            const auto inBuses = inner->getBusCount(isInput);

            while (getBusCount(isInput) < inBuses)
                addBus(isInput);

            while (inBuses < getBusCount(isInput))
                removeBus(isInput);
        }

        std::unique_ptr<AudioProcessor> inner;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InternalPlugin)
    };

    //==============================================================================

#pragma region InternalPluginFactory class implementation.

    //==============================================================================
    // InternalPluginFactory implementation
    //==============================================================================

    InternalPluginFormat::InternalPluginFactory::InternalPluginFactory(
                                    const std::initializer_list<Constructor>& constructorsIn)
        :   constructors(constructorsIn),
            descriptions([&]
            {
                std::vector<juce::PluginDescription> result;

                for (const auto& constructor : constructors)
                    result.push_back(constructor()->getPluginDescription());

                return result;
            }())
    {
    }

    // --------------------------------------------------------------------------------------------

    /**
     * @brief Gets the cached plugin descriptions for all internal plugin types.
     * @return Const reference to description list.
     */
    const std::vector<juce::PluginDescription>& InternalPluginFormat::InternalPluginFactory::getDescriptions() const
    {
        return descriptions;
    }

    // --------------------------------------------------------------------------------------------

    std::unique_ptr<juce::AudioPluginInstance>
        InternalPluginFormat::InternalPluginFactory::createInstance(const juce::String& name) const
    {
        const auto begin = descriptions.begin();
        const auto it = std::find_if(begin,
            descriptions.end(),
            [&](const juce::PluginDescription& desc) { return name.equalsIgnoreCase(desc.name); });

        if (it == descriptions.end())
            return nullptr;

        const auto index = (size_t)std::distance(begin, it);
        return constructors[index]();
    }

#pragma endregion

    //==============================================================================
    // InternalPluginFormat implementation
    //==============================================================================

    /**
     * @brief Initializes the internal plugin format and registers built-in internal plugins.
     */
    InternalPluginFormat::InternalPluginFormat() :
        factory
        {
            [] { return std::make_unique<InternalPlugin>(std::make_unique<juce::GainProcessor>()); }
        }
    {
    }

    //--------------------------------------------------------------------------------------------

    std::unique_ptr<juce::AudioPluginInstance> InternalPluginFormat::createInstance(const juce::String& name)
    {
        return factory.createInstance(name);
    }

    //--------------------------------------------------------------------------------------------

    void InternalPluginFormat::createPluginInstance(const juce::PluginDescription& description,
                                                    double initialSampleRate,
                                                    int initialBufferSize,
                                                    PluginCreationCallback callback)
    {
        auto instance = createInstance(description.name);

        if (instance != nullptr)
        {
            instance->setPlayConfigDetails(0, 0, initialSampleRate, initialBufferSize);
            callback(std::move(instance), {});
        }
        else
        {
            callback(nullptr, "Failed to create plugin instance");
        }
    }

    //--------------------------------------------------------------------------------------------

    /**
     * @brief Indicates whether plugin creation requires an unblocked message thread.
     * @return Always `false` for internal plugins.
     */
    bool InternalPluginFormat::requiresUnblockedMessageThreadDuringCreation(
        const juce::PluginDescription&) const
    {
        return false;
    }

    //--------------------------------------------------------------------------------------------

    /**
     * @brief Gets the unique format identifier for internal plugins.
     * @return Identifier string: `"Internal"`.
     */
    juce::String InternalPluginFormat::getIdentifier()
    {
        return "Internal";
    }

    juce::String InternalPluginFormat::getName() const
    {
        return getIdentifier();
    }

    bool InternalPluginFormat::fileMightContainThisPluginType(const juce::String&)
    {
        juce::ignoreUnused();
        return true;
    }

    juce::FileSearchPath InternalPluginFormat::getDefaultLocationsToSearch()
    {
        return {};
    }

    /**
     * @brief Indicates if this format supports scanning the file system.
     * @return Always `false`.
     */
    bool InternalPluginFormat::canScanForPlugins() const
    {
        return false;
    }

    /**
     * @brief Indicates if scanning is trivial for this format.
     * @return Always `true`.
     */
    bool InternalPluginFormat::isTrivialToScan() const
    {
        return true;
    }

    void InternalPluginFormat::findAllTypesForFile(juce::OwnedArray<juce::PluginDescription>&, const juce::String&)
    {
    }

    /**
     * @brief Checks whether a plugin described by metadata still exists.
     * @return Always `true` for internal plugins.
     */
    bool InternalPluginFormat::doesPluginStillExist(const juce::PluginDescription&)
    {
        return true;
    }

    /**
     * @brief Gets the plugin name from an identifier.
     * @param fileOrIdentifier Identifier value.
     * @return Identifier unchanged.
     */
    juce::String InternalPluginFormat::getNameOfPluginFromIdentifier(const juce::String& fileOrIdentifier)
    {
        return fileOrIdentifier;
    }

    /**
     * @brief Indicates whether plugin metadata should be rescanned.
     * @return Always `false` for internal plugins.
     */
    bool InternalPluginFormat::pluginNeedsRescanning(const juce::PluginDescription&)
    {
        return false;
    }

    juce::StringArray InternalPluginFormat::searchPathsForPlugins(const juce::FileSearchPath&, bool, bool)
    {
        return {};
    }

    /**
     * @brief Gets all available internal plugin descriptions.
     * @return Const reference to the registered type list.
     */
    const std::vector<juce::PluginDescription>& InternalPluginFormat::getAllTypes() const
    {
        return factory.getDescriptions();
    }


}