#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "evil_daw_application_settings.h"
#include "evil_daw_application_menu_model.h"


namespace evil
{

    class EvilDAWApplicationMenuModel;

    /**
     * @class EvilDAWApplication
     * @brief Main application class for the EvilDAW digital audio workstation.
     *
     * This class serves as the entry point and lifecycle manager for the EvilDAW application.
     * It inherits from juce::JUCEApplication and handles application initialization, shutdown,
     * and instance management. The class is marked as final to prevent further inheritance.
     */
    class EvilDAWApplication final : public juce::JUCEApplication,
                                     private juce::AsyncUpdater
    {
    public:

        // Friend function declaration
        friend void EvilDAWApplicationMenuModel::GFG_Function(EvilDAWApplication& app);

        static EvilDAWApplication& getApp();
        static juce::ApplicationCommandManager& getCommandManager();

    public:
        /**
         * @brief Default constructor for the EvilDAW application.
         */
        EvilDAWApplication() {};

        /**
         * @brief Returns the name of the application.
         * @return A juce::String containing the application name.
         */
        const juce::String getApplicationName() override;

        /**
         * @brief Returns the current version of the application.
         * @return A juce::String containing the application version.
         */
        const juce::String getApplicationVersion() override;

        /**
         * @brief Retrieves the menu bar model for the application.
         * @return A pointer to the juce::MenuBarModel instance used by the application.
         */
        juce::MenuBarModel* getMenuBarModel();

        /**
         * @brief Determines if multiple instances of the application can run simultaneously.
         * @return true if multiple instances are allowed, false otherwise.
         */
        bool moreThanOneInstanceAllowed() override;

        /**
         * @brief Initializes the application on startup.
         * @param commandLine The command line arguments passed to the application.
         */
        void initialise(const juce::String& commandLine) override;

        /**
         * @brief Performs cleanup operations when the application is shutting down.
         */
        void shutdown() override;

        /**
         * @brief Handles system-level requests to quit the application.
         */
        void systemRequestedQuit() override;

        /**
         * @brief Handles the event when another instance of the application is started.
         * @param commandLine The command line arguments passed to the new instance.
         */
        void anotherInstanceStarted(const juce::String& commandLine) override;

        EvilDawApplicationSettings& getApplicationSettings();

        juce::PropertiesFile::Options getPropertyFileOptionsFor(const juce::String& filename, bool isProjectSettings);

    private:

        //void globalFocusChanged(juce::Component* focusedComponent) override;
        juce::PopupMenu& getPopupMenu() 
        {
            return * new juce::PopupMenu();
        };

        void handleAsyncUpdate() override;


        void initCommandManager();
        bool initialiseLogger(const char* filePrefix);
        void shutdownLogger();

        std::unique_ptr<juce::FileLogger> _logger;
        std::unique_ptr<EvilDawApplicationSettings> _applicationSettings;
        std::unique_ptr<juce::ApplicationCommandManager> _commandManager;
        std::unique_ptr<juce::MenuBarModel> _menuModel;

        /**
         * @brief Shared pointer to the main application window.
         */
        std::shared_ptr<juce::DocumentWindow> _mainWindow;

    private:
        // ------------------------------------------------------------------------------
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EvilDAWApplication)
        JUCE_DECLARE_WEAK_REFERENCEABLE(EvilDAWApplication)
        // ------------------------------------------------------------------------------
    };
}
