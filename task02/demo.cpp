#include "tree.h"

#include <iostream>
#include <stdexcept>

int main() {
    SearchTree tree;
    std::cout << "Empty: " << tree.empty() << '\n';
    try {
        std::cout << tree.nearest(0) << '\n';
    } catch (const std::out_of_range& error) {
        std::cout << error.what() << '\n';
    }
    tree.insert(8);
    std::cout << "Single: " << *tree.begin() << '\n';
    tree.erase(8);
    std::cout << "Repeated erase: " << tree.erase(8) << '\n';
    for (int value : {8, 3, 10, 1, 6, 14, 4, 7, 13}) {
        tree.insert(value);
    }
    std::cout << "Duplicate changed tree: " << tree.insert(8) << '\n';
    SearchTree copy(tree);
    SearchTree assigned;
    assigned = tree;
    tree.erase(8);
    std::cout << "Copy contains 8: " << copy.contains(8) << '\n';
    std::cout << "Copy and assigned heights: " << copy.height() << ' ' << assigned.height() << '\n';
    std::cout << "Forward: ";
    for (int value : tree) {
        std::cout << value << ' ';
    }
    std::cout << "\nBackward: ";
    for (auto it = tree.end(); it != tree.begin();) {
        std::cout << *--it << ' ';
    }
    std::cout << "\nRange [4, 10]: ";
    tree.range(4, 10, [](int value) { std::cout << value << ' '; });
    std::cout << "\nNearest to -100, 5, 100: "
              << tree.nearest(-100) << ' ' << tree.nearest(5) << ' ' << tree.nearest(100) << '\n';
    try {
        std::cout << *tree.end() << '\n';
    } catch (const std::out_of_range& error) {
        std::cout << error.what() << '\n';
    }
}
