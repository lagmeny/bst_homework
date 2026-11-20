#include "tree.h"

#include <algorithm>
#include <cassert>
#include <climits>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <random>
#include <set>
#include <stdexcept>
#include <vector>

template <class Action>
void expect_out_of_range(Action action) {
    bool caught = false;
    try {
        action();
    } catch (const std::out_of_range&) {
        caught = true;
    }
    assert(caught);
}

void verify(const SearchTree& tree, const std::set<int>& expected) {
    assert(tree.size() == expected.size());
    assert(tree.empty() == expected.empty());
    assert(std::equal(tree.begin(), tree.end(), expected.begin(), expected.end()));
    auto it = tree.end();
    for (auto reference = expected.rbegin(); reference != expected.rend(); ++reference) {
        assert(*--it == *reference);
    }
    assert(it == tree.begin());
    SearchTree copy(tree);
    assert(copy.height() == tree.height());
    assert(std::equal(copy.begin(), copy.end(), tree.begin(), tree.end()));
    copy.insert(1000000);
    assert(!tree.contains(1000000));
}

int main() {
    SearchTree tree;
    expect_out_of_range([&] { (void)tree.nearest(0); });
    expect_out_of_range([&] { (void)*tree.end(); });
    expect_out_of_range([&] { ++tree.end(); });
    expect_out_of_range([&] { --tree.end(); });
    SearchTree::ConstIterator detached;
    expect_out_of_range([&] { --detached; });
    bool invalid_callback = false;
    try {
        tree.range(0, 1, {});
    } catch (const std::invalid_argument&) {
        invalid_callback = true;
    }
    assert(invalid_callback);
    for (int value : {0, INT_MIN, INT_MAX}) {
        assert(tree.insert(value));
    }
    assert(tree.nearest(INT_MIN) == INT_MIN);
    assert(tree.nearest(INT_MAX) == INT_MAX);
    assert(tree.nearest(-1) == 0);
    tree = SearchTree();
    tree.insert(-1);
    tree.insert(1);
    assert(tree.nearest(0) == -1);
    tree = SearchTree();
    std::set<int> expected;
    std::mt19937 generator(11);
    std::uniform_int_distribution<int> values(-1000, 1000);
    for (int step = 0; step < 50000; ++step) {
        const int value = values(generator);
        if ((generator() & 1u) != 0) {
            assert(tree.insert(value) == expected.insert(value).second);
        } else {
            assert(tree.erase(value) == (expected.erase(value) != 0));
        }
        assert(tree.contains(value) == (expected.count(value) != 0));
        if (step % 251 == 0) {
            verify(tree, expected);
            const int lower = values(generator);
            const int upper = values(generator);
            std::vector<int> actual;
            std::vector<int> reference;
            tree.range(lower, upper, [&](int item) { actual.push_back(item); });
            if (lower <= upper) {
                for (auto it = expected.lower_bound(lower);
                     it != expected.end() && *it <= upper; ++it) {
                    reference.push_back(*it);
                }
            }
            assert(actual == reference);
            if (!expected.empty()) {
                const int query = values(generator);
                int best = *expected.begin();
                auto distance = [query](int item) {
                    const std::int64_t delta = static_cast<std::int64_t>(item) - query;
                    return delta < 0 ? -delta : delta;
                };
                for (int item : expected) {
                    if (distance(item) < distance(best)) {
                        best = item;
                    }
                }
                assert(tree.nearest(query) == best);
            }
        }
    }
    verify(tree, expected);
    SearchTree assigned;
    assigned = tree;
    SearchTree* self = &assigned;
    assigned = *self;
    verify(assigned, expected);
    SearchTree stable;
    for (int value : {8, 3, 10, 1, 6, 14, 4, 7, 13}) {
        stable.insert(value);
    }
    auto saved = std::find(stable.begin(), stable.end(), 10);
    stable.erase(8);
    assert(*saved == 10);
    expect_out_of_range([&] { --stable.begin(); });
    SearchTree chain;
    for (int value = 0; value < 10000; ++value) {
        chain.insert(value);
    }
    assert(chain.height() == 10000);
    SearchTree chain_copy(chain);
    assert(chain_copy.height() == 10000);
    assert(std::distance(chain_copy.begin(), chain_copy.end()) == 10000);
    std::cout << "Task 02: all tests passed\n";
}
