#include "XXMainWindowView.h"

#include "application/EvilDAWApplication.h"
#include "application/edaw_app_main_keys.h"
#include <mvc/MVCTypeFactories.h>
#include "application/mvc/datamodels/edaw_app_main_datamodel.h"
#include "application/mvc/controllers/EvilDAWMainController.h"
#include <commands/IncrementCommand.h>
#include <mvc/CommandHandler.h>
#include <mvc/Controller.h>
#include <commands/keys/CommandKeys.h>

namespace evil
{
    
    struct MainWindowView::MenuModel : public juce::MenuBarModel
    {
        enum MenuIds
        {
            menuFile = 1,
            menuEdit,
            menuPlugins,
            menuView,
            menuHelp,
            menuQuit,
            menuPlaceholder
        };

        explicit MenuModel(MainWindowView& ownerIn)
            : owner(ownerIn)
        {
        }

        juce::StringArray getMenuBarNames() override
        {
            return { "File", "Edit", "Plugins", "View", "Help" };
        }

        juce::PopupMenu getMenuForIndex(int topLevelMenuIndex, const juce::String&) override
        {
            juce::PopupMenu menu;

            switch (topLevelMenuIndex)
            {
                case 0: menu.addItem(menuQuit, "Quit"); break;
                case 1:
                    //menu.addCommandItem(&EvilDAWApplication::getCommandManager(), (int)CommandID::Undo);
                    //menu.addCommandItem(&EvilDAWApplication::getCommandManager(), (int)CommandID::Redo);
                    menu.addSeparator();
                    menu.addItem(menuPlaceholder, "Preferences...");
                    break;
                case 2:
                    //menu.addCommandItem(&EvilDAWApplication::getCommandManager(), (int)CommandID::OpenPluginList);
                    //menu.addCommandItem(&EvilDAWApplication::getCommandManager(), (int)CommandID::DeleteAllPlugins);
                    break;
                case 3: menu.addItem(menuPlaceholder, "Toggle Sidebar"); break;
                case 4: menu.addItem(menuPlaceholder, "About"); break;
                default: break;
            }

            return menu;
        }

        void menuItemSelected(int menuItemID, int) override
        {
            if (menuItemID == menuQuit)
                owner.application_.systemRequestedQuit();
        }

        MainWindowView& owner;
    };

    
    MainWindowView::MainWindowView(juce::JUCEApplication& application,
                                   mvc::Controller& controller,
                                   const juce::String& name,
                                   juce::Colour backgroundColour,
                                   int requiredButtons) :
        View(controller),
        menuModel_(std::make_unique<MenuModel>(*this)),
        juce::DocumentWindow(name, backgroundColour, requiredButtons),
        application_(application)
    {
        setUsingNativeTitleBar(false);

        dataModel_ = mvc::makeModel<EvilDAWDataModel>();

        #if JUCE_IOS || JUCE_ANDROID
            setFullScreen(true);
        #else
            setResizable(true, true);
            setResizeLimits(300, 250, 10000, 10000);
        #endif

        auto content = std::make_unique<juce::Component>();
        auto* contentPtr = content.get();
        setContentComponent(content.release(), true);

        menuBar_.setModel(menuModel_.get());
        contentPtr->addAndMakeVisible(&menuBar_);

        mainButton_.setButtonText("Action");
        mainButton_.onClick = [this]
        {
            handleMainButtonClicked();
        };
        contentPtr->addAndMakeVisible(&mainButton_);

        triggerAsyncUpdate();
    }

    // -------------------------------------------------------------------------------

    MainWindowView::~MainWindowView()
    {
        menuBar_.setModel(nullptr);
    }
    // -------------------------------------------------------------------------------
    // -------------------------------------------------------------------------------

    void MainWindowView::handleMainButtonClicked()
    {
        // For demonstration purposes, we create and execute an 
        // IncrementCommand when the button is clicked. 
        auto command = mvc::makeIncrementCommand(counter_, 1);
        // Execute the command through the view's command 
        // execution mechanism.
        executeCommand(*command);
        /***
        auto controller = getController();
        if (controller == nullptr)
        {
            showError("No controller attached to the view.");
            return;
        }
        if (controller->canExecuteCommand(*command))
        {
            auto result = controller->executeCommand(*command);
            if (!result.success)
            {
                showValidationResult(result);
                return;
            }
        }
        else
        {
            showError("Command cannot be executed.");
        }
        ***/
    }

    // -------------------------------------------------------------------------------

    void MainWindowView::handleAsyncUpdate()
    {
    }

    // -------------------------------------------------------------------------------

    void MainWindowView::render(const mvc::ViewModel& viewModel)
    {
        if (!viewModel.messages.empty())
            showError(viewModel.messages);
    }

    void MainWindowView::onInit()
    {
       #if !(JUCE_IOS || JUCE_ANDROID)
        setSize(1000, 700);
        centreWithSize(getWidth(), getHeight());
       #endif

        setVisible(true);
    }

    void MainWindowView::onShow()
    {
        setVisible(true);
    }

    void MainWindowView::onHide()
    {
        setVisible(false);
    }

    void MainWindowView::onDestroy()
    {
        setVisible(false);
    }

    void MainWindowView::showError(const std::vector<std::string>& errors)
    {
        if (errors.empty())
            return;

        juce::StringArray errorLines;

        for (const auto& error : errors)
            errorLines.add(error);

        showError(errorLines.joinIntoString("\n").toStdString());
    }

    void MainWindowView::showError(const std::string& error)
    {
        if (error.empty())
            return;

        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                                               "Error",
                                               juce::String(error));
    }

    void MainWindowView::handleModelEvent(const mvc::ModelEvent& event)
    {
        if (event.type != mvc::ModelEventType::ValidationFailed)
            return;

        std::vector<std::string> errors;
        errors.reserve(event.payload.size());

        for (const auto& [key, value] : event.payload)
        {
            if (!value.empty())
                errors.push_back(value);
            else if (!key.empty())
                errors.push_back(key);
        }

        showError(errors);
    }

    void MainWindowView::closeButtonPressed()
    {
        application_.systemRequestedQuit();
    }

    void MainWindowView::resized()
    {
        juce::DocumentWindow::resized();

        if (auto* content = getContentComponent())
        {
            auto bounds = content->getLocalBounds();
            menuBar_.setBounds(bounds.removeFromTop(24));

            auto buttonBounds = bounds.reduced(12);
            mainButton_.setBounds(buttonBounds.removeFromTop(28).removeFromLeft(120));
            return;
        }

    //        auto bounds = getLocalBounds().reduced(12);
    //        mainButton_.setBounds(bounds.removeFromTop(28).removeFromLeft(120));
  ///      auto bounds = getLocalBounds();
     //   mainButton_.setBounds(bounds.removeFromTop(28).removeFromLeft(120));

    }
}