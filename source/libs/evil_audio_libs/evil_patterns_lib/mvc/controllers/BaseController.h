/**
 * @file BaseController.h
 * @brief Declares the `BaseController` base class for MVC controllers.
 *
 * A `BaseController` mediates between a `BaseDataModel` and one or more
 * `BaseView` instances. It manages registered views, wires lifecycle callbacks,
 * creates commands/view-models, and coordinates model observer attachment.
 *
 * Subclasses typically override protected hooks (`onView*`, command creation/
 * execution hooks, etc.) to customize behavior.
 *
 * @see BaseView
 * @see BaseDataModel
 */

#pragma once

#include <juce_core/juce_core.h>
#include <any>
#include <map>
#include <memory>
#include <mutex>
#include <type_traits>
#include <typeindex>
#include <vector>

#include "mvc/views/BaseView.h"
#include "mvc/commands/BaseCommand.h"
#include "mvc/datamodels/BaseDataModel.h"
#include "mvc/validations/ValidationResult.h"
#include "mvc/viewmodels/BaseViewModel.h"
#include <concepts>

namespace evil::mvc
{
class BaseController
{
public:
    BaseController();
    virtual ~BaseController();

    /**
     * @brief Returns a shared_ptr to the current data model, or nullptr if expired.
     *
     * The returned shared_ptr keeps the model alive for the duration of the caller's
     * use. Prefer this over storing the raw pointer.
     */
    std::shared_ptr<BaseDataModel> getDataModel() const;

    void addView(const std::shared_ptr<BaseView>& view);
    void removeView(BaseView* view);
    std::vector<BaseView*> getViews() const;

    BaseViewModel* getViewModel(const BaseView* view) const;

    template <typename TView = BaseView>
        requires std::derived_from<TView, BaseView>
    TView* getView(const BaseViewModel* viewModel) const
    {
        return dynamic_cast<TView*>(getViewInternal(viewModel));
    }

    void clearViews();

protected:
    virtual void setDataModel(std::shared_ptr<BaseDataModel> dataModel);

    /**
     * @brief Creates a view-model for the given runtime view type.
     *
     * The @p viewType is the `std::type_index` of the *view* (not the view-model),
     * matching the key used by `ViewModelFactory::registerViewType<TView>()`.
     *
     * @return Owning `std::unique_ptr<BaseViewModel>`, or `nullptr` if not registered.
     */
    virtual std::unique_ptr<BaseViewModel> onCreateViewModel(std::type_index viewType,
                                                             const std::vector<std::any>& args);
    virtual std::unique_ptr<BaseViewModel> onCreateViewModel(std::type_index viewType);

    virtual bool onViewInit(BaseView& view);
    virtual bool onViewShow(BaseView& view);
    virtual bool onViewHide(BaseView& view);
    virtual bool onViewDestroy(BaseView& view);

    virtual std::unique_ptr<BaseCommand> onCreateCommand(std::type_index commandType,
                                                          const std::vector<std::any>& args);
    virtual std::unique_ptr<BaseCommand> onCreateCommand(std::type_index commandType);

    virtual bool onCanExecuteCommand(const BaseCommand& command);
    virtual ValidationResult onExecuteCommand(const BaseCommand& command);

    virtual void attachControllerToView(BaseView* view);
    virtual void detachControllerFromView(BaseView* view);

    virtual void attachViewModelToDataModel(BaseViewModel* viewModel);
    virtual void detachViewModelFromDataModel(BaseViewModel* viewModel);

    static void pruneExpired(std::vector<std::shared_ptr<BaseView>>& views);

private:
    using ViewModelMap = std::map<std::shared_ptr<BaseView>,
                         std::unique_ptr<BaseViewModel>,
                         std::owner_less<std::shared_ptr<BaseView>>>;

    BaseView* getViewInternal(const BaseViewModel* viewModel) const;

    // Guards _views and _viewModels. Virtual hooks (attach/detach) are always
    // called outside this lock to avoid deadlocks with BaseDataModel::_observersMutex.
    mutable std::mutex _viewsMutex;

    ViewModelMap _viewModels;
    std::weak_ptr<BaseDataModel> _dataModelWeak;
    std::vector<std::shared_ptr<BaseView>> _views;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BaseController);

};

} // namespace evil::mvc