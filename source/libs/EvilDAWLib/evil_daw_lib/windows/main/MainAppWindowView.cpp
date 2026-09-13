#include <functional>
#include <memory>
#include <typeindex>
#include "MainAppWindowView.h"
#include "MainAppDataModel.h"
#include <mvc/commands/NullCommand.h>
#include <mvc/commands/DataModelCommand.h>
#include <mvc/views/BaseWindowView.h>

namespace evil::daw
{
    MainAppWindow::MainAppWindow(juce::JUCEApplication& application,
                                 const juce::String& name,
                                 juce::Colour backgroundColour,
                                 int requiredButtons) : 
        BaseWindowView(application, name, backgroundColour, requiredButtons),
        application_(application)
    {
        auto content = std::make_unique<juce::Component>();
        auto* contentPtr = content.get();
        setContentOwned(content.release(), true);

        statusLabel_.setText("Ready", juce::dontSendNotification);
        statusLabel_.setJustificationType(juce::Justification::centredLeft);
        contentPtr->addAndMakeVisible(statusLabel_);

        mainButton_.setButtonText("Action");
        mainButton_.onClick = [this]
        {
            handleMainButtonClicked();
        };
        contentPtr->addAndMakeVisible(mainButton_);

        testDatModelCommandButton_.setButtonText("Test DatModelCommand");
        testDatModelCommandButton_.onClick = [this]
        {
            handleTestDatModelCommandButtonClicked();
        };
        contentPtr->addAndMakeVisible(testDatModelCommandButton_);

        setResizable(true, true);
        setResizeLimits(300, 250, 10000, 10000);
        setSize(1000, 700);
        centreWithSize(getWidth(), getHeight());
        setVisible(true);

        triggerAsyncUpdate();
    }

    MainAppWindow::~MainAppWindow()
    {
        // Important: Cancel any pending async updates before destruction
        cancelPendingUpdate();
    }

    int MainAppWindow::getClickCounter() const
    {
        return clickCounter_;
    }

    void MainAppWindow::setClickCounter(int newValue)
    {
        clickCounter_ = newValue;
    }

    void MainAppWindow::closeButtonPressed()
    {
        application_.systemRequestedQuit();
    }

    void MainAppWindow::handleAsyncUpdate()
    {
        // Put the code that should run asynchronously here
        // This will be called on the message thread
    }

    void MainAppWindow::resized()
    {
        BaseWindowView::resized();

        if (auto* content = getContentComponent())
        {
            auto area = content->getLocalBounds().reduced(12);

            statusLabel_.setBounds(area.removeFromTop(24));
            area.removeFromTop(8);

            mainButton_.setBounds(area.removeFromTop(32).removeFromLeft(180));
            area.removeFromTop(8);

            testDatModelCommandButton_.setBounds(area.removeFromTop(32).removeFromLeft(180));
        }
    }

    void MainAppWindow::handleMainButtonClicked()
    {
        auto command = createCommand(std::type_index(typeid(mvc::NullCommand)));
        if (command && command->isEnabled())
        {
            (void)command->execute();
        }
    }

    void MainAppWindow::handleTestDatModelCommandButtonClicked()
    {
        auto command = createCommand(std::type_index(typeid(mvc::DataModelCommand<std::shared_ptr<MainAppDataModel>>)));
        if (command && command->isEnabled())
        {
            (void)command->execute();
        }
    }

    void MainAppWindow::someMethod()
    {
        // Check if we're on the message thread
        if (juce::MessageManager::getInstance()->isThisTheMessageThread())
            handleAsyncUpdate();  // Call directly if already on message thread
        else
            triggerAsyncUpdate();  // Schedule for message thread otherwise
    }
}
