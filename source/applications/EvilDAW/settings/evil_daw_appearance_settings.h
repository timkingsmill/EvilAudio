#pragma once

#include <juce_data_structures/juce_data_structures.h>

namespace evil
{
    //==============================================================================
    class EvilDawAppearanceSettings final : private juce::ValueTree::Listener
    {
    public:
        EvilDawAppearanceSettings(bool updateAppWhenChanged);

        bool readFromFile(const juce::File& file);
        bool readFromXML(const juce::XmlElement&);
        bool writeToFile(const juce::File& file) const;

        /**
        void updateColourScheme();
        void applyToCodeEditor(CodeEditorComponent& editor) const;

        StringArray getColourNames() const;
        Value getColourValue(const String& colourName);
        bool getColour(const String& name, Colour& resultIfFound) const;

        Font getCodeFont() const;
        Value getCodeFontValue();

        ValueTree settings;

        static File getSchemesFolder();
        StringArray getPresetSchemes();
        void refreshPresetSchemeList();
        void selectPresetScheme(int index);

        static Font getDefaultCodeFont();

        static const char* getSchemeFileSuffix() { return ".scheme"; }
        static const char* getSchemeFileWildCard() { return "*.scheme"; }
        */
    private:
/*
        Array<File> presetSchemeFiles;

        static void writeDefaultSchemeFile(const String& xml, const String& name);

        void valueTreePropertyChanged(ValueTree&, const Identifier&) override { updateColourScheme(); }
        void valueTreeChildAdded(ValueTree&, ValueTree&) override { updateColourScheme(); }
        void valueTreeChildRemoved(ValueTree&, ValueTree&, int) override { updateColourScheme(); }
        void valueTreeChildOrderChanged(ValueTree&, int, int) override { updateColourScheme(); }
        void valueTreeParentChanged(ValueTree&) override { updateColourScheme(); }
        void valueTreeRedirected(ValueTree&) override { updateColourScheme(); }
        */
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EvilDawAppearanceSettings)
    };
}