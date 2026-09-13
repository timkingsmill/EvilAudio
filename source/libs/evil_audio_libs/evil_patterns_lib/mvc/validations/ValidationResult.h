#pragma once

#include <string>
#include <vector>

namespace evil::mvc
{
    struct ValidationResult
    {
        /// Indicates whether the operation succeeded.
        bool success = false;
        /// Collection of validation or processing errors.
        std::vector<std::string> errors;
    };
}
