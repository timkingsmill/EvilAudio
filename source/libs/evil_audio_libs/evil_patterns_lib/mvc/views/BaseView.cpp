#include "BaseView.h"
#include "mvc/commands/BaseCommand.h"
#include "mvc/validations/ValidationResult.h"
#include <typeindex>
#include <vector>
#include <any>

namespace evil::mvc
{
    std::unique_ptr<BaseCommand> BaseView::createCommand(std::type_index commandType, 
                                                          const std::vector<std::any>& args) const
    {
        if (onCreateCommand)
            return onCreateCommand(commandType, args);
        return nullptr;
    }

    std::unique_ptr<BaseCommand> BaseView::createCommand(std::type_index commandType) const
    {
        return createCommand(commandType, {});
    }

    bool BaseView::canExecuteCommand(const BaseCommand& command) const
    {
        if (onCanExecuteCommand)
            return onCanExecuteCommand(command);
        return false;
    }

    ValidationResult BaseView::executeCommand(const BaseCommand& command) const
    {
        if (onExecuteCommand)
            return onExecuteCommand(command);
        return ValidationResult();
    }
}
