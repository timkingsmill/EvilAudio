#include <memory>

#include <mvc/controllers/BaseController.h>
#include <mvc/datamodels/BaseDataModel.h>
#include <mvc/commands/NullCommand.h>
#include <mvc/commands/BaseCommand.h>
#include <mvc/commands/DataModelCommand.h>
#include <mvc/commands/CommandFactory.h>
#include <mvc/viewmodels/ViewModelFactory.h>
#include <mvc/validations/ValidationResult.h>

#include "MainAppController.h"
#include "MainAppWindowView.h"
#include "MainAppViewModel.h"

namespace evil::daw
{
    MainAppController::MainAppController()
    {

        // ---------------------------------------------------------------------------------

        evil::mvc::CommandFactory::registerType<mvc::NullCommand>(
            [](const evil::mvc::CommandFactory::ArgList&) 
            -> std::unique_ptr<evil::mvc::BaseCommand>
            {
                return std::make_unique<mvc::NullCommand>();
            });

        evil::mvc::CommandFactory::registerType<mvc::DataModelCommand<std::shared_ptr<MainAppDataModel>>>(
            [this](const evil::mvc::CommandFactory::ArgList&)
            -> std::unique_ptr<evil::mvc::BaseCommand>
            {
                auto model = _typedModelWeak.lock();
                if (model == nullptr)
                    return std::make_unique<mvc::NullCommand>();
                return std::make_unique<mvc::DataModelCommand<std::shared_ptr<MainAppDataModel>>>(std::move(model));
            });

        // ---------------------------------------------------------------------------------
        // Register view models for views that don't have custom ones. This allows the view 
        // to be added without a custom view model, and the base view model can handle it 
        // with default behavior.

        /*
        evil::mvc::ViewModelFactory::registerType<mvc::BaseViewModel>(
            [this](const evil::mvc::ViewModelFactory::ArgList&)
            -> std::unique_ptr<evil::mvc::BaseViewModel>
            {
                return std::make_unique<mvc::BaseViewModel>();
            });
            */
        // ---------------------------------------------------------------------------------
        // Register the main app view model, which is used by the main app view. 
        // This is a custom view model that can handle the specific needs of the main app view.

        //factory[std::type_index(typeid(Foo))] = []() { return std::make_unique<Foo>(); }


        /***
        evil::mvc::ViewModelFactory::registerType<evil::daw::MainAppViewModel>(
            [this](const evil::mvc::ViewModelFactory::ArgList&)
            -> std::unique_ptr<evil::daw::MainAppViewModel>
            {
                return std::make_unique<evil::daw::MainAppViewModel>();
            });
            **********/

        evil::mvc::ViewModelFactory::registerViewType<evil::daw::MainAppWindow>(
            [](const evil::mvc::ViewModelFactory::ArgList& args)
            -> std::unique_ptr<evil::mvc::BaseViewModel>
            {
                auto* base = std::any_cast<evil::mvc::BaseView*>(args.at(0));
                auto* window = dynamic_cast<evil::daw::MainAppWindow*>(base);
                return std::make_unique<evil::daw::MainAppViewModel>(window);
            });

        // ---------------------------------------------------------------------------------

    }

    MainAppController::~MainAppController()
    {
        evil::mvc::CommandFactory::unregisterType<mvc::DataModelCommand<std::shared_ptr<MainAppDataModel>>>();
        evil::mvc::CommandFactory::unregisterType<mvc::NullCommand>();
        evil::mvc::ViewModelFactory::unregisterViewType<evil::daw::MainAppWindow>();
    }

    bool MainAppController::onCanExecuteCommand(const mvc::BaseCommand& command)
    {
        if (dynamic_cast<const mvc::NullCommand*>(&command) != nullptr)
        {
            return true;
        }

        if (dynamic_cast<const mvc::DataModelCommand<std::shared_ptr<MainAppDataModel>>*>(&command) != nullptr)
        {
            return true;
        }

        return false;
    }

    mvc::ValidationResult MainAppController::onExecuteCommand(const mvc::BaseCommand& command)
    {
        if (dynamic_cast<const mvc::NullCommand*>(&command) != nullptr)
        {
            return { true, {} };
        }
        if (dynamic_cast<const mvc::DataModelCommand<std::shared_ptr<MainAppDataModel>>*>(&command) != nullptr)
        {
            return { true, {} };
        }
        return { false, { "Unknown command" } };
    }

    // -------------------------------------------------------------------------

    void MainAppController::setDataModel(std::shared_ptr<mvc::BaseDataModel> model)
    {
        _typedModelWeak = std::dynamic_pointer_cast<MainAppDataModel>(model);
        BaseController::setDataModel(std::move(model));
    }
}
