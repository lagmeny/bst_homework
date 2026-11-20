#ifndef TASK03_TREE_H
#define TASK03_TREE_H

#include <atomic>
#include <cstddef>
#include <functional>

class PersistentTree {
public:
    PersistentTree() noexcept = default;
    PersistentTree(const PersistentTree& other) noexcept;
    PersistentTree(PersistentTree&& other) noexcept;
    PersistentTree& operator=(const PersistentTree& other) noexcept;
    PersistentTree& operator=(PersistentTree&& other) noexcept;
    ~PersistentTree();

    PersistentTree insert(int value) const;
    PersistentTree erase(int value) const;
    bool contains(int value) const noexcept;
    void inorder(const std::function<void(int)>& visitor) const;
    PersistentTree range(int lower, int upper) const;
    PersistentTree transform(const std::function<int(int)>& function) const;
    PersistentTree filter(const std::function<bool(int)>& predicate) const;
    long long fold(long long initial,
                   const std::function<long long(long long, int)>& function) const;
    std::size_t size() const noexcept;
    std::size_t height() const noexcept;
    bool empty() const noexcept;
    static std::size_t created_nodes() noexcept;
    static std::size_t live_nodes() noexcept;
    static PersistentTree from_sorted(const int* values, std::size_t count);

private:
    struct Node;
    class Path;
    explicit PersistentTree(Node* root) noexcept;
    static void retain(Node* node) noexcept;
    static void release(Node* node) noexcept;
    static Node* rebuild(const Path& path, int value, PersistentTree replacement);
    static Node* build(const int* values, std::size_t first, std::size_t last);
    static std::atomic<std::size_t> created_;
    static std::atomic<std::size_t> live_;
    Node* root_ = nullptr;
};

#endif
