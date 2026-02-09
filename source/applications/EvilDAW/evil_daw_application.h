#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace evil
{

    /**
     * @class EvilDAWApplication
     * @brief Main application class for the EvilDAW digital audio workstation.
     *
     * This class serves as the entry point and lifecycle manager for the EvilDAW application.
     * It inherits from juce::JUCEApplication and handles application initialization, shutdown,
     * and instance management. The class is marked as final to prevent further inheritance.
     */
    class EvilDAWApplication final : public juce::JUCEApplication
    {
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

    private:
        /**
         * @brief Shared pointer to the main application window.
         */
        std::shared_ptr<juce::DocumentWindow> _mainWindow;

    };

}