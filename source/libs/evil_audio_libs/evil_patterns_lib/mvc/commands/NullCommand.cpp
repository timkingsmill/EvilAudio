#include "NullCommand.h"

namespace evil::mvc
{
    NullCommand::NullCommand()
        : BaseCommand()
    {
        setEnabled(false);
    }

    std::string_view NullCommand::getCommandKey() const noexcept
    {
        return "NullCommand";
    }

    bool NullCommand::onExecute()
    {
        return false;
    }
} // namespace evil::mvc