#pragma once

/**
 * @file Command.h
 * @brief Base interface for command objects.
 *
 * @details
 * Defines the minimal command contract shared by command implementations.
 */

#include <string>
#include <string_view>
#include <vector>

namespace evil::mvc
{

	// =================================================================================================

	/**
	 * @brief Abstract command contract.
	 */
	class BaseCommand
	{
	public:
		/** @brief Constructs the command with a stable command key. */
		explicit BaseCommand();

		/** @brief Destroys the command through a base pointer. */
		virtual ~BaseCommand();

		/**
		 * @brief Executes the command action.
		 * @return `true` if execution succeeded; otherwise `false`.
		 */
		virtual bool execute();


		/**
		 * @brief Returns whether this command is currently enabled.
		 * @return `true` if the command accepts execution requests; otherwise `false`.
		 */
		[[nodiscard]] virtual bool isEnabled() const noexcept;

		/** @brief Enables or disables this command. */
		virtual void setEnabled(bool enabled) noexcept;

		/**
		 * @brief Returns whether the command is in an error state.
		 * @return `true` if the command has an error condition; otherwise `false`.
		 */
		[[nodiscard]] virtual bool isError() const noexcept;

		/**
		 * @brief Returns all command error messages.
		 * @return Immutable view of the current error message list.
		 */
		[[nodiscard]] virtual const std::vector<std::string>& getErrorMessages() const noexcept;

		/**
		 * @brief Appends a new error message to this command.
		 * @param message Error description to add.
		 */
		virtual void addErrorMessage(std::string message);

		/** @brief Clears all current error messages. */
		virtual void clearErrorMessages() noexcept;

		/**
		 * @brief Returns a stable command name for logging and UI display.
		 * @return Human-readable command name.
		 */
		[[nodiscard]] virtual std::string_view getName() const noexcept;


	protected:
		virtual std::string_view getCommandKey() const noexcept = 0;
		/**
		 * @brief Override this method to implement the command's execution logic.
		 * @return `true` if execution succeeded; otherwise `false`.
		 */
		virtual bool onExecute() = 0;

	private:
		/// Tracks whether the command may currently be invoked.
		bool enabled_ = true;
		std::vector<std::string> errorMessages_;
	};
}