#include <gtest/gtest.h>

#include "mvc/bindings/PropertyBindings.h"

/**
 * @file PropertyBindingsTest.cpp
 * @brief Unit tests for the evil::PropertyBindings library.
 *
 * Covers all binding classes and factory functions:
 * - evil::PropertyBinding (one-way, manual sync)
 * - evil::TwoWayPropertyBinding (bidirectional, manual sync)
 * - evil::ValueBinding (one-way, automatic juce::Value listener)
 * - evil::TwoWayValueBinding (bidirectional, automatic juce::Value listeners)
 * - evil::ValueTreePropertyBinding (one-way, automatic juce::ValueTree listener)
 * - evil::TwoWayValueTreePropertyBinding (bidirectional, automatic listeners)
 * - All corresponding factory functions (makePropertyBinding, makeTwoWayPropertyBinding,
 *   makeValueBinding, makeTwoWayValueBinding, makeValueTreePropertyBinding,
 *   makeTwoWayValueTreePropertyBinding, makeValueTreeToValueBinding)
 */

namespace evil::bindings::test
{

// ============================================================================
// Shared test fixtures
// ============================================================================

/**
 * @brief Minimal source object used as a binding source in PropertyBinding tests.
 */
struct SourceObject
{
    int value = 0;
};

/**
 * @brief Minimal target object used as a binding target in PropertyBinding tests.
 */
struct TargetObject
{
    int value = 0;
};

// ============================================================================
// PropertyBinding tests
// ============================================================================

/**
 * @brief Verifies that sync() applies the transform and writes the result to the target.
 *
 * @details Constructs a PropertyBinding with a multiply-by-3 transform. After calling sync(),
 * the target value must equal source.value * 3 (4 * 3 = 12).
 */
TEST(PropertyBindingTest, SyncsWithTransform)
{
    SourceObject source{4};
    TargetObject target{0};

    evil::PropertyBinding<SourceObject, TargetObject, int> binding{
        source,
        target,
        [](const SourceObject& s) { return s.value; },
        [](TargetObject& t, const int& v) { t.value = v; },
        [](int v) { return v * 3; }};

    binding.sync();
    EXPECT_EQ(target.value, 12);
}

/**
 * @brief Verifies that sync() copies the source value to the target without modification
 *        when no transform is supplied.
 *
 * @details Constructs a PropertyBinding without a transform. After calling sync(),
 * the target value must equal the source value unchanged (7).
 */
TEST(PropertyBindingTest, SyncsWithoutTransform)
{
    SourceObject source{7};
    TargetObject target{0};

    evil::PropertyBinding<SourceObject, TargetObject, int> binding{
        source,
        target,
        [](const SourceObject& s) { return s.value; },
        [](TargetObject& t, const int& v) { t.value = v; }};

    binding.sync();
    EXPECT_EQ(target.value, 7);
}

/**
 * @brief Verifies that PropertyBinding does NOT automatically propagate changes to the target.
 *
 * @details Without an explicit sync() call the target must remain at its initialised value,
 * even after the source object is mutated. This confirms the strictly manual-sync contract
 * of PropertyBinding.
 */
TEST(PropertyBindingTest, DoesNotSyncAutomatically)
{
    SourceObject source{1};
    TargetObject target{0};

    evil::PropertyBinding<SourceObject, TargetObject, int> binding{
        source,
        target,
        [](const SourceObject& s) { return s.value; },
        [](TargetObject& t, const int& v) { t.value = v; }};

    // No sync() call — target should remain at its initial value.
    EXPECT_EQ(target.value, 0);

    source.value = 99;
    EXPECT_EQ(target.value, 0);
}

/**
 * @brief Verifies that makePropertyBinding() deduces the value type and produces a
 *        working binding with a transform.
 *
 * @details Uses the factory with a multiply-by-2 transform. After sync() the target
 * must equal source.value * 2 (5 * 2 = 10).
 */
TEST(PropertyBindingTest, MakePropertyBindingFactory)
{
    SourceObject source{5};
    TargetObject target{0};

    auto binding = evil::makePropertyBinding(
        source,
        target,
        [](const SourceObject& s) { return s.value; },
        [](TargetObject& t, const int& v) { t.value = v; },
        [](int v) { return v * 2; });

    binding.sync();
    EXPECT_EQ(target.value, 10);
}

/**
 * @brief Verifies that makePropertyBinding() works correctly when no transform is provided.
 *
 * @details Omitting the optional transform argument must produce a valid binding that
 * copies the source value directly to the target on sync() (source.value = 6).
 */
TEST(PropertyBindingTest, MakePropertyBindingFactoryWithoutTransform)
{
    SourceObject source{6};
    TargetObject target{0};

    auto binding = evil::makePropertyBinding(
        source,
        target,
        [](const SourceObject& s) { return s.value; },
        [](TargetObject& t, const int& v) { t.value = v; });

    binding.sync();
    EXPECT_EQ(target.value, 6);
}

// ============================================================================
// TwoWayPropertyBinding tests
// ============================================================================

/**
 * @brief Verifies that syncLeftToRight() copies the left value to the right object,
 *        returns true on the first sync, and returns false when values are already equal.
 *
 * @details After the first successful sync, getLastSyncedValue() must be populated with
 * the synced value. A subsequent call with no change must return false while keeping
 * getLastSyncedValue() set to the previously synced value.
 */
TEST(TwoWayPropertyBindingTest, SyncLeftToRight)
{
    SourceObject left{3};
    TargetObject right{0};

    auto binding = evil::makeTwoWayPropertyBinding(
        left,
        right,
        [](const SourceObject& s) { return s.value; },
        [](SourceObject& s, const int& v) { s.value = v; },
        [](const TargetObject& t) { return t.value; },
        [](TargetObject& t, const int& v) { t.value = v; });

    EXPECT_TRUE(binding.syncLeftToRight());
    EXPECT_EQ(right.value, 3);
    ASSERT_TRUE(binding.getLastSyncedValue().has_value());
    EXPECT_EQ(*binding.getLastSyncedValue(), 3);

    // Already equal — returns false but lastSyncedValue is still updated.
    EXPECT_FALSE(binding.syncLeftToRight());
    ASSERT_TRUE(binding.getLastSyncedValue().has_value());
    EXPECT_EQ(*binding.getLastSyncedValue(), 3);
}

/**
 * @brief Verifies that syncRightToLeft() applies the right-to-left transform when
 *        propagating the right value to the left object.
 *
 * @details The right-to-left transform adds 2. Starting with right.value = 5,
 * syncRightToLeft() must write 5 + 2 = 7 to left.value.
 */
TEST(TwoWayPropertyBindingTest, SyncRightToLeftWithTransform)
{
    SourceObject left{1};
    TargetObject right{5};

    auto binding = evil::makeTwoWayPropertyBinding(
        left,
        right,
        [](const SourceObject& s) { return s.value; },
        [](SourceObject& s, const int& v) { s.value = v; },
        [](const TargetObject& t) { return t.value; },
        [](TargetObject& t, const int& v) { t.value = v; },
        nullptr,
        [](int v) { return v + 2; });

    EXPECT_TRUE(binding.syncRightToLeft());
    EXPECT_EQ(left.value, 7);
}

/**
 * @brief Verifies that syncLeftToRight() applies the left-to-right transform when
 *        propagating the left value to the right object.
 *
 * @details The left-to-right transform multiplies by 5. Starting with left.value = 4,
 * syncLeftToRight() must write 4 * 5 = 20 to right.value. getLastSyncedValue() must
 * reflect the post-transform value (20).
 */
TEST(TwoWayPropertyBindingTest, SyncLeftToRightWithTransform)
{
    SourceObject left{4};
    TargetObject right{0};

    auto binding = evil::makeTwoWayPropertyBinding(
        left,
        right,
        [](const SourceObject& s) { return s.value; },
        [](SourceObject& s, const int& v) { s.value = v; },
        [](const TargetObject& t) { return t.value; },
        [](TargetObject& t, const int& v) { t.value = v; },
        [](int v) { return v * 5; });

    EXPECT_TRUE(binding.syncLeftToRight());
    EXPECT_EQ(right.value, 20);
    ASSERT_TRUE(binding.getLastSyncedValue().has_value());
    EXPECT_EQ(*binding.getLastSyncedValue(), 20);
}

/**
 * @brief Verifies that syncLeftToRight() and syncRightToLeft() both return false and
 *        leave the target unchanged when source and target values are already equal.
 *
 * @details Both left.value and right.value are initialised to 9. Neither sync direction
 * should write a new value or return true.
 */
TEST(TwoWayPropertyBindingTest, NoUpdateWhenValuesAlreadyEqual)
{
    SourceObject left{9};
    TargetObject right{9};

    auto binding = evil::makeTwoWayPropertyBinding(
        left,
        right,
        [](const SourceObject& s) { return s.value; },
        [](SourceObject& s, const int& v) { s.value = v; },
        [](const TargetObject& t) { return t.value; },
        [](TargetObject& t, const int& v) { t.value = v; });

    EXPECT_FALSE(binding.syncLeftToRight());
    EXPECT_EQ(right.value, 9);

    EXPECT_FALSE(binding.syncRightToLeft());
    EXPECT_EQ(left.value, 9);
}

/**
 * @brief Verifies that getLastSyncedValue() returns an empty optional before any
 *        sync operation has been performed.
 *
 * @details Constructing a TwoWayPropertyBinding with differing values must not
 * implicitly perform a sync. getLastSyncedValue() must therefore have no value.
 */
TEST(TwoWayPropertyBindingTest, LastSyncedValueEmptyBeforeFirstSync)
{
    SourceObject left{1};
    TargetObject right{0};

    auto binding = evil::makeTwoWayPropertyBinding(
        left,
        right,
        [](const SourceObject& s) { return s.value; },
        [](SourceObject& s, const int& v) { s.value = v; },
        [](const TargetObject& t) { return t.value; },
        [](TargetObject& t, const int& v) { t.value = v; });

    EXPECT_FALSE(binding.getLastSyncedValue().has_value());
}

/**
 * @brief Verifies that a custom comparator can suppress updates when the comparator
 *        considers two different values to be logically equal.
 *
 * @details The comparator treats any two positive integers as equal. With left.value = 3
 * and right.value = 5 (both positive), syncLeftToRight() must return false and leave
 * right.value unchanged at 5.
 */
TEST(TwoWayPropertyBindingTest, CustomComparatorPreventsUpdate)
{
    SourceObject left{3};
    TargetObject right{5};

    // Custom comparator that treats all positive values as equal.
    auto binding = evil::makeTwoWayPropertyBinding(
        left,
        right,
        [](const SourceObject& s) { return s.value; },
        [](SourceObject& s, const int& v) { s.value = v; },
        [](const TargetObject& t) { return t.value; },
        [](TargetObject& t, const int& v) { t.value = v; },
        nullptr,
        nullptr,
        [](const int& a, const int& b) { return (a > 0) == (b > 0); });

    // Both are positive, so comparator considers them equal → no update.
    EXPECT_FALSE(binding.syncLeftToRight());
    EXPECT_EQ(right.value, 5);
}

// ============================================================================
// ValueBinding tests
// ============================================================================

/**
 * @brief Verifies that ValueBinding applies the transform and propagates the source
 *        value to the target immediately on construction.
 *
 * @details Constructs a ValueBinding with a multiply-by-2 transform. With source = 10,
 * the target must be 20 without any manual sync() call.
 */
TEST(ValueBindingTest, SyncsFromSourceWithTransformOnConstruction)
{
    juce::Value source;
    juce::Value target;

    source = 10;
    target = 0;

    evil::ValueBinding binding{
        source,
        target,
        [](juce::var value) { return juce::var(static_cast<int>(value) * 2); }};

    EXPECT_EQ(static_cast<int>(target.getValue()), 20);
}

/**
 * @brief Verifies that calling sync() after the source value changes propagates the
 *        updated value (with transform) to the target.
 *
 * @details After changing source to 4, calling sync() must update the target to
 * 4 * 2 = 8, demonstrating that sync() re-reads the current source value.
 */
TEST(ValueBindingTest, ManualSyncPropagatesUpdatedSourceValue)
{
    juce::Value source;
    juce::Value target;

    source = 10;
    target = 0;

    evil::ValueBinding binding{
        source,
        target,
        [](juce::var value) { return juce::var(static_cast<int>(value) * 2); }};

    source = 4;
    binding.sync();
    EXPECT_EQ(static_cast<int>(target.getValue()), 8);
}

/**
 * @brief Verifies that ValueBinding copies the source value unchanged to the target
 *        when no transform is supplied, both on construction and after a manual sync().
 *
 * @details With source = 42, target must equal 42 immediately. After changing source
 * to 7 and calling sync(), target must be 7.
 */
TEST(ValueBindingTest, SyncsWithoutTransform)
{
    juce::Value source;
    juce::Value target;

    source = 42;

    evil::ValueBinding binding{source, target};

    EXPECT_EQ(static_cast<int>(target.getValue()), 42);

    source = 7;
    binding.sync();
    EXPECT_EQ(static_cast<int>(target.getValue()), 7);
}

/**
 * @brief Verifies that makeValueBinding() produces a working ValueBinding with a transform.
 *
 * @details With source = 3 and an add-10 transform, target must be 13 immediately
 * after the binding is created via the factory function.
 */
TEST(ValueBindingTest, MakeValueBindingFactory)
{
    juce::Value source;
    juce::Value target;

    source = 3;

    auto binding = evil::makeValueBinding(
        source,
        target,
        [](juce::var value) { return juce::var(static_cast<int>(value) + 10); });

    EXPECT_EQ(static_cast<int>(target.getValue()), 13);
}

/**
 * @brief Verifies that makeValueBinding() works correctly when no transform is provided.
 *
 * @details With source = 55 and no transform, the factory must produce a binding that
 * copies the source value directly (55) to the target on construction.
 */
TEST(ValueBindingTest, MakeValueBindingFactoryWithoutTransform)
{
    juce::Value source;
    juce::Value target;

    source = 55;

    auto binding = evil::makeValueBinding(source, target);

    EXPECT_EQ(static_cast<int>(target.getValue()), 55);
}

// ============================================================================
// TwoWayValueBinding tests
// ============================================================================

/**
 * @brief Verifies that TwoWayValueBinding applies direction-specific transforms when
 *        syncing in both directions.
 *
 * @details Left-to-right transform multiplies by 10; right-to-left divides by 10.
 * - syncLeftToRight() with left = 1 must write 10 to right.
 * - syncRightToLeft() with right = 70 must write 7 to left.
 */
TEST(TwoWayValueBindingTest, SyncsBothDirectionsWithTransforms)
{
    juce::Value left;
    juce::Value right;

    left = 1;
    right = 20;

    evil::TwoWayValueBinding binding{
        left,
        right,
        [](juce::var value) { return juce::var(static_cast<int>(value) * 10); },
        [](juce::var value) { return juce::var(static_cast<int>(value) / 10); }};

    EXPECT_TRUE(binding.syncLeftToRight());
    EXPECT_EQ(static_cast<int>(right.getValue()), 10);

    right = 70;
    EXPECT_TRUE(binding.syncRightToLeft());
    EXPECT_EQ(static_cast<int>(left.getValue()), 7);
}

/**
 * @brief Verifies that TwoWayValueBinding copies values unchanged in both directions
 *        when no transforms are supplied.
 *
 * @details syncLeftToRight() with left = 42 must write 42 to right.
 * syncRightToLeft() with right = 99 must write 99 to left.
 */
TEST(TwoWayValueBindingTest, SyncsWithoutTransforms)
{
    juce::Value left;
    juce::Value right;

    left = 42;
    right = 0;

    evil::TwoWayValueBinding binding{left, right};

    EXPECT_TRUE(binding.syncLeftToRight());
    EXPECT_EQ(static_cast<int>(right.getValue()), 42);

    left = 0;
    right = 99;
    EXPECT_TRUE(binding.syncRightToLeft());
    EXPECT_EQ(static_cast<int>(left.getValue()), 99);
}

/**
 * @brief Verifies that syncLeftToRight() and syncRightToLeft() both return false and
 *        leave the target unchanged when the two values are already equal.
 *
 * @details Both left and right are initialised to 5. Neither sync direction should
 * write a new value or return true.
 */
TEST(TwoWayValueBindingTest, NoUpdateWhenValuesAlreadyEqual)
{
    juce::Value left;
    juce::Value right;

    left = 5;
    right = 5;

    evil::TwoWayValueBinding binding{left, right};

    EXPECT_FALSE(binding.syncLeftToRight());
    EXPECT_EQ(static_cast<int>(right.getValue()), 5);

    EXPECT_FALSE(binding.syncRightToLeft());
    EXPECT_EQ(static_cast<int>(left.getValue()), 5);
}
/**
 * @brief Verifies that a custom comparator suppresses updates when the comparator
 *        considers the two values logically equal.
 *
 * @details The comparator treats values with the same parity as equal. With left = 3
 * (odd) and right = 7 (odd), syncLeftToRight() must return false and leave right
 * unchanged at 7.
 */TEST(TwoWayValueBindingTest, CustomComparatorPreventsUpdate)
{
    juce::Value left;
    juce::Value right;

    left = 3;
    right = 7;

    // Comparator that considers all odd numbers equal to each other.
    evil::TwoWayValueBinding binding{
        left,
        right,
        {},
        {},
        [](const juce::var& a, const juce::var& b) {
            return (static_cast<int>(a) % 2) == (static_cast<int>(b) % 2);
        }};

    // Both are odd, so comparator says equal → no update.
    EXPECT_FALSE(binding.syncLeftToRight());
    EXPECT_EQ(static_cast<int>(right.getValue()), 7);
}

/**
 * @brief Verifies that makeTwoWayValueBinding() produces a working TwoWayValueBinding
 *        with direction-specific transforms.
 *
 * @details Left-to-right transform adds 1. With left = 8, syncLeftToRight() must write
 * 9 to right and return true.
 */
TEST(TwoWayValueBindingTest, MakeTwoWayValueBindingFactory)
{
    juce::Value left;
    juce::Value right;

    left = 8;
    right = 0;

    auto binding = evil::makeTwoWayValueBinding(
        left,
        right,
        [](juce::var value) { return juce::var(static_cast<int>(value) + 1); },
        [](juce::var value) { return juce::var(static_cast<int>(value) - 1); });

    EXPECT_TRUE(binding.syncLeftToRight());
    EXPECT_EQ(static_cast<int>(right.getValue()), 9);
}

// ============================================================================
// ValueTreePropertyBinding tests
// ============================================================================

/**
 * @brief Verifies that ValueTreePropertyBinding reads the current tree property and
 *        applies the transform to the target immediately on construction.
 *
 * @details With tree property "answer" = 42 and an add-1 transform, the target must
 * be 43 as soon as the binding is constructed.
 */
TEST(ValueTreePropertyBindingTest, SyncsFromTreeOnConstruction)
{
    juce::ValueTree tree{"TestTree"};
    const juce::Identifier key{"answer"};
    juce::Value target;

    tree.setProperty(key, 42, nullptr);

    evil::ValueTreePropertyBinding binding{
        tree,
        key,
        target,
        [](juce::var value) { return juce::var(static_cast<int>(value) + 1); }};

    EXPECT_EQ(static_cast<int>(target.getValue()), 43);
}

/**
 * @brief Verifies that ValueTreePropertyBinding automatically updates the target
 *        whenever the watched tree property changes.
 *
 * @details After constructing the binding, setting the watched property to 9 must
 * trigger the listener, apply the add-1 transform, and write 10 to the target.
 */
TEST(ValueTreePropertyBindingTest, AutoSyncsOnTreePropertyChange)
{
    juce::ValueTree tree{"TestTree"};
    const juce::Identifier key{"answer"};
    juce::Value target;

    tree.setProperty(key, 42, nullptr);

    evil::ValueTreePropertyBinding binding{
        tree,
        key,
        target,
        [](juce::var value) { return juce::var(static_cast<int>(value) + 1); }};

    tree.setProperty(key, 9, nullptr);
    EXPECT_EQ(static_cast<int>(target.getValue()), 10);
}

/**
 * @brief Verifies that ValueTreePropertyBinding copies the tree property unchanged
 *        to the target when no transform is provided, on construction and on change.
 *
 * @details With property = 77, target must equal 77. After setting property to 33,
 * target must equal 33.
 */
TEST(ValueTreePropertyBindingTest, SyncsWithoutTransform)
{
    juce::ValueTree tree{"TestTree"};
    const juce::Identifier key{"val"};
    juce::Value target;

    tree.setProperty(key, 77, nullptr);

    evil::ValueTreePropertyBinding binding{tree, key, target};

    EXPECT_EQ(static_cast<int>(target.getValue()), 77);

    tree.setProperty(key, 33, nullptr);
    EXPECT_EQ(static_cast<int>(target.getValue()), 33);
}

/**
 * @brief Verifies that constructing a ValueTreePropertyBinding with an invalid
 *        (default-constructed) ValueTree does not crash and does not modify the target.
 *
 * @details The target is pre-set to 99. After constructing the binding with an invalid
 * tree the target must remain 99, demonstrating the validity guard in the constructor.
 */
TEST(ValueTreePropertyBindingTest, InvalidTreeDoesNotCrashOrSync)
{
    juce::ValueTree invalidTree{};
    const juce::Identifier key{"prop"};
    juce::Value target;

    target = 99;

    // Should not crash; target should remain unchanged.
    evil::ValueTreePropertyBinding binding{invalidTree, key, target};
    EXPECT_EQ(static_cast<int>(target.getValue()), 99);
}

/**
 * @brief Verifies that ValueTreePropertyBinding only reacts to changes on its watched
 *        property and ignores changes to any other property in the same tree.
 *
 * @details Changing the "other" property (not the watched one) must not affect the
 * target value.
 */
TEST(ValueTreePropertyBindingTest, IgnoresOtherPropertyChanges)
{
    juce::ValueTree tree{"TestTree"};
    const juce::Identifier watched{"watched"};
    const juce::Identifier other{"other"};
    juce::Value target;

    tree.setProperty(watched, 10, nullptr);
    tree.setProperty(other, 0, nullptr);

    evil::ValueTreePropertyBinding binding{tree, watched, target};

    EXPECT_EQ(static_cast<int>(target.getValue()), 10);

    // Changing a different property must not affect target.
    tree.setProperty(other, 999, nullptr);
    EXPECT_EQ(static_cast<int>(target.getValue()), 10);
}

/**
 * @brief Verifies that calling sync() manually re-reads the current tree property
 *        and writes it to the target.
 *
 * @details After construction the target equals the tree property (5). Calling sync()
 * again must be idempotent — target must still be 5.
 */
TEST(ValueTreePropertyBindingTest, ManualSyncReflectsCurrentTreeValue)
{
    juce::ValueTree tree{"TestTree"};
    const juce::Identifier key{"v"};
    juce::Value target;

    tree.setProperty(key, 5, nullptr);

    evil::ValueTreePropertyBinding binding{tree, key, target};
    EXPECT_EQ(static_cast<int>(target.getValue()), 5);

    // Manually invoke sync again — target should still reflect the current tree value.
    binding.sync();
    EXPECT_EQ(static_cast<int>(target.getValue()), 5);
}

/**
 * @brief Verifies that the one-way overload of makeValueTreePropertyBinding() produces
 *        a working ValueTreePropertyBinding with a transform.
 *
 * @details With tree property "x" = 4 and a multiply-by-3 transform, target must
 * equal 12 immediately after the binding is created.
 */
TEST(ValueTreePropertyBindingTest, MakeValueTreePropertyBindingFactory)
{
    juce::ValueTree tree{"Tree"};
    const juce::Identifier key{"x"};
    juce::Value target;

    tree.setProperty(key, 4, nullptr);

    auto binding = evil::makeValueTreePropertyBinding(
        tree,
        key,
        target,
        [](juce::var value) { return juce::var(static_cast<int>(value) * 3); });

    EXPECT_EQ(static_cast<int>(target.getValue()), 12);
}

/**
 * @brief Verifies that makeValueTreeToValueBinding() produces a working one-way
 *        ValueTreePropertyBinding that auto-syncs on tree property changes.
 *
 * @details With property = 6 and an add-4 transform, target must be 10 on construction.
 * After setting the property to 1, the listener must fire and update target to 5.
 */
TEST(ValueTreePropertyBindingTest, MakeValueTreeToValueBindingFactory)
{
    juce::ValueTree tree{"Tree"};
    const juce::Identifier key{"x"};
    juce::Value target;

    tree.setProperty(key, 6, nullptr);

    auto binding = evil::makeValueTreeToValueBinding(
        tree,
        key,
        target,
        [](juce::var value) { return juce::var(static_cast<int>(value) + 4); });

    EXPECT_EQ(static_cast<int>(target.getValue()), 10);

    tree.setProperty(key, 1, nullptr);
    EXPECT_EQ(static_cast<int>(target.getValue()), 5);
}

// ============================================================================
// TwoWayValueTreePropertyBinding tests
// ============================================================================

/**
 * @brief Verifies that TwoWayValueTreePropertyBinding reads the tree property and
 *        applies the tree-to-value transform to the target on construction.
 *
 * @details With tree property "gain" = 3 and a multiply-by-2 tree-to-value transform,
 * the target must equal 6 immediately after construction.
 */
TEST(TwoWayValueTreePropertyBindingTest, SyncsTreeToValueOnConstruction)
{
    juce::ValueTree tree{"Model"};
    const juce::Identifier key{"gain"};
    juce::Value value;

    tree.setProperty(key, 3, nullptr);
    value = 0;

    evil::TwoWayValueTreePropertyBinding binding{
        tree,
        key,
        value,
        [](juce::var treeValue) { return juce::var(static_cast<int>(treeValue) * 2); },
        [](juce::var uiValue) { return juce::var(static_cast<int>(uiValue) / 2); }};

    EXPECT_EQ(static_cast<int>(value.getValue()), 6);
}

/**
 * @brief Verifies that syncValueToTree() applies the value-to-tree transform and
 *        writes the result back to the tree property, returning true on a real update.
 *
 * @details With a divide-by-2 value-to-tree transform and value = 40, syncValueToTree()
 * must write 20 to the tree property and return true.
 */
TEST(TwoWayValueTreePropertyBindingTest, SyncValueToTreeUpdatesTreeProperty)
{
    juce::ValueTree tree{"Model"};
    const juce::Identifier key{"gain"};
    juce::Value value;

    tree.setProperty(key, 3, nullptr);
    value = 0;

    evil::TwoWayValueTreePropertyBinding binding{
        tree,
        key,
        value,
        [](juce::var treeValue) { return juce::var(static_cast<int>(treeValue) * 2); },
        [](juce::var uiValue) { return juce::var(static_cast<int>(uiValue) / 2); }};

    value = 40;
    EXPECT_TRUE(binding.syncValueToTree());
    EXPECT_EQ(static_cast<int>(tree.getProperty(key)), 20);
}

/**
 * @brief Verifies that TwoWayValueTreePropertyBinding automatically updates the target
 *        value when the watched tree property changes, and that a subsequent manual
 *        syncTreeToValue() is a no-op.
 *
 * @details Setting the tree property to 8 must fire the listener and write 16
 * (8 * 2) to the value. A second syncTreeToValue() call must return false because
 * the value already equals the transformed tree property.
 */
TEST(TwoWayValueTreePropertyBindingTest, AutoSyncsTreeToValueOnPropertyChange)
{
    juce::ValueTree tree{"Model"};
    const juce::Identifier key{"gain"};
    juce::Value value;

    tree.setProperty(key, 3, nullptr);
    value = 0;

    evil::TwoWayValueTreePropertyBinding binding{
        tree,
        key,
        value,
        [](juce::var treeValue) { return juce::var(static_cast<int>(treeValue) * 2); },
        [](juce::var uiValue) { return juce::var(static_cast<int>(uiValue) / 2); }};

    tree.setProperty(key, 8, nullptr);
    EXPECT_EQ(static_cast<int>(value.getValue()), 16);

    // Calling syncTreeToValue() again is a no-op because value already matches.
    EXPECT_FALSE(binding.syncTreeToValue());
}

/**
 * @brief Verifies that syncTreeToValue() and syncValueToTree() both return false when
 *        the tree property and the target value already hold equal content.
 *
 * @details Identity transforms are used. Tree property = 10, value = 10. After
 * construction (no net change), both manual sync directions must be no-ops.
 */
TEST(TwoWayValueTreePropertyBindingTest, NoUpdateWhenTreeAndValueAlreadyEqual)
{
    juce::ValueTree tree{"Model"};
    const juce::Identifier key{"v"};
    juce::Value value;

    tree.setProperty(key, 10, nullptr);
    value = 10;

    // Identity transforms — no conversion.
    evil::TwoWayValueTreePropertyBinding binding{
        tree,
        key,
        value,
        [](juce::var v) { return v; },
        [](juce::var v) { return v; }};

    // Constructor syncs tree (10) to value — but value is already 10, so no change.
    EXPECT_EQ(static_cast<int>(value.getValue()), 10);

    EXPECT_FALSE(binding.syncTreeToValue());
    EXPECT_FALSE(binding.syncValueToTree());
}

/**
 * @brief Verifies that constructing a TwoWayValueTreePropertyBinding with an invalid
 *        (default-constructed) ValueTree does not crash and that both manual sync
 *        methods return false.
 *
 * @details The value is pre-set to 55. Construction must leave it unchanged.
 * syncTreeToValue() and syncValueToTree() must both return false because the
 * tree is invalid.
 */
TEST(TwoWayValueTreePropertyBindingTest, InvalidTreeDoesNotCrashOrSync)
{
    juce::ValueTree invalidTree{};
    const juce::Identifier key{"prop"};
    juce::Value value;

    value = 55;

    // Should not crash; value should remain unchanged.
    evil::TwoWayValueTreePropertyBinding binding{
        invalidTree,
        key,
        value,
        [](juce::var v) { return v; },
        [](juce::var v) { return v; }};

    EXPECT_EQ(static_cast<int>(value.getValue()), 55);

    EXPECT_FALSE(binding.syncTreeToValue());
    EXPECT_FALSE(binding.syncValueToTree());
}

/**
 * @brief Verifies that TwoWayValueTreePropertyBinding only reacts to changes on its
 *        watched property and ignores changes to any other property in the same tree.
 *
 * @details Changing the "other" property (not the watched one) must not affect the
 * target value.
 */
TEST(TwoWayValueTreePropertyBindingTest, IgnoresOtherPropertyChanges)
{
    juce::ValueTree tree{"Model"};
    const juce::Identifier watched{"watched"};
    const juce::Identifier other{"other"};
    juce::Value value;

    tree.setProperty(watched, 10, nullptr);
    tree.setProperty(other, 0, nullptr);

    evil::TwoWayValueTreePropertyBinding binding{
        tree,
        watched,
        value,
        [](juce::var v) { return v; },
        [](juce::var v) { return v; }};

    EXPECT_EQ(static_cast<int>(value.getValue()), 10);

    // Changing a different property must not affect value.
    tree.setProperty(other, 999, nullptr);
    EXPECT_EQ(static_cast<int>(value.getValue()), 10);
}

/**
 * @brief Verifies that makeTwoWayValueTreePropertyBinding() produces a working
 *        TwoWayValueTreePropertyBinding with direction-specific transforms.
 *
 * @details With property = 5 and an add-1 tree-to-value transform, value must be 6
 * on construction. Setting value = 20 and calling syncValueToTree() with a subtract-1
 * value-to-tree transform must write 19 to the tree property.
 */
TEST(TwoWayValueTreePropertyBindingTest, MakeTwoWayValueTreePropertyBindingFactory)
{
    juce::ValueTree tree{"Tree"};
    const juce::Identifier key{"k"};
    juce::Value value;

    tree.setProperty(key, 5, nullptr);

    auto binding = evil::makeTwoWayValueTreePropertyBinding(
        tree,
        key,
        value,
        [](juce::var v) { return juce::var(static_cast<int>(v) + 1); },
        [](juce::var v) { return juce::var(static_cast<int>(v) - 1); });

    EXPECT_EQ(static_cast<int>(value.getValue()), 6);

    value = 20;
    EXPECT_TRUE(binding.syncValueToTree());
    EXPECT_EQ(static_cast<int>(tree.getProperty(key)), 19);
}

/**
 * @brief Verifies that the two-way overload of makeValueTreePropertyBinding() (which
 *        accepts both treeToValue and valueToTree transforms) produces a fully
 *        functional TwoWayValueTreePropertyBinding.
 *
 * @details With property = 2 and a multiply-by-10 tree-to-value transform, value must
 * be 20 on construction. Setting value = 50 and calling syncValueToTree() with a
 * divide-by-10 value-to-tree transform must write 5 to the tree property.
 */
TEST(TwoWayValueTreePropertyBindingTest, MakeValueTreePropertyBindingTwoWayOverload)
{
    juce::ValueTree tree{"Tree"};
    const juce::Identifier key{"k"};
    juce::Value value;

    tree.setProperty(key, 2, nullptr);

    // Two-way overload: both treeToValue and valueToTree transforms provided.
    auto binding = evil::makeValueTreePropertyBinding(
        tree,
        key,
        value,
        [](juce::var v) { return juce::var(static_cast<int>(v) * 10); },
        [](juce::var v) { return juce::var(static_cast<int>(v) / 10); });

    EXPECT_EQ(static_cast<int>(value.getValue()), 20);

    value = 50;
    EXPECT_TRUE(binding.syncValueToTree());
    EXPECT_EQ(static_cast<int>(tree.getProperty(key)), 5);
}

} // namespace evil::bindings::test
