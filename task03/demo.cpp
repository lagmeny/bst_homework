#include "tree.h"

#include <algorithm>
#include <iostream>
#include <random>
#include <numeric>
#include <vector>

void print(const PersistentTree& tree) {
    tree.inorder([](int value) { std::cout << value << ' '; });
    std::cout << '\n';
}

int main() {
    PersistentTree empty;
    std::cout << "Empty: " << empty.size() << ' ' << empty.height() << '\n';
    const PersistentTree single = empty.insert(8);
    print(single);
    std::cout << "Single erased twice: " << single.erase(8).erase(8).size() << '\n';
    PersistentTree tree;
    for (int value : {8, 3, 10, 1, 6, 14, 4, 7, 13}) {
        tree = tree.insert(value);
    }
    print(tree.erase(1));
    print(tree.erase(14));
    print(tree.erase(8));
    std::cout << "Original: ";
    print(tree);
    std::cout << "Range [4, 10]: ";
    print(tree.range(4, 10));
    std::cout << "Transform: ";
    print(tree.transform([](int value) { return -value; }));
    std::cout << "Filter: ";
    print(tree.filter([](int value) { return value % 2 == 0; }));
    std::cout << "Sum: "
              << tree.fold(0, [](long long sum, int value) { return sum + value; }) << '\n';
    std::vector<int> values(1000);
    std::iota(values.begin(), values.end(), 0);
    tree = PersistentTree::from_sorted(values.data(), values.size());
    const std::size_t before = PersistentTree::created_nodes();
    const PersistentTree extra = tree.insert(1000);
    std::cout << "Base height=" << tree.height()
              << ", nodes created by insert(1000)="
              << PersistentTree::created_nodes() - before
              << ", new height=" << extra.height() << '\n';
    const std::size_t live_before = PersistentTree::live_nodes();
    std::vector<PersistentTree> versions;
    versions.push_back(tree);
    for (int i = 0; i < 10; ++i) {
        versions.push_back(versions.back().insert(1000 + i));
    }
    std::cout << "Ten additional versions: additional live nodes="
              << PersistentTree::live_nodes() - live_before
              << ", total unique nodes of these versions="
              << tree.size() + PersistentTree::live_nodes() - live_before
              << ", full copies=";
    std::size_t full_copies = 0;
    for (std::size_t i = 1; i < versions.size(); ++i) {
        full_copies += versions[i].size();
    }
    std::cout << full_copies << '\n';
    const std::size_t hundred_before = PersistentTree::created_nodes();
    std::vector<int> additions(100);
    std::iota(additions.begin(), additions.end(), -100);
    std::mt19937 generator(11);
    std::shuffle(additions.begin(), additions.end(), generator);
    for (int value : additions) {
        tree = tree.insert(value);
    }
    std::cout << "100 shuffled smaller keys: created="
              << PersistentTree::created_nodes() - hundred_before << '\n';
}
