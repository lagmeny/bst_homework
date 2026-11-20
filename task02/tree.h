#ifndef TASK02_TREE_H
#define TASK02_TREE_H

#include <cstddef>
#include <functional>
#include <iterator>

class SearchTree {
private:
    struct Node;

public:
    class ConstIterator {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = int;
        using difference_type = std::ptrdiff_t;
        using pointer = const int*;
        using reference = const int&;

        ConstIterator() noexcept = default;
        reference operator*() const;
        pointer operator->() const;
        ConstIterator& operator++();
        ConstIterator operator++(int);
        ConstIterator& operator--();
        ConstIterator operator--(int);
        bool operator==(const ConstIterator& other) const noexcept;
        bool operator!=(const ConstIterator& other) const noexcept;

    private:
        friend class SearchTree;
        ConstIterator(const SearchTree* owner, const Node* node) noexcept;
        const SearchTree* owner_ = nullptr;
        const Node* node_ = nullptr;
    };

    SearchTree() noexcept = default;
    SearchTree(const SearchTree& other);
    SearchTree& operator=(const SearchTree& other);
    ~SearchTree();

    bool insert(int value);
    bool erase(int value) noexcept;
    bool contains(int value) const noexcept;
    void range(int lower, int upper, const std::function<void(int)>& visitor) const;
    int nearest(int value) const;
    std::size_t height() const noexcept;
    std::size_t size() const noexcept;
    bool empty() const noexcept;
    ConstIterator begin() const noexcept;
    ConstIterator end() const noexcept;
    void swap(SearchTree& other) noexcept;

private:
    static Node* minimum(Node* node) noexcept;
    static const Node* minimum(const Node* node) noexcept;
    static const Node* maximum(const Node* node) noexcept;
    static const Node* successor(const Node* node) noexcept;
    static const Node* predecessor(const Node* node) noexcept;
    void replace(Node* node, Node* replacement) noexcept;
    void clear() noexcept;

    Node* root_ = nullptr;
    std::size_t size_ = 0;
};

#endif
