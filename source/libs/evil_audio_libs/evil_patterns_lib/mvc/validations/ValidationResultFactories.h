#pragma once

#include <string>
#include <utility>
#include <vector>

#include "ValidationResult.h"

namespace evil::mvc
{
    /**
     * @brief Creates a successful validation result.
     * @return ValidationResult with `success=true` and no errors.
     */
    inline ValidationResult makeSuccess()
    {
        ValidationResult result;
        result.success = true;
        result.errors.clear();
        return result;
    }

    /**
     * @brief Creates a failed validation result with one error.
     * @param error Error message to include.
     * @return ValidationResult with `success=false`.
     */
    inline ValidationResult makeFailure(const std::string& error)
    {
        ValidationResult result;
        result.success = false;
        result.errors = { error };
        return result;
    }

    /**
     * @brief Creates a failed validation result with multiple errors.
     * @param errors Error messages to include.
     * @return ValidationResult with `success=false`.
     */
    inline ValidationResult makeFailure(std::vector<std::string> errors)
    {
        ValidationResult result;
        result.success = false;
        result.errors = std::move(errors);
        return result;
    }
}
