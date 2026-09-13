#include "BaseCommand.h"

namespace evil::mvc
{
    BaseCommand::BaseCommand() = default;
    BaseCommand::~BaseCommand() = default;

    bool BaseCommand::execute()
    {
        return enabled_ && onExecute();
    }

    bool BaseCommand::isEnabled() const noexcept
    {
        return enabled_;
    }

    void BaseCommand::setEnabled(bool enabled) noexcept
    {
        enabled_ = enabled;
    }

    bool BaseCommand::isError() const noexcept
    {
        return !errorMessages_.empty();
    }

    const std::vector<std::string>& BaseCommand::getErrorMessages() const noexcept
    {
        return errorMessages_;
    }

    void BaseCommand::addErrorMessage(std::string message)
    {
        errorMessages_.push_back(std::move(message));
    }

    void BaseCommand::clearErrorMessages() noexcept
    {
        errorMessages_.clear();
    }

    std::string_view BaseCommand::getName() const noexcept
    {
        return getCommandKey();
    }
}
