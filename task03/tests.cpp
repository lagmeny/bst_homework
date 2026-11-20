#include "tree.h"

#include <algorithm>
#include <cassert>
#include <climits>
#include <iostream>
#include <numeric>
#include <random>
#include <set>
#include <stdexcept>
#include <utility>
#include <vector>

std::vector<int> collect(const PersistentTree& tree) {
    std::vector<int> result;
    tree.inorder([&](int value) { result.push_back(value); });
    return result;
}

template <class Action>
void expect_invalid_argument(Action action) {
    bool caught = false;
    try {
        action();
    } catch (const std::invalid_argument&) {
        caught = true;
    }
    assert(caught);
}

int main() {
    const std::size_t baseline = PersistentTree::live_nodes();
    {
        PersistentTree tree;
        std::set<int> reference;
        std::mt19937 generator(11);
        std::uniform_int_distribution<int> distribution(-500, 500);
        std::vector<PersistentTree> saved;
        std::vector<std::vector<int>> expected;
        for (int step = 0; step < 20000; ++step) {
            const int value = distribution(generator);
            const auto old = collect(tree);
            const PersistentTree original = tree;
            if ((generator() & 1u) != 0) {
                tree = tree.insert(value);
                reference.insert(value);
            } else {
                tree = tree.erase(value);
                reference.erase(value);
            }
            assert(collect(original) == old);
            assert(tree.size() == reference.size());
            assert(tree.contains(value) == (reference.count(value) != 0));
            if (step % 149 == 0) {
                assert(collect(tree) == std::vector<int>(reference.begin(), reference.end()));
                saved.push_back(tree);
                expected.push_back(collect(tree));
            }
        }
        for (std::size_t i = 0; i < saved.size(); ++i) {
            assert(collect(saved[i]) == expected[i]);
        }
        tree = PersistentTree();
        tree = tree.insert(INT_MIN).insert(INT_MAX).insert(0);
        assert(collect(tree) == std::vector<int>({INT_MIN, 0, INT_MAX}));
        assert(collect(tree.range(INT_MIN, INT_MAX)) == collect(tree));
        assert(tree.range(1, -1).empty());
        assert(tree.range(0, 0).size() == 1);
        const PersistentTree collapsed = tree.transform([](int) { return 7; });
        assert(collapsed.size() == 1 && collapsed.contains(7));
        assert(tree.filter([](int) { return false; }).empty());
        assert(tree.fold(0, [](long long sum, int value) { return sum + value; }) == -1);
        const auto before = PersistentTree::created_nodes();
        (void)tree.insert(0);
        (void)tree.erase(17);
        assert(PersistentTree::created_nodes() == before);
        expect_invalid_argument([&] { tree.inorder({}); });
        expect_invalid_argument([&] { (void)tree.transform({}); });
        expect_invalid_argument([&] { (void)tree.filter({}); });
        expect_invalid_argument([&] { (void)tree.fold(0, {}); });
        expect_invalid_argument([] { (void)PersistentTree::from_sorted(nullptr, 1); });
        const int unsorted[] = {3, 1, 2};
        const int duplicates[] = {1, 1};
        expect_invalid_argument([&] { (void)PersistentTree::from_sorted(unsorted, 3); });
        expect_invalid_argument([&] { (void)PersistentTree::from_sorted(duplicates, 2); });
        assert(PersistentTree::from_sorted(nullptr, 0).empty());
        bool caught = false;
        try {
            (void)tree.transform([](int) -> int { throw std::runtime_error("test"); });
        } catch (const std::runtime_error&) {
            caught = true;
        }
        assert(caught && tree.size() == 3);
        std::vector<int> values(1000);
        std::iota(values.begin(), values.end(), 0);
        tree = PersistentTree::from_sorted(values.data(), values.size());
        assert(tree.height() == 10);
        const auto count = PersistentTree::created_nodes();
        PersistentTree next = tree.insert(1000);
        assert(PersistentTree::created_nodes() - count == 10);
        assert(!tree.contains(1000) && next.contains(1000));
        PersistentTree assigned;
        assigned = tree;
        PersistentTree* alias = &assigned;
        assigned = *alias;
        assert(collect(assigned) == collect(tree));
        PersistentTree moved(std::move(assigned));
        assert(assigned.empty() && moved.size() == tree.size());
        alias = &moved;
        moved = std::move(*alias);
        assert(moved.size() == tree.size());
        const PersistentTree base;
        assert(base.insert(8).erase(8).empty());
        assert(base.insert(8).insert(3).erase(8).contains(3));
        const PersistentTree two_children = base.insert(8).insert(3).insert(10).insert(9);
        assert(collect(two_children.erase(8)) == std::vector<int>({3, 9, 10}));
        tree = PersistentTree();
        for (int i = 0; i < 10000; ++i) {
            tree = tree.insert(i);
        }
        assert(tree.height() == 10000);
        PersistentTree copy = tree;
        tree = PersistentTree();
        assert(copy.size() == 10000);
        copy = copy.erase(9999);
        assert(copy.height() == 9999);
    }
    assert(PersistentTree::live_nodes() == baseline);
    std::cout << "Task 03: all tests passed, live nodes restored\n";
}
