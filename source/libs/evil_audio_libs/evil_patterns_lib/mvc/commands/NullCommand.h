#pragma once

#include "BaseCommand.h"

namespace evil::mvc
{
    /**
     * @brief Null-object implementation of the command contract.
     *
     * @details
     * This command performs no action and is always disabled. It serves as a safe default when a
     * valid command cannot be provided.
     */
    class NullCommand : public BaseCommand
    {
    public:
        NullCommand();

    protected:
        std::string_view getCommandKey() const noexcept override;
        bool onExecute() override;
    };
}