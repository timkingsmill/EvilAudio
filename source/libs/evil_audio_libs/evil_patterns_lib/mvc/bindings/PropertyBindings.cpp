/**
 * @file PropertyBindings.cpp
 * @brief Translation unit for property binding helpers.
 */

#include "PropertyBindings.h"

namespace evil
{
	namespace detail
	{
		ScopedUpdateFlag::ScopedUpdateFlag(bool& flag) noexcept
			: flag(flag)
		{
			this->flag = true;
		}

		ScopedUpdateFlag::~ScopedUpdateFlag() noexcept
		{
			flag = false;
		}
	}

	ValueBinding::ValueBinding(juce::Value& source,
		juce::Value& target,
		Transform transform)
		: source(source),
		target(target),
		transform(std::move(transform))
	{
		source.addListener(this);
		sync();
	}

	ValueBinding::~ValueBinding()
	{
		source.removeListener(this);
	}

	void ValueBinding::sync()
	{
		auto value = source.getValue();
		if (transform)
		{
			value = transform(std::move(value));
		}

		target = value;
	}

	void ValueBinding::valueChanged(juce::Value& changedValue)
	{
		if (changedValue.refersToSameSourceAs(source))
		{
			sync();
		}
	}

	TwoWayValueBinding::TwoWayValueBinding(juce::Value& left,
		juce::Value& right,
		Transform leftToRightTransform,
		Transform rightToLeftTransform,
		Comparator comparator)
		: left(left),
		right(right),
		leftToRightTransform(std::move(leftToRightTransform)),
		rightToLeftTransform(std::move(rightToLeftTransform)),
		comparator(std::move(comparator))
	{
		if (!this->comparator)
		{
			this->comparator = [](const juce::var& a, const juce::var& b) { return a == b; };
		}

		left.addListener(this);
		right.addListener(this);
	}

	TwoWayValueBinding::~TwoWayValueBinding()
	{
		left.removeListener(this);
		right.removeListener(this);
	}

	bool TwoWayValueBinding::syncLeftToRight()
	{
		if (isUpdating)
		{
			return false;
		}

		auto value = left.getValue();
		if (leftToRightTransform)
		{
			value = leftToRightTransform(std::move(value));
		}

		const auto current = right.getValue();
		if (comparator(current, value))
		{
			return false;
		}

		detail::ScopedUpdateFlag updateGuard{ isUpdating };
		right = value;
		return true;
	}

	bool TwoWayValueBinding::syncRightToLeft()
	{
		if (isUpdating)
		{
			return false;
		}

		auto value = right.getValue();
		if (rightToLeftTransform)
		{
			value = rightToLeftTransform(std::move(value));
		}

		const auto current = left.getValue();
		if (comparator(current, value))
		{
			return false;
		}

		detail::ScopedUpdateFlag updateGuard{ isUpdating };
		left = value;
		return true;
	}

	void TwoWayValueBinding::valueChanged(juce::Value& changedValue)
	{
		if (isUpdating)
		{
			return;
		}

		if (changedValue.refersToSameSourceAs(left))
		{
			syncLeftToRight();
		}
		else if (changedValue.refersToSameSourceAs(right))
		{
			syncRightToLeft();
		}
	}

	ValueTreePropertyBinding::ValueTreePropertyBinding(juce::ValueTree sourceTree,
		juce::Identifier sourceProperty,
		juce::Value& targetValue,
		Transform transform)
		: sourceTree(std::move(sourceTree)),
		sourceProperty(std::move(sourceProperty)),
		targetValue(targetValue),
		transform(std::move(transform))
	{
		if (this->sourceTree.isValid())
		{
			this->sourceTree.addListener(this);
			sync();
		}
	}

	ValueTreePropertyBinding::~ValueTreePropertyBinding()
	{
		if (sourceTree.isValid())
		{
			sourceTree.removeListener(this);
		}
	}

	void ValueTreePropertyBinding::sync()
	{
		if (!sourceTree.isValid())
		{
			return;
		}

		auto value = sourceTree.getProperty(sourceProperty);
		if (transform)
		{
			value = transform(std::move(value));
		}

		targetValue = value;
	}

	void ValueTreePropertyBinding::valueTreePropertyChanged(juce::ValueTree& tree,
		const juce::Identifier& property)
	{
		if (tree == sourceTree && property == sourceProperty)
		{
			sync();
		}
	}

	TwoWayValueTreePropertyBinding::TwoWayValueTreePropertyBinding(juce::ValueTree sourceTree,
		juce::Identifier sourceProperty,
		juce::Value& targetValue,
		Transform treeToValueTransform,
		Transform valueToTreeTransform,
		Comparator comparator)
		: sourceTree(std::move(sourceTree)),
		sourceProperty(std::move(sourceProperty)),
		targetValue(targetValue),
		treeToValueTransform(std::move(treeToValueTransform)),
		valueToTreeTransform(std::move(valueToTreeTransform)),
		comparator(std::move(comparator))
	{
		if (!this->comparator)
		{
			this->comparator = [](const juce::var& a, const juce::var& b) { return a == b; };
		}

		if (this->sourceTree.isValid())
		{
			this->sourceTree.addListener(this);
		}

		this->targetValue.addListener(this);
		syncTreeToValue();
	}

	TwoWayValueTreePropertyBinding::~TwoWayValueTreePropertyBinding()
	{
		if (sourceTree.isValid())
		{
			sourceTree.removeListener(this);
		}

		targetValue.removeListener(this);
	}

	bool TwoWayValueTreePropertyBinding::syncTreeToValue()
	{
		if (isUpdating || !sourceTree.isValid())
		{
			return false;
		}

		auto value = sourceTree.getProperty(sourceProperty);
		if (treeToValueTransform)
		{
			value = treeToValueTransform(std::move(value));
		}

		const auto current = targetValue.getValue();
		if (comparator(current, value))
		{
			return false;
		}

		detail::ScopedUpdateFlag updateGuard{ isUpdating };
		targetValue = value;
		return true;
	}

	bool TwoWayValueTreePropertyBinding::syncValueToTree()
	{
		if (isUpdating || !sourceTree.isValid())
		{
			return false;
		}

		auto value = targetValue.getValue();
		if (valueToTreeTransform)
		{
			value = valueToTreeTransform(std::move(value));
		}

		const auto current = sourceTree.getProperty(sourceProperty);
		if (comparator(current, value))
		{
			return false;
		}

		detail::ScopedUpdateFlag updateGuard{ isUpdating };
		sourceTree.setProperty(sourceProperty, value, nullptr);
		return true;
	}

	void TwoWayValueTreePropertyBinding::valueTreePropertyChanged(juce::ValueTree& tree,
		const juce::Identifier& property)
	{
		if (tree == sourceTree && property == sourceProperty)
		{
			syncTreeToValue();
		}
	}

	void TwoWayValueTreePropertyBinding::valueChanged(juce::Value& changedValue)
	{
		if (changedValue.refersToSameSourceAs(targetValue))
		{
			syncValueToTree();
		}
	}

	ValueBinding makeValueBinding(juce::Value& source,
		juce::Value& target,
		ValueBinding::Transform transform)
	{
		return ValueBinding{ source, target, std::move(transform) };
	}

	TwoWayValueBinding makeTwoWayValueBinding(juce::Value& left,
		juce::Value& right,
		TwoWayValueBinding::Transform leftToRightTransform,
		TwoWayValueBinding::Transform rightToLeftTransform,
		TwoWayValueBinding::Comparator comparator)
	{
		return TwoWayValueBinding{ left,
			right,
			std::move(leftToRightTransform),
			std::move(rightToLeftTransform),
			std::move(comparator) };
	}

	ValueTreePropertyBinding makeValueTreePropertyBinding(juce::ValueTree sourceTree,
		juce::Identifier sourceProperty,
		juce::Value& targetValue,
		ValueTreePropertyBinding::Transform transform)
	{
		return ValueTreePropertyBinding{ std::move(sourceTree),
			std::move(sourceProperty),
			targetValue,
			std::move(transform) };
	}

	TwoWayValueTreePropertyBinding makeValueTreePropertyBinding(
		juce::ValueTree sourceTree,
		juce::Identifier sourceProperty,
		juce::Value& targetValue,
		TwoWayValueTreePropertyBinding::Transform treeToValueTransform,
		TwoWayValueTreePropertyBinding::Transform valueToTreeTransform,
		TwoWayValueTreePropertyBinding::Comparator comparator)
	{
		return TwoWayValueTreePropertyBinding{ std::move(sourceTree),
			std::move(sourceProperty),
			targetValue,
			std::move(treeToValueTransform),
			std::move(valueToTreeTransform),
			std::move(comparator) };
	}

	TwoWayValueTreePropertyBinding makeTwoWayValueTreePropertyBinding(
		juce::ValueTree sourceTree,
		juce::Identifier sourceProperty,
		juce::Value& targetValue,
		TwoWayValueTreePropertyBinding::Transform treeToValueTransform,
		TwoWayValueTreePropertyBinding::Transform valueToTreeTransform,
		TwoWayValueTreePropertyBinding::Comparator comparator)
	{
		return makeValueTreePropertyBinding(std::move(sourceTree),
			std::move(sourceProperty),
			targetValue,
			std::move(treeToValueTransform),
			std::move(valueToTreeTransform),
			std::move(comparator));
	}

	ValueTreePropertyBinding makeValueTreeToValueBinding(juce::ValueTree sourceTree,
		juce::Identifier sourceProperty,
		juce::Value& targetValue,
		ValueTreePropertyBinding::Transform transform)
	{
		return makeValueTreePropertyBinding(std::move(sourceTree),
			std::move(sourceProperty),
			targetValue,
			std::move(transform));
	}
}
