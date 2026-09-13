#include "edaw_plugins_list_component.h"
#include "application/EvilDAWApplication.h"

namespace evil
{
    /// Name of the global settings key that controls plugin scan / validation mode.
    constexpr const char* scanModeKey = "pluginScanMode";

    EvilDAWPluginsListComponent::EvilDAWPluginsListComponent(juce::AudioPluginFormatManager& manager,
                                                             juce::KnownPluginList& listToRepresent,
                                                             const juce::File& pedal,
                                                             juce::PropertiesFile* props,
                                                             bool async)
        : juce::PluginListComponent(manager, listToRepresent, pedal, props, async)
    {
        addAndMakeVisible(_validationModeLabel);
        addAndMakeVisible(_validationModeBox);

        _validationModeLabel.attachToComponent(&_validationModeBox, true);
        _validationModeLabel.setJustificationType(juce::Justification::right);
        _validationModeLabel.setSize(100, 30);

        auto unusedId = 1;

        for (const auto mode : { "In-process", "Out-of-process" })
            _validationModeBox.addItem(mode, unusedId++);

        _validationModeBox.setSelectedItemIndex(getAppProperties().getUserSettings()->getIntValue(scanModeKey));

        _validationModeBox.onChange = [this]
            {
                auto* settings = getAppProperties().getUserSettings();
                if (settings != nullptr) // runtime null check per project guidelines
                    settings->setValue(scanModeKey, _validationModeBox.getSelectedItemIndex());
            };

        handleResize();
    }

    /**
     * @brief Destructor for EvilDAWPluginsListComponent.
     *
     * The owned label and combo box are automatically destroyed. Any callbacks
     * attached to `_validationModeBox` are released as part of normal JUCE
     * component teardown.
     */
    EvilDAWPluginsListComponent::~EvilDAWPluginsListComponent()
    {
    }

    void EvilDAWPluginsListComponent::resized()
    {
        handleResize();
    }

    void EvilDAWPluginsListComponent::handleResize()
    {
        PluginListComponent::resized();

        const auto& buttonBounds = getOptionsButton().getBounds();
        _validationModeBox.setBounds(buttonBounds.withWidth(130).withRightX(getWidth() - buttonBounds.getX()));
    }

    void EvilDAWPluginsListComponent::paint(juce::Graphics& graphics)
    {
        juce::PluginListComponent::paint(graphics);
    }

}
