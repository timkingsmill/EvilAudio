#pragma once

#include "BaseCommand.h"
#include "mvc/datamodels/BaseDataModel.h"

#include <concepts>
#include <memory>
#include <string_view>
#include <utility>

namespace evil::mvc
{
    /**
     * @brief Concept that constrains TDataModel to a pointer-like handle to a BaseDataModel.
     *
     * The handle must be contextually convertible to bool (null check) and must expose
     * a BaseDataModel through its arrow operator. Satisfied by std::shared_ptr<T>,
     * std::unique_ptr<T>, or raw pointer T* where T derives from BaseDataModel.
     */
    template <typename TDataModel>
    concept ModelHandle = requires(TDataModel m)
    {
        { static_cast<bool>(m) } -> std::convertible_to<bool>;
        { m.operator->() } -> std::convertible_to<BaseDataModel*>;
    };

    /**
     * @brief Base command that holds a typed data model handle.
     *
     * Subclasses override `onExecute()` to implement their specific command logic
     * using `getDataModel()` to access the model. The base implementation does
     * nothing and returns false — override in concrete commands.
     */
    template <ModelHandle TDataModel>
    class DataModelCommand : public BaseCommand
    {
    public:
        explicit DataModelCommand(TDataModel dataModel)
            : dataModel_(std::move(dataModel))
        {
        }

        ~DataModelCommand() override = default;

        [[nodiscard]] std::string_view getCommandKey() const noexcept override
        {
            return "DataModelCommand";
        }

    protected:
        [[nodiscard]] bool onExecute() override
        {
            return false;
        }

        virtual TDataModel getDataModel() const
        {
            return dataModel_;
        }

    private:
        TDataModel dataModel_;
    };
} // namespace evil::mvc
