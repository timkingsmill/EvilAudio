/**
 * @file edaw_property_bindings.h
 * @brief Property binding utilities for synchronizing values between different objects and JUCE data structures.
 *
 * This file provides a comprehensive set of classes and factory functions for creating bindings between
 * properties, enabling automatic synchronization of values across different parts of an application.
 * Supports one-way and two-way bindings with optional value transformations.
 */

#pragma once

#include <functional>
#include <optional>
#include <type_traits>
#include <utility>

#include <juce_data_structures.h>

namespace evil
{
	namespace detail
	{
		/**
		 * @class ScopedUpdateFlag
		 * @brief RAII wrapper for setting and resetting a boolean flag.
		 *
		 * Sets the flag to true on construction and false on destruction.
		 * Used to prevent infinite recursion in bidirectional bindings.
		 */
		class ScopedUpdateFlag
		{
		public:
			/**
			 * @brief Constructs a scoped update flag and sets it to true.
			 * @param flag Reference to the boolean flag to manage.
			 */
			explicit ScopedUpdateFlag(bool& flag) noexcept;

			ScopedUpdateFlag(const ScopedUpdateFlag&) = delete;
			ScopedUpdateFlag& operator=(const ScopedUpdateFlag&) = delete;

			/**
			 * @brief Destructor that resets the flag to false.
			 */
			~ScopedUpdateFlag() noexcept;

		private:
			bool& flag;
		};
	};

	/**
	 * @class PropertyBinding
	 * @brief One-way property binding that synchronizes values from a source to a target.
	 *
	 * @tparam SourceType The type of the source object.
	 * @tparam TargetType The type of the target object.
	 * @tparam ValueType The type of the value being synchronized.
	 *
	 * This class allows manual synchronization of a property value from a source object to a target object,
	 * with optional value transformation. The sync() method must be called explicitly to propagate changes.
	 */
	template <typename SourceType, typename TargetType, typename ValueType>
	class PropertyBinding
	{
	public:
		using Getter = std::function<ValueType(const SourceType&)>;
		using Setter = std::function<void(TargetType&, const ValueType&)>;
		using Transform = std::function<ValueType(ValueType)>;

		/**
		 * @brief Constructs a property binding.
		 * @param source Reference to the source object.
		 * @param target Reference to the target object.
		 * @param getter Function to retrieve the value from the source.
		 * @param setter Function to set the value on the target.
		 * @param transform Optional function to transform the value during synchronization.
		 */
		PropertyBinding(SourceType& source,
			TargetType& target,
			Getter getter,
			Setter setter,
			Transform transform = {})
			: source(source),
			target(target),
			getter(std::move(getter)),
			setter(std::move(setter)),
			transform(std::move(transform))
		{};

		/**
		 * @brief Synchronizes the value from source to target.
		 *
		 * Retrieves the value from the source using the getter, applies the transform if provided,
		 * and sets the result on the target using the setter.
		 */
		void sync() const
		{
			auto value = getter(source);
			if (transform)
			{
				value = transform(std::move(value));
			}

			setter(target, value);
		};

	private:
		SourceType& source;
		TargetType& target;
		Getter getter;
		Setter setter;
		Transform transform;
	};

	/**
	 * @class TwoWayPropertyBinding
	 * @brief Bidirectional property binding that keeps two properties synchronized.
	 *
	 * @tparam LeftType The type of the left object.
	 * @tparam RightType The type of the right object.
	 * @tparam ValueType The type of the value being synchronized.
	 *
	 * Provides manual bidirectional synchronization between two properties with optional transformations
	 * in each direction. Includes comparison logic to prevent unnecessary updates and guards against
	 * infinite recursion. Tracks the last synchronized value.
	 */
	template <typename LeftType, typename RightType, typename ValueType>
	class TwoWayPropertyBinding
	{
	public:
		using LeftGetter = std::function<ValueType(const LeftType&)>;
		using LeftSetter = std::function<void(LeftType&, const ValueType&)>;
		using RightGetter = std::function<ValueType(const RightType&)>;
		using RightSetter = std::function<void(RightType&, const ValueType&)>;
		using Transform = std::function<ValueType(ValueType)>;
		using Comparator = std::function<bool(const ValueType&, const ValueType&)>;

		/**
		 * @brief Constructs a two-way property binding.
		 * @param left Reference to the left object.
		 * @param right Reference to the right object.
		 * @param leftGetter Function to retrieve the value from the left object.
		 * @param leftSetter Function to set the value on the left object.
		 * @param rightGetter Function to retrieve the value from the right object.
		 * @param rightSetter Function to set the value on the right object.
		 * @param leftToRightTransform Optional function to transform values when syncing left to right.
		 * @param rightToLeftTransform Optional function to transform values when syncing right to left.
		 * @param comparator Optional function to compare values for equality. Defaults to operator==.
		 */
		TwoWayPropertyBinding(LeftType& left,
			RightType& right,
			LeftGetter leftGetter,
			LeftSetter leftSetter,
			RightGetter rightGetter,
			RightSetter rightSetter,
			Transform leftToRightTransform = {},
			Transform rightToLeftTransform = {},
			Comparator comparator = {})
			: left(left),
			right(right),
			leftGetter(std::move(leftGetter)),
			leftSetter(std::move(leftSetter)),
			rightGetter(std::move(rightGetter)),
			rightSetter(std::move(rightSetter)),
			leftToRightTransform(std::move(leftToRightTransform)),
			rightToLeftTransform(std::move(rightToLeftTransform)),
			comparator(std::move(comparator))
		{
			if (!this->comparator)
			{
				this->comparator = [](const ValueType& a, const ValueType& b) { return a == b; };
			}
		};

		TwoWayPropertyBinding(const TwoWayPropertyBinding&) = delete;
		TwoWayPropertyBinding& operator=(const TwoWayPropertyBinding&) = delete;
		TwoWayPropertyBinding(TwoWayPropertyBinding&&) = delete;
		TwoWayPropertyBinding& operator=(TwoWayPropertyBinding&&) = delete;

		/**
		 * @brief Synchronizes the value from left to right.
		 * @return true if the value was updated, false if already synchronized or an update is in progress.
		 */
		bool syncLeftToRight()
		{
			if (isUpdating)
			{
				return false;
			}

			auto value = leftGetter(left);
			if (leftToRightTransform)
			{
				value = leftToRightTransform(std::move(value));
			}

			const auto currentRightValue = rightGetter(right);
			if (comparator(currentRightValue, value))
			{
				lastSyncedValue = value;
				return false;
			}

			detail::ScopedUpdateFlag updateGuard{ isUpdating };
			rightSetter(right, value);
			lastSyncedValue = value;
			return true;
		};

		/**
		 * @brief Synchronizes the value from right to left.
		 * @return true if the value was updated, false if already synchronized or an update is in progress.
		 */
		bool syncRightToLeft()
		{
			if (isUpdating)
			{
				return false;
			}

			auto value = rightGetter(right);
			if (rightToLeftTransform)
			{
				value = rightToLeftTransform(std::move(value));
			}

			const auto currentLeftValue = leftGetter(left);
			if (comparator(currentLeftValue, value))
			{
				lastSyncedValue = value;
				return false;
			}

			detail::ScopedUpdateFlag updateGuard{ isUpdating };
			leftSetter(left, value);
			lastSyncedValue = value;
			return true;
		};

		/**
		 * @brief Gets the last successfully synchronized value.
		 * @return Optional containing the last synced value, or empty if no sync has occurred.
		 */
		const std::optional<ValueType>& getLastSyncedValue() const
		{
			return lastSyncedValue;
		};

	private:
		LeftType& left;
		RightType& right;
		LeftGetter leftGetter;
		LeftSetter leftSetter;
		RightGetter rightGetter;
		RightSetter rightSetter;
		Transform leftToRightTransform;
		Transform rightToLeftTransform;
		Comparator comparator;
		bool isUpdating = false;
		std::optional<ValueType> lastSyncedValue;
	};

	/**
	 * @class ValueBinding
	 * @brief One-way binding between JUCE Value objects with automatic synchronization.
	 *
	 * Listens to changes on the source Value and automatically propagates them to the target Value.
	 * Supports optional value transformation. The binding is active for the lifetime of the object.
	 */
	class ValueBinding : private juce::Value::Listener
	{
	public:
		using Transform = std::function<juce::var(juce::var)>;

		/**
		 * @brief Constructs a value binding and performs initial synchronization.
		 * @param source Reference to the source juce::Value to monitor.
		 * @param target Reference to the target juce::Value to update.
		 * @param transform Optional function to transform the value during synchronization.
		 */
		ValueBinding(juce::Value& source,
			juce::Value& target,
			Transform transform = {});

		/**
		 * @brief Destructor that removes the listener from the source.
		 */
		~ValueBinding() override;

		ValueBinding(const ValueBinding&) = delete;
		ValueBinding& operator=(const ValueBinding&) = delete;
		ValueBinding(ValueBinding&&) = delete;
		ValueBinding& operator=(ValueBinding&&) = delete;

		/**
		 * @brief Manually synchronizes the value from source to target.
		 */
		void sync();

	private:
		/**
		 * @brief Callback invoked when the source value changes.
		 * @param changedValue The juce::Value that changed.
		 */
		void valueChanged(juce::Value& changedValue) override;

		juce::Value& source;
		juce::Value& target;
		Transform transform;
	};

	/**
	 * @class TwoWayValueBinding
	 * @brief Bidirectional binding between two JUCE Value objects with automatic synchronization.
	 *
	 * Listens to changes on both Values and automatically synchronizes them in either direction.
	 * Supports optional transformations in each direction and custom comparison logic.
	 * Prevents infinite recursion using an update flag.
	 */
	class TwoWayValueBinding : private juce::Value::Listener
	{
	public:
		using Transform = std::function<juce::var(juce::var)>;
		using Comparator = std::function<bool(const juce::var&, const juce::var&)>;

		/**
		 * @brief Constructs a two-way value binding.
		 * @param left Reference to the left juce::Value.
		 * @param right Reference to the right juce::Value.
		 * @param leftToRightTransform Optional function to transform values when syncing left to right.
		 * @param rightToLeftTransform Optional function to transform values when syncing right to left.
		 * @param comparator Optional function to compare values for equality. Defaults to operator==.
		 */
		TwoWayValueBinding(juce::Value& left,
			juce::Value& right,
			Transform leftToRightTransform = {},
			Transform rightToLeftTransform = {},
			Comparator comparator = {});

		/**
		 * @brief Destructor that removes listeners from both values.
		 */
		~TwoWayValueBinding() override;

		TwoWayValueBinding(const TwoWayValueBinding&) = delete;
		TwoWayValueBinding& operator=(const TwoWayValueBinding&) = delete;
		TwoWayValueBinding(TwoWayValueBinding&&) = delete;
		TwoWayValueBinding& operator=(TwoWayValueBinding&&) = delete;

		/**
		 * @brief Manually synchronizes the value from left to right.
		 * @return true if the value was updated, false if already synchronized or an update is in progress.
		 */
		bool syncLeftToRight();

		/**
		 * @brief Manually synchronizes the value from right to left.
		 * @return true if the value was updated, false if already synchronized or an update is in progress.
		 */
		bool syncRightToLeft();

	private:
		/**
		 * @brief Callback invoked when either value changes.
		 * @param changedValue The juce::Value that changed.
		 */
		void valueChanged(juce::Value& changedValue) override;

		juce::Value& left;
		juce::Value& right;
		Transform leftToRightTransform;
		Transform rightToLeftTransform;
		Comparator comparator;
		bool isUpdating = false;
	};

	/**
	 * @class ValueTreePropertyBinding
	 * @brief One-way binding from a JUCE ValueTree property to a JUCE Value with automatic synchronization.
	 *
	 * Listens to changes on a specific property in a ValueTree and automatically propagates them to a Value.
	 * Supports optional value transformation. Performs runtime validity checks on the ValueTree.
	 */
	class ValueTreePropertyBinding : private juce::ValueTree::Listener
	{
	public:
		using Transform = std::function<juce::var(juce::var)>;

		/**
		 * @brief Constructs a ValueTree property binding and performs initial synchronization.
		 * @param sourceTree The juce::ValueTree to monitor.
		 * @param sourceProperty The property identifier to watch for changes.
		 * @param targetValue Reference to the target juce::Value to update.
		 * @param transform Optional function to transform the value during synchronization.
		 */
		ValueTreePropertyBinding(juce::ValueTree sourceTree,
			juce::Identifier sourceProperty,
			juce::Value& targetValue,
			Transform transform = {});

		/**
		 * @brief Destructor that removes the listener from the ValueTree if valid.
		 */
		~ValueTreePropertyBinding() override;

		ValueTreePropertyBinding(const ValueTreePropertyBinding&) = delete;
		ValueTreePropertyBinding& operator=(const ValueTreePropertyBinding&) = delete;
		ValueTreePropertyBinding(ValueTreePropertyBinding&&) = delete;
		ValueTreePropertyBinding& operator=(ValueTreePropertyBinding&&) = delete;

		/**
		 * @brief Manually synchronizes the property value from the ValueTree to the target Value.
		 */
		void sync();

	private:
		/**
		 * @brief Callback invoked when a property in the ValueTree changes.
		 * @param tree The ValueTree that changed.
		 * @param property The identifier of the property that changed.
		 */
		void valueTreePropertyChanged(juce::ValueTree& tree,
			const juce::Identifier& property) override;

		juce::ValueTree sourceTree;
		juce::Identifier sourceProperty;
		juce::Value& targetValue;
		Transform transform;
	};

	/**
	 * @class TwoWayValueTreePropertyBinding
	 * @brief Bidirectional binding between a JUCE ValueTree property and a JUCE Value with automatic synchronization.
	 *
	 * Listens to changes on both the ValueTree property and the Value, automatically synchronizing them
	 * in either direction. Supports optional transformations in each direction and custom comparison logic.
	 * Prevents infinite recursion using an update flag. Performs runtime validity checks on the ValueTree.
	 */
	class TwoWayValueTreePropertyBinding : private juce::ValueTree::Listener,
		private juce::Value::Listener
	{
	public:
		using Transform = std::function<juce::var(juce::var)>;
		using Comparator = std::function<bool(const juce::var&, const juce::var&)>;

		/**
		 * @brief Constructs a two-way ValueTree property binding and performs initial synchronization.
		 * @param sourceTree The juce::ValueTree to monitor and modify.
		 * @param sourceProperty The property identifier to watch for changes.
		 * @param targetValue Reference to the target juce::Value.
		 * @param treeToValueTransform Function to transform values when syncing from tree to value.
		 * @param valueToTreeTransform Function to transform values when syncing from value to tree.
		 * @param comparator Optional function to compare values for equality. Defaults to operator==.
		 */
		TwoWayValueTreePropertyBinding(juce::ValueTree sourceTree,
			juce::Identifier sourceProperty,
			juce::Value& targetValue,
			Transform treeToValueTransform,
			Transform valueToTreeTransform,
			Comparator comparator = {});

		/**
		 * @brief Destructor that removes listeners from both the ValueTree and Value.
		 */
		~TwoWayValueTreePropertyBinding() override;

		TwoWayValueTreePropertyBinding(const TwoWayValueTreePropertyBinding&) = delete;
		TwoWayValueTreePropertyBinding& operator=(const TwoWayValueTreePropertyBinding&) = delete;
		TwoWayValueTreePropertyBinding(TwoWayValueTreePropertyBinding&&) = delete;
		TwoWayValueTreePropertyBinding& operator=(TwoWayValueTreePropertyBinding&&) = delete;

		/**
		 * @brief Manually synchronizes the property from the ValueTree to the Value.
		 * @return true if the value was updated, false if already synchronized, an update is in progress, or the tree is invalid.
		 */
		bool syncTreeToValue();

		/**
		 * @brief Manually synchronizes the Value to the ValueTree property.
		 * @return true if the property was updated, false if already synchronized, an update is in progress, or the tree is invalid.
		 */
		bool syncValueToTree();

	private:
		/**
		 * @brief Callback invoked when a property in the ValueTree changes.
		 * @param tree The ValueTree that changed.
		 * @param property The identifier of the property that changed.
		 */
		void valueTreePropertyChanged(juce::ValueTree& tree,
			const juce::Identifier& property) override;

		/**
		 * @brief Callback invoked when the target Value changes.
		 * @param changedValue The juce::Value that changed.
		 */
		void valueChanged(juce::Value& changedValue) override;

		juce::ValueTree sourceTree;
		juce::Identifier sourceProperty;
		juce::Value& targetValue;
		Transform treeToValueTransform;
		Transform valueToTreeTransform;
		Comparator comparator;
		bool isUpdating = false;
	};

	/**
	 * @brief Factory function to create a PropertyBinding with type deduction.
	 * @tparam SourceType The type of the source object.
	 * @tparam TargetType The type of the target object.
	 * @tparam Getter The getter function type.
	 * @tparam Setter The setter function type.
	 * @tparam Transform The transform function type.
	 * @param source Reference to the source object.
	 * @param target Reference to the target object.
	 * @param getter Function to retrieve the value from the source.
	 * @param setter Function to set the value on the target.
	 * @param transform Optional function to transform the value during synchronization.
	 * @return A PropertyBinding instance with deduced value type.
	 */
	template <typename SourceType, typename TargetType, typename Getter, typename Setter, typename Transform = std::nullptr_t>
	auto makePropertyBinding(SourceType& source,
		TargetType& target,
		Getter&& getter,
		Setter&& setter,
		Transform&& transform = nullptr)
	{
		using ValueType = std::invoke_result_t<Getter, const SourceType&>;
		using BindingType = PropertyBinding<SourceType, TargetType, std::decay_t<ValueType>>;

		if constexpr (std::is_same_v<std::decay_t<Transform>, std::nullptr_t>)
		{
			return BindingType{ source,
						   target,
						   std::forward<Getter>(getter),
						   std::forward<Setter>(setter) };
		}

		return BindingType{ source,
					   target,
					   std::forward<Getter>(getter),
					   std::forward<Setter>(setter),
					   std::forward<Transform>(transform) };
	}

	/**
	 * @brief Factory function to create a TwoWayPropertyBinding with type deduction.
	 * @tparam LeftType The type of the left object.
	 * @tparam RightType The type of the right object.
	 * @tparam LeftGetter The left getter function type.
	 * @tparam LeftSetter The left setter function type.
	 * @tparam RightGetter The right getter function type.
	 * @tparam RightSetter The right setter function type.
	 * @tparam LeftToRightTransform The left-to-right transform function type.
	 * @tparam RightToLeftTransform The right-to-left transform function type.
	 * @tparam Comparator The comparator function type.
	 * @param left Reference to the left object.
	 * @param right Reference to the right object.
	 * @param leftGetter Function to retrieve the value from the left object.
	 * @param leftSetter Function to set the value on the left object.
	 * @param rightGetter Function to retrieve the value from the right object.
	 * @param rightSetter Function to set the value on the right object.
	 * @param leftToRightTransform Optional function to transform values when syncing left to right.
	 * @param rightToLeftTransform Optional function to transform values when syncing right to left.
	 * @param comparator Optional function to compare values for equality.
	 * @return A TwoWayPropertyBinding instance with deduced value type.
	 */
	template <typename LeftType,
		typename RightType,
		typename LeftGetter,
		typename LeftSetter,
		typename RightGetter,
		typename RightSetter,
		typename LeftToRightTransform = std::nullptr_t,
		typename RightToLeftTransform = std::nullptr_t,
		typename Comparator = std::nullptr_t>
	auto makeTwoWayPropertyBinding(LeftType& left,
		RightType& right,
		LeftGetter&& leftGetter,
		LeftSetter&& leftSetter,
		RightGetter&& rightGetter,
		RightSetter&& rightSetter,
		LeftToRightTransform&& leftToRightTransform = nullptr,
		RightToLeftTransform&& rightToLeftTransform = nullptr,
		Comparator&& comparator = nullptr)
	{
		using ValueType = std::invoke_result_t<LeftGetter, const LeftType&>;
		using BindingType = TwoWayPropertyBinding<LeftType, RightType, std::decay_t<ValueType>>;

		return BindingType{ left,
					   right,
					   std::forward<LeftGetter>(leftGetter),
					   std::forward<LeftSetter>(leftSetter),
					   std::forward<RightGetter>(rightGetter),
					   std::forward<RightSetter>(rightSetter),
					   std::forward<LeftToRightTransform>(leftToRightTransform),
					   std::forward<RightToLeftTransform>(rightToLeftTransform),
					   std::forward<Comparator>(comparator) };
	}

	/**
	 * @brief Factory function to create a ValueBinding.
	 * @param source Reference to the source juce::Value.
	 * @param target Reference to the target juce::Value.
	 * @param transform Optional function to transform the value during synchronization.
	 * @return A ValueBinding instance.
	 */
	ValueBinding makeValueBinding(juce::Value& source,
		juce::Value& target,
		ValueBinding::Transform transform = {});

	/**
	 * @brief Factory function to create a TwoWayValueBinding.
	 * @param left Reference to the left juce::Value.
	 * @param right Reference to the right juce::Value.
	 * @param leftToRightTransform Optional function to transform values when syncing left to right.
	 * @param rightToLeftTransform Optional function to transform values when syncing right to left.
	 * @param comparator Optional function to compare values for equality.
	 * @return A TwoWayValueBinding instance.
	 */
	TwoWayValueBinding makeTwoWayValueBinding(juce::Value& left,
		juce::Value& right,
		TwoWayValueBinding::Transform leftToRightTransform = {},
		TwoWayValueBinding::Transform rightToLeftTransform = {},
		TwoWayValueBinding::Comparator comparator = {});

	/**
	 * @brief Factory function to create a ValueTreePropertyBinding (one-way).
	 * @param sourceTree The juce::ValueTree to monitor.
	 * @param sourceProperty The property identifier to watch for changes.
	 * @param targetValue Reference to the target juce::Value.
	 * @param transform Optional function to transform the value during synchronization.
	 * @return A ValueTreePropertyBinding instance.
	 */
	ValueTreePropertyBinding makeValueTreePropertyBinding(juce::ValueTree sourceTree,
		juce::Identifier sourceProperty,
		juce::Value& targetValue,
		ValueTreePropertyBinding::Transform transform = {});

	/**
	 * @brief Factory function to create a TwoWayValueTreePropertyBinding (bidirectional, overload 1).
	 * @param sourceTree The juce::ValueTree to monitor and modify.
	 * @param sourceProperty The property identifier to watch for changes.
	 * @param targetValue Reference to the target juce::Value.
	 * @param treeToValueTransform Function to transform values when syncing from tree to value.
	 * @param valueToTreeTransform Function to transform values when syncing from value to tree.
	 * @param comparator Optional function to compare values for equality.
	 * @return A TwoWayValueTreePropertyBinding instance.
	 */
TwoWayValueTreePropertyBinding makeValueTreePropertyBinding(
		juce::ValueTree sourceTree,
		juce::Identifier sourceProperty,
		juce::Value& targetValue,
		TwoWayValueTreePropertyBinding::Transform treeToValueTransform,
		TwoWayValueTreePropertyBinding::Transform valueToTreeTransform,
		TwoWayValueTreePropertyBinding::Comparator comparator = {});

	/**
	 * @brief Factory function to create a TwoWayValueTreePropertyBinding (bidirectional, overload 2).
	 * @param sourceTree The juce::ValueTree to monitor and modify.
	 * @param sourceProperty The property identifier to watch for changes.
	 * @param targetValue Reference to the target juce::Value.
	 * @param treeToValueTransform Function to transform values when syncing from tree to value.
	 * @param valueToTreeTransform Function to transform values when syncing from value to tree.
	 * @param comparator Optional function to compare values for equality.
	 * @return A TwoWayValueTreePropertyBinding instance.
	 */
	TwoWayValueTreePropertyBinding makeTwoWayValueTreePropertyBinding(
		juce::ValueTree sourceTree,
		juce::Identifier sourceProperty,
		juce::Value& targetValue,
		TwoWayValueTreePropertyBinding::Transform treeToValueTransform,
		TwoWayValueTreePropertyBinding::Transform valueToTreeTransform,
		TwoWayValueTreePropertyBinding::Comparator comparator = {});

	/**
	 * @brief Convenience factory function to create a one-way ValueTree to Value binding.
	 * @param sourceTree The juce::ValueTree to monitor.
	 * @param sourceProperty The property identifier to watch for changes.
	 * @param targetValue Reference to the target juce::Value.
	 * @param transform Optional function to transform the value during synchronization.
	 * @return A ValueTreePropertyBinding instance.
	 */
	ValueTreePropertyBinding makeValueTreeToValueBinding(juce::ValueTree sourceTree,
		juce::Identifier sourceProperty,
		juce::Value& targetValue,
		ValueTreePropertyBinding::Transform transform = {});
}