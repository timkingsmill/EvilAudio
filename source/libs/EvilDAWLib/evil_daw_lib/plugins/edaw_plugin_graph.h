#pragma once

#include <juce_audio_processors.h>

namespace evil
{
    /**
     * @brief Holds a JUCE plugin description together with user preferences.
     *
     * This structure associates a `juce::PluginDescription` with additional
     * metadata describing how the host should treat the plugin, such as
     * whether ARA should be used when available.
     */
    struct PluginDescriptionAndPreference
    {
        /**
         * @brief Indicates whether ARA should be used for the plugin.
         */
        enum class UseARA
        {
            /** @brief Do not use ARA, even if the plugin supports it. */
            no,

            /** @brief Use ARA when the plugin supports it. */
            yes
        };

        /**
         * @brief Constructs an empty plugin description with default preferences.
         *
         * The `useARA` preference is initialized to `UseARA::no`.
         */
        PluginDescriptionAndPreference() = default;

        /**
         * @brief Constructs the object from a plugin description.
         *
         * The ARA usage preference is inferred from the description's
         * `hasARAExtension` flag.
         *
         * @param pd The JUCE plugin description to store.
         */
        explicit PluginDescriptionAndPreference(juce::PluginDescription pd)
            : pluginDescription(std::move(pd)),
              useARA(pluginDescription.hasARAExtension ? PluginDescriptionAndPreference::UseARA::yes
                                                      : PluginDescriptionAndPreference::UseARA::no)
        {
        }

        /**
         * @brief Constructs the object from a plugin description and explicit ARA preference.
         *
         * This allows overriding the default behavior based on `hasARAExtension`.
         *
         * @param pd  The JUCE plugin description to store.
         * @param ara The explicit ARA usage preference.
         */
        PluginDescriptionAndPreference(juce::PluginDescription pd, UseARA ara)
            : pluginDescription(std::move(pd)), useARA(ara)
        {
        }

        /**
         * @brief The JUCE plugin description providing identifying and capability information.
         */
        juce::PluginDescription pluginDescription;

        /**
         * @brief User or host preference indicating whether to use ARA.
         *
         * Defaults to `UseARA::no`.
         */
        UseARA useARA = UseARA::no;
    };

}