#include "evil_daw_application_settings.h"
#include "evil_daw_application.h"

namespace evil
{

    static juce::PropertiesFile* createPropsFile(const juce::String& filename, bool isProjectSettings);

        // ------------------------------------------------------------------------------

    EvilDawApplicationSettings::EvilDawApplicationSettings() :
        _appearanceSettings(std::make_unique<EvilDawAppearanceSettings>(true))
        //projectDefaults("PROJECT_DEFAULT_SETTINGS"),
        //fallbackPaths("FALLBACK_PATHS")

    {
        //updateOldProjectSettingsFiles();

        reload();
        //changed(true);
        flush();

    }

    EvilDawApplicationSettings::~EvilDawApplicationSettings()
    {
    }

    // ------------------------------------------------------------------------------

    void EvilDawApplicationSettings::flush()
    {
        //updateGlobalPreferences();
        //saveSwatchColours();
        for (auto i = _propertyFiles.size(); --i >= 0;)
            _propertyFiles.getUnchecked(i)->saveIfNeeded();
    }

    void EvilDawApplicationSettings::reload()
    {
        _propertyFiles.clear();
        _propertyFiles.add(createPropsFile("EvilDAW", false));

        //if (auto projectDefaultsXml = _propertyFiles.getFirst()->getXmlValue("PROJECT_DEFAULT_SETTINGS"))
        //    projectDefaults = ValueTree::fromXml(*projectDefaultsXml);

        //if (auto fallbackPathsXml = propertyFiles.getFirst()->getXmlValue("FALLBACK_PATHS"))
        //    fallbackPaths = ValueTree::fromXml(*fallbackPathsXml);

        /***
        // recent files...
        recentFiles.restoreFromString(getGlobalProperties().getValue("recentFiles"));
        recentFiles.removeNonExistentFiles();
        lastWizardFolder = getGlobalProperties().getValue("lastWizardFolder");
        loadSwatchColours();
        ***/
    }

    // ------------------------------------------------------------------------------

    juce::PropertiesFile& EvilDawApplicationSettings::getGlobalProperties()
    {
        return *_propertyFiles.getUnchecked(0);
    }

    juce::PropertiesFile& EvilDawApplicationSettings::getProjectProperties(const juce::String& projectUID)
    {
        const auto filename = juce::String("Projucer_Project_" + projectUID);
        for (auto i = _propertyFiles.size(); --i >= 0;)
        {
            auto* const props = _propertyFiles.getUnchecked(i);
            if (props->getFile().getFileNameWithoutExtension() == filename)
                return *props;
        }
        auto* p = createPropsFile(filename, true);
        _propertyFiles.add(p);
        return *p;
    }

    // ------------------------------------------------------------------------------

    EvilDawApplicationSettings& getAppSettings()
    {
        return EvilDAWApplication::getApp().getApplicationSettings();
    }

    juce::PropertiesFile& getGlobalProperties()
    { 
        return getAppSettings().getGlobalProperties();
    }

    // ------------------------------------------------------------------------------

    static juce::PropertiesFile* createPropsFile(const juce::String& filename, bool isProjectSettings)
    {
        return new juce::PropertiesFile(
            EvilDAWApplication::getApp().getPropertyFileOptionsFor(filename, isProjectSettings));
    }

    // ------------------------------------------------------------------------------

}