#include <algorithm>
#include <memory>
#include <mutex>
#include <typeindex>
#include <utility>
#include <vector>

#include "mvc/commands/BaseCommand.h"
#include "mvc/controllers/BaseController.h"
#include "mvc/views/BaseView.h"
#include "mvc/commands/CommandFactory.h"
#include "mvc/datamodels/BaseDataModel.h"
#include "mvc/datamodels/BaseDataModelObserver.h"
#include "mvc/validations/ValidationResult.h"
#include "mvc/viewmodels/BaseViewModel.h"
#include "mvc/viewmodels/ViewModelFactory.h"

namespace evil::mvc
{
    BaseController::BaseController() = default;
    BaseController::~BaseController()
    {
        clearViews();
    };

    void BaseController::setDataModel(std::shared_ptr<BaseDataModel> dataModel)
    {
        // Collect view models to detach/attach outside the lock to avoid
        // holding _viewsMutex while calling into BaseDataModel::_observersMutex.
        std::vector<BaseViewModel*> toDetach;
        std::vector<BaseViewModel*> toAttach;

        {
            std::lock_guard lock(_viewsMutex);

            if (_dataModelWeak.lock())
                for (const auto& [view, viewModel] : _viewModels)
                    toDetach.push_back(viewModel.get());

            _dataModelWeak = dataModel;

            if (dataModel)
                for (const auto& [view, viewModel] : _viewModels)
                    toAttach.push_back(viewModel.get());
        }

        for (auto* vm : toDetach)
            detachViewModelFromDataModel(vm);

        for (auto* vm : toAttach)
            attachViewModelToDataModel(vm);
    }

    std::shared_ptr<BaseDataModel> BaseController::getDataModel() const
    {
        return _dataModelWeak.lock();
    }

    void BaseController::pruneExpired(std::vector<std::shared_ptr<BaseView>>& views)
    {
        for (auto it = views.begin(); it != views.end(); )
        {
            if (!(*it))
                it = views.erase(it);
            else
                ++it;
        }
    }

    void BaseController::addView(const std::shared_ptr<BaseView>& view)
    {
        if (!view)
            return;

        // Create the view model outside the lock — factory lookup has its own
        // internal state and must not run while we hold _viewsMutex.
        //const std::type_index viewType(typeid(*view));

        const std::type_index viewType(typeid(*view));
        //
        auto viewModel = onCreateViewModel(viewType, std::vector<std::any>{ view.get() });

        if (viewModel)
        {
            viewModel->setRequestViewCallback([this, rawView = view.get()]()
            {
                return getView(getViewModel(rawView));
            });
        }

        BaseViewModel* viewModelRaw = nullptr;
        {
            std::lock_guard lock(_viewsMutex);

            pruneExpired(_views);

            const bool exists = std::any_of(_views.begin(), _views.end(),
                [&view](const auto& existingView)
                {
                    return existingView.get() == view.get();
                });

            if (exists)
                return;

            if (viewModel)
            {
                viewModelRaw = viewModel.get();
                _viewModels[view] = std::move(viewModel);
            }

            _views.push_back(view);
        }

        // Attach hooks and observer outside the lock.
        attachControllerToView(view.get());
        attachViewModelToDataModel(viewModelRaw);
    }

    void BaseController::removeView(BaseView* view)
    {
        if (view == nullptr)
            return;

        BaseViewModel* viewModelToDetach = nullptr;
        {
            std::lock_guard lock(_viewsMutex);

            auto viewIt = std::find_if(_views.begin(), _views.end(),
                [view](const std::shared_ptr<BaseView>& sharedView)
                {
                    return sharedView.get() == view;
                });

            if (viewIt != _views.end())
            {
                auto it = _viewModels.find(*viewIt);
                if (it != _viewModels.end())
                {
                    it->second->setRequestViewCallback({});
                    viewModelToDetach = it->second.get();
                    _viewModels.erase(it);
                }
                _views.erase(viewIt);
            }
        }

        // Detach hooks and observer outside the lock.
        detachControllerFromView(view);
        detachViewModelFromDataModel(viewModelToDetach);
    }

    void BaseController::attachViewModelToDataModel(BaseViewModel* viewModel)
    {
        if (viewModel == nullptr)
            return;

        if (auto dataModel = getDataModel())
            dataModel->addObserver(static_cast<BaseDataModelObserver<BaseDataModel>*>(viewModel));
    }

    void BaseController::detachViewModelFromDataModel(BaseViewModel* viewModel)
    {
        if (viewModel == nullptr)
            return;

        if (auto dataModel = getDataModel())
            dataModel->removeObserver(static_cast<BaseDataModelObserver<BaseDataModel>*>(viewModel));
    }

    std::vector<BaseView*> BaseController::getViews() const
    {
        std::lock_guard lock(_viewsMutex);

        std::vector<BaseView*> result;
        result.reserve(_views.size());
        for (const auto& view : _views)
            result.push_back(view.get());
        return result;
    }

    BaseViewModel* BaseController::getViewModel(const BaseView* view) const
    {
        if (view == nullptr)
            return nullptr;

        std::lock_guard lock(_viewsMutex);

        auto viewIt = std::find_if(_views.begin(), _views.end(),
            [view](const std::shared_ptr<BaseView>& sharedView)
            {
                return sharedView.get() == view;
            });

        if (viewIt == _views.end())
            return nullptr;

        auto it = _viewModels.find(*viewIt);
        if (it == _viewModels.end())
            return nullptr;

        return it->second.get();
    }

    BaseView* BaseController::getViewInternal(const BaseViewModel* viewModel) const
    {
        if (viewModel == nullptr)
            return nullptr;

        std::lock_guard lock(_viewsMutex);

        for (const auto& [view, mappedViewModel] : _viewModels)
        {
            if (mappedViewModel.get() == viewModel)
                return view.get();
        }

        return nullptr;
    }

    void BaseController::clearViews()
    {
        ViewModelMap viewModels;
        std::vector<std::shared_ptr<BaseView>> views;

        {
            std::lock_guard lock(_viewsMutex);
            viewModels = std::move(_viewModels);
            views      = std::move(_views);
        }

        // Detach everything outside the lock.
        for (const auto& view : views)
            detachControllerFromView(view.get());

        for (const auto& [view, viewModel] : viewModels)
        {
            viewModel->setRequestViewCallback({});
            detachViewModelFromDataModel(viewModel.get());
        }
    }

    bool BaseController::onViewInit(BaseView& view)   { (void)view; return true; }
    bool BaseController::onViewShow(BaseView& view)   { (void)view; return true; }
    bool BaseController::onViewHide(BaseView& view)   { (void)view; return true; }

    bool BaseController::onViewDestroy(BaseView& view)
    {
        detachControllerFromView(&view);
        return true;
    }

    std::unique_ptr<BaseViewModel> BaseController::onCreateViewModel(std::type_index viewType,
                                                                     const std::vector<std::any>& args)
    {
        return ViewModelFactory::create(viewType, args);
    }

    std::unique_ptr<BaseViewModel> BaseController::onCreateViewModel(std::type_index viewType)
    {
        return onCreateViewModel(viewType, {});
    }

    std::unique_ptr<BaseCommand> BaseController::onCreateCommand(std::type_index commandType,
                                                              const std::vector<std::any>& args)
    {
        return CommandFactory::create(commandType, args);
    }

    std::unique_ptr<BaseCommand> BaseController::onCreateCommand(std::type_index commandType)
    {
        return onCreateCommand(commandType, {});
    }

    bool BaseController::onCanExecuteCommand(const BaseCommand& command) { (void)command; return false; }

    ValidationResult BaseController::onExecuteCommand(const BaseCommand& command)
    {
        (void)command;
        return ValidationResult{};
    }

    void BaseController::attachControllerToView(BaseView* view)
    {
        if (view == nullptr)
            return;

        view->onInit    = [this](BaseView& v) { return onViewInit(v); };
        view->onShow    = [this](BaseView& v) { return onViewShow(v); };
        view->onHide    = [this](BaseView& v) { return onViewHide(v); };
        view->onDestroy = [this](BaseView& v) { return onViewDestroy(v); };

        view->onCreateCommand = [this](std::type_index t, const std::vector<std::any>& a) -> std::unique_ptr<BaseCommand>
        {
            return onCreateCommand(t, a);
        };
        view->onCanExecuteCommand = [this](const BaseCommand& c) { return onCanExecuteCommand(c); };
        view->onExecuteCommand    = [this](const BaseCommand& c) { return onExecuteCommand(c); };
    }

    void BaseController::detachControllerFromView(BaseView* view)
    {
        if (view == nullptr)
            return;

        view->onCreateCommand     = {};
        view->onCanExecuteCommand = {};
        view->onExecuteCommand    = {};
        view->onInit              = {};
        view->onShow              = {};
        view->onHide              = {};
        view->onDestroy           = {};
    }
}