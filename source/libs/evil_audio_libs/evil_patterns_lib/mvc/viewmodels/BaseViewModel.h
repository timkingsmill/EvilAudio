#pragma once

#include <concepts>
#include <functional>
#include <typeindex>

#include "mvc/views/BaseView.h"
#include "mvc/datamodels/BaseDataModel.h"
#include "mvc/datamodels/BaseDataModelObserver.h"

namespace evil::mvc
{
    class BaseView;

    class BaseViewModel : public BaseDataModelObserver<BaseDataModel>
    {
    public:
        BaseViewModel() = default;

        using RequestViewCallback = std::function<BaseView*()>;
        void setRequestViewCallback(RequestViewCallback callback);

        template <typename TView>
            requires std::derived_from<TView, BaseView>
        TView* requestView() const
        {
            return dynamic_cast<TView*>(requestViewInternal());
        }

    protected:
        template <typename TDataModel>
            requires std::derived_from<TDataModel, BaseDataModel>
        void bindViewToDataModel(const TDataModel& dataModel)
        {
            bindViewToDataModelImpl(static_cast<const BaseDataModel&>(dataModel),
                                    std::type_index(typeid(dataModel)));
        }

        /**
         * @brief Routes a data-model notification into `bindViewToDataModelImpl`.
         *
         * `dataModelType` is the concrete runtime type supplied by
         * `BaseDataModel::notifyObservers()` via `typeid(*this)`. It is forwarded
         * to `bindViewToDataModelImpl` so subclasses can guard on the exact model
         * type without performing an additional `typeid` call or `dynamic_cast`.
         */
        void handleDataModelEvent(const BaseDataModel& dataModel,
                                  std::type_index dataModelType) override
        {
            bindViewToDataModelImpl(dataModel, dataModelType);
        }

        /**
         * @brief Override to push data-model state into the bound view.
         *
         * @param dataModel      The model whose state changed.
         * @param dataModelType  The concrete runtime type of @p dataModel,
         *                       as supplied by `typeid(*this)` in the model.
         *                       Use this to guard against unexpected model types
         *                       before performing a `static_cast` or `dynamic_cast`.
         *
         * Call `requestView<TView>()` here — do not cache the result.
         * The view pointer is always resolved fresh through the controller so
         * that stale/replaced views are never accessed.
         *
         * @code
         * void MyViewModel::bindViewToDataModelImpl(const BaseDataModel& dataModel,
         *                                           std::type_index dataModelType)
         * {
         *     if (dataModelType != std::type_index(typeid(MyDataModel)))
         *         return;
         *     const auto& model = static_cast<const MyDataModel&>(dataModel);
         *     if (auto* view = requestView<MyView>())
         *         view->setTitle(model.getTitle());
         * }
         * @endcode
         */
        virtual void bindViewToDataModelImpl(const BaseDataModel& dataModel,
                                             std::type_index dataModelType)
        {
            (void)dataModel;
            (void)dataModelType;
        }

    private:
        BaseView* requestViewInternal() const;
        RequestViewCallback _requestViewCallback;
    };
}