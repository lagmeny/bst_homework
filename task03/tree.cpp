#include "tree.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

std::atomic<std::size_t> PersistentTree::created_{0};
std::atomic<std::size_t> PersistentTree::live_{0};

struct PersistentTree::Node {
    std::atomic<std::size_t> references{1};
    int value;
    Node* left;
    Node* right;
    std::size_t count;
    std::size_t depth;
    Node* garbage = nullptr;

    Node(int item, Node* left_child, Node* right_child) noexcept
        : value(item), left(left_child), right(right_child),
          count(1 + (left == nullptr ? 0 : left->count)
                  + (right == nullptr ? 0 : right->count)),
          depth(1 + std::max(left == nullptr ? 0 : left->depth,
                             right == nullptr ? 0 : right->depth)) {
        retain(left);
        retain(right);
        created_.fetch_add(1, std::memory_order_relaxed);
        live_.fetch_add(1, std::memory_order_relaxed);
    }
};

class PersistentTree::Path {
public:
    explicit Path(std::size_t capacity)
        : nodes_(capacity == 0 ? nullptr : new Node*[capacity]) {}
    Path(const Path&) = delete;
    Path& operator=(const Path&) = delete;

    ~Path() {
        delete[] nodes_;
    }

    void push(Node* node) noexcept {
        nodes_[size_++] = node;
    }

    Node* pop() noexcept {
        return nodes_[--size_];
    }

    Node* operator[](std::size_t index) const noexcept {
        return nodes_[index];
    }

    std::size_t size() const noexcept {
        return size_;
    }

private:
    Node** nodes_;
    std::size_t size_ = 0;
};

void PersistentTree::retain(Node* node) noexcept {
    if (node != nullptr) {
        node->references.fetch_add(1, std::memory_order_relaxed);
    }
}

void PersistentTree::release(Node* node) noexcept {
    if (node == nullptr || node->references.fetch_sub(1, std::memory_order_acq_rel) != 1) {
        return;
    }
    node->garbage = nullptr;
    while (node != nullptr) {
        Node* next = node->garbage;
        for (Node* child : {node->left, node->right}) {
            if (child != nullptr
                && child->references.fetch_sub(1, std::memory_order_acq_rel) == 1) {
                child->garbage = next;
                next = child;
            }
        }
        delete node;
        live_.fetch_sub(1, std::memory_order_relaxed);
        node = next;
    }
}

PersistentTree::PersistentTree(Node* root) noexcept : root_(root) {}

PersistentTree::PersistentTree(const PersistentTree& other) noexcept : root_(other.root_) {
    retain(root_);
}

PersistentTree::PersistentTree(PersistentTree&& other) noexcept
    : root_(std::exchange(other.root_, nullptr)) {}

PersistentTree& PersistentTree::operator=(const PersistentTree& other) noexcept {
    retain(other.root_);
    release(root_);
    root_ = other.root_;
    return *this;
}

PersistentTree& PersistentTree::operator=(PersistentTree&& other) noexcept {
    if (this != &other) {
        release(root_);
        root_ = std::exchange(other.root_, nullptr);
    }
    return *this;
}

PersistentTree::~PersistentTree() {
    release(root_);
}

PersistentTree::Node* PersistentTree::rebuild(const Path& path, int value,
                                               PersistentTree replacement) {
    for (std::size_t i = path.size(); i > 0; --i) {
        Node* parent = path[i - 1];
        PersistentTree next(new Node(parent->value,
                                     value < parent->value ? replacement.root_ : parent->left,
                                     value > parent->value ? replacement.root_ : parent->right));
        replacement = std::move(next);
    }
    return std::exchange(replacement.root_, nullptr);
}

PersistentTree PersistentTree::insert(int value) const {
    Path path(height());
    Node* node = root_;
    while (node != nullptr) {
        if (value == node->value) {
            return *this;
        }
        path.push(node);
        node = value < node->value ? node->left : node->right;
    }
    PersistentTree leaf(new Node(value, nullptr, nullptr));
    return PersistentTree(rebuild(path, value, std::move(leaf)));
}

PersistentTree PersistentTree::erase(int value) const {
    Path path(height());
    Node* node = root_;
    while (node != nullptr && node->value != value) {
        path.push(node);
        node = value < node->value ? node->left : node->right;
    }
    if (node == nullptr) {
        return *this;
    }
    PersistentTree replacement;
    if (node->left == nullptr || node->right == nullptr) {
        replacement.root_ = node->left == nullptr ? node->right : node->left;
        retain(replacement.root_);
    } else {
        Path right_path(node->right->depth);
        Node* next = node->right;
        while (next->left != nullptr) {
            right_path.push(next);
            next = next->left;
        }
        retain(next->right);
        PersistentTree child(next->right);
        PersistentTree right(rebuild(right_path, next->value, std::move(child)));
        replacement = PersistentTree(new Node(next->value, node->left, right.root_));
    }
    return PersistentTree(rebuild(path, value, std::move(replacement)));
}

bool PersistentTree::contains(int value) const noexcept {
    Node* node = root_;
    while (node != nullptr) {
        if (value == node->value) {
            return true;
        }
        node = value < node->value ? node->left : node->right;
    }
    return false;
}

void PersistentTree::inorder(const std::function<void(int)>& visitor) const {
    if (!visitor) {
        throw std::invalid_argument("Empty traversal callback");
    }
    Path path(height());
    Node* node = root_;
    while (node != nullptr || path.size() != 0) {
        while (node != nullptr) {
            path.push(node);
            node = node->left;
        }
        node = path.pop();
        visitor(node->value);
        node = node->right;
    }
}

PersistentTree PersistentTree::range(int lower, int upper) const {
    if (lower > upper || root_ == nullptr) {
        return {};
    }
    Path path(height());
    int* values = new int[size()];
    std::size_t count = 0;
    try {
        Node* node = root_;
        while (node != nullptr || path.size() != 0) {
            while (node != nullptr) {
                if (node->value < lower) {
                    node = node->right;
                } else {
                    path.push(node);
                    node = node->left;
                }
            }
            if (path.size() == 0) {
                break;
            }
            node = path.pop();
            if (node->value > upper) {
                break;
            }
            values[count++] = node->value;
            node = node->right;
        }
        PersistentTree result = from_sorted(values, count);
        delete[] values;
        return result;
    } catch (...) {
        delete[] values;
        throw;
    }
}

PersistentTree PersistentTree::transform(const std::function<int(int)>& function) const {
    if (!function) {
        throw std::invalid_argument("Empty transform callback");
    }
    PersistentTree result;
    inorder([&](int value) { result = result.insert(function(value)); });
    return result;
}

PersistentTree PersistentTree::filter(const std::function<bool(int)>& predicate) const {
    if (!predicate) {
        throw std::invalid_argument("Empty filter callback");
    }
    PersistentTree result;
    inorder([&](int value) {
        if (predicate(value)) {
            result = result.insert(value);
        }
    });
    return result;
}

long long PersistentTree::fold(long long initial,
                                const std::function<long long(long long, int)>& function) const {
    if (!function) {
        throw std::invalid_argument("Empty fold callback");
    }
    inorder([&](int value) { initial = function(initial, value); });
    return initial;
}

std::size_t PersistentTree::size() const noexcept {
    return root_ == nullptr ? 0 : root_->count;
}

std::size_t PersistentTree::height() const noexcept {
    return root_ == nullptr ? 0 : root_->depth;
}

bool PersistentTree::empty() const noexcept {
    return root_ == nullptr;
}

std::size_t PersistentTree::created_nodes() noexcept {
    return created_.load(std::memory_order_relaxed);
}

std::size_t PersistentTree::live_nodes() noexcept {
    return live_.load(std::memory_order_relaxed);
}

PersistentTree::Node* PersistentTree::build(const int* values,
                                             std::size_t first, std::size_t last) {
    if (first == last) {
        return nullptr;
    }
    const std::size_t middle = first + (last - first) / 2;
    PersistentTree left(build(values, first, middle));
    PersistentTree right(build(values, middle + 1, last));
    return new Node(values[middle], left.root_, right.root_);
}

PersistentTree PersistentTree::from_sorted(const int* values, std::size_t count) {
    if (values == nullptr && count != 0) {
        throw std::invalid_argument("Null sorted array");
    }
    for (std::size_t i = 1; i < count; ++i) {
        if (values[i - 1] >= values[i]) {
            throw std::invalid_argument("Array must be strictly increasing");
        }
    }
    return PersistentTree(build(values, 0, count));
}
