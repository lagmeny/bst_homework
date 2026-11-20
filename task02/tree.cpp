#include "tree.h"

#include <cstdint>
#include <stdexcept>
#include <utility>

struct SearchTree::Node {
    int value;
    Node* parent;
    Node* left = nullptr;
    Node* right = nullptr;
};

SearchTree::Node* SearchTree::minimum(Node* node) noexcept {
    while (node != nullptr && node->left != nullptr) {
        node = node->left;
    }
    return node;
}

const SearchTree::Node* SearchTree::minimum(const Node* node) noexcept {
    while (node != nullptr && node->left != nullptr) {
        node = node->left;
    }
    return node;
}

const SearchTree::Node* SearchTree::maximum(const Node* node) noexcept {
    while (node != nullptr && node->right != nullptr) {
        node = node->right;
    }
    return node;
}

const SearchTree::Node* SearchTree::successor(const Node* node) noexcept {
    if (node->right != nullptr) {
        return minimum(node->right);
    }
    const Node* parent = node->parent;
    while (parent != nullptr && node == parent->right) {
        node = parent;
        parent = parent->parent;
    }
    return parent;
}

const SearchTree::Node* SearchTree::predecessor(const Node* node) noexcept {
    if (node->left != nullptr) {
        return maximum(node->left);
    }
    const Node* parent = node->parent;
    while (parent != nullptr && node == parent->left) {
        node = parent;
        parent = parent->parent;
    }
    return parent;
}

SearchTree::ConstIterator::ConstIterator(const SearchTree* owner, const Node* node) noexcept
    : owner_(owner), node_(node) {}

SearchTree::ConstIterator::reference SearchTree::ConstIterator::operator*() const {
    if (node_ == nullptr) {
        throw std::out_of_range("Cannot dereference the end iterator");
    }
    return node_->value;
}

SearchTree::ConstIterator::pointer SearchTree::ConstIterator::operator->() const {
    return &operator*();
}

SearchTree::ConstIterator& SearchTree::ConstIterator::operator++() {
    if (node_ == nullptr) {
        throw std::out_of_range("Cannot increment the end iterator");
    }
    node_ = SearchTree::successor(node_);
    return *this;
}

SearchTree::ConstIterator SearchTree::ConstIterator::operator++(int) {
    ConstIterator old = *this;
    ++*this;
    return old;
}

SearchTree::ConstIterator& SearchTree::ConstIterator::operator--() {
    const Node* previous = node_ == nullptr
        ? (owner_ == nullptr ? nullptr : SearchTree::maximum(owner_->root_))
        : SearchTree::predecessor(node_);
    if (previous == nullptr) {
        throw std::out_of_range("Cannot decrement this iterator");
    }
    node_ = previous;
    return *this;
}

SearchTree::ConstIterator SearchTree::ConstIterator::operator--(int) {
    ConstIterator old = *this;
    --*this;
    return old;
}

bool SearchTree::ConstIterator::operator==(const ConstIterator& other) const noexcept {
    return owner_ == other.owner_ && node_ == other.node_;
}

bool SearchTree::ConstIterator::operator!=(const ConstIterator& other) const noexcept {
    return !(*this == other);
}

SearchTree::SearchTree(const SearchTree& other) {
    if (other.root_ == nullptr) {
        return;
    }
    try {
        root_ = new Node{other.root_->value, nullptr};
        const Node* source = other.root_;
        Node* target = root_;
        while (source != nullptr) {
            if (source->left != nullptr && target->left == nullptr) {
                target->left = new Node{source->left->value, target};
                source = source->left;
                target = target->left;
            } else if (source->right != nullptr && target->right == nullptr) {
                target->right = new Node{source->right->value, target};
                source = source->right;
                target = target->right;
            } else {
                source = source->parent;
                target = target->parent;
            }
        }
        size_ = other.size_;
    } catch (...) {
        clear();
        throw;
    }
}

SearchTree& SearchTree::operator=(const SearchTree& other) {
    if (this != &other) {
        SearchTree copy(other);
        swap(copy);
    }
    return *this;
}

SearchTree::~SearchTree() {
    clear();
}

void SearchTree::clear() noexcept {
    Node* node = root_;
    while (node != nullptr) {
        if (node->left != nullptr) {
            Node* left = node->left;
            node->left = left->right;
            left->right = node;
            node = left;
        } else {
            Node* next = node->right;
            delete node;
            node = next;
        }
    }
    root_ = nullptr;
    size_ = 0;
}

bool SearchTree::insert(int value) {
    Node** link = &root_;
    Node* parent = nullptr;
    while (*link != nullptr) {
        parent = *link;
        if (value == parent->value) {
            return false;
        }
        link = value < parent->value ? &parent->left : &parent->right;
    }
    *link = new Node{value, parent};
    ++size_;
    return true;
}

void SearchTree::replace(Node* node, Node* replacement) noexcept {
    if (node->parent == nullptr) {
        root_ = replacement;
    } else if (node == node->parent->left) {
        node->parent->left = replacement;
    } else {
        node->parent->right = replacement;
    }
    if (replacement != nullptr) {
        replacement->parent = node->parent;
    }
}

bool SearchTree::erase(int value) noexcept {
    Node* node = root_;
    while (node != nullptr && node->value != value) {
        node = value < node->value ? node->left : node->right;
    }
    if (node == nullptr) {
        return false;
    }
    if (node->left == nullptr) {
        replace(node, node->right);
    } else if (node->right == nullptr) {
        replace(node, node->left);
    } else {
        Node* next = minimum(node->right);
        if (next->parent != node) {
            replace(next, next->right);
            next->right = node->right;
            next->right->parent = next;
        }
        replace(node, next);
        next->left = node->left;
        next->left->parent = next;
    }
    delete node;
    --size_;
    return true;
}

bool SearchTree::contains(int value) const noexcept {
    const Node* node = root_;
    while (node != nullptr) {
        if (value == node->value) {
            return true;
        }
        node = value < node->value ? node->left : node->right;
    }
    return false;
}

void SearchTree::range(int lower, int upper, const std::function<void(int)>& visitor) const {
    if (!visitor) {
        throw std::invalid_argument("Empty range callback");
    }
    if (lower > upper) {
        return;
    }
    const Node* node = root_;
    const Node* candidate = nullptr;
    while (node != nullptr) {
        if (node->value < lower) {
            node = node->right;
        } else {
            candidate = node;
            node = node->left;
        }
    }
    while (candidate != nullptr && candidate->value <= upper) {
        visitor(candidate->value);
        candidate = successor(candidate);
    }
}

int SearchTree::nearest(int value) const {
    if (root_ == nullptr) {
        throw std::out_of_range("Nearest value in an empty tree");
    }
    const Node* node = root_;
    int best = node->value;
    auto distance = [value](int candidate) {
        const std::int64_t delta = static_cast<std::int64_t>(candidate) - value;
        return delta < 0 ? -delta : delta;
    };
    while (node != nullptr) {
        if (distance(node->value) < distance(best)
            || (distance(node->value) == distance(best) && node->value < best)) {
            best = node->value;
        }
        if (node->value == value) {
            break;
        }
        node = value < node->value ? node->left : node->right;
    }
    return best;
}

std::size_t SearchTree::height() const noexcept {
    const Node* node = root_;
    const Node* previous = nullptr;
    std::size_t depth = 0;
    std::size_t result = 0;
    while (node != nullptr) {
        const Node* next;
        if (previous == node->parent) {
            ++depth;
            if (depth > result) {
                result = depth;
            }
            if (node->left != nullptr) {
                next = node->left;
            } else if (node->right != nullptr) {
                next = node->right;
            } else {
                next = node->parent;
                --depth;
            }
        } else if (previous == node->left && node->right != nullptr) {
            next = node->right;
        } else {
            next = node->parent;
            --depth;
        }
        previous = node;
        node = next;
    }
    return result;
}

std::size_t SearchTree::size() const noexcept {
    return size_;
}

bool SearchTree::empty() const noexcept {
    return size_ == 0;
}

SearchTree::ConstIterator SearchTree::begin() const noexcept {
    return ConstIterator(this, minimum(root_));
}

SearchTree::ConstIterator SearchTree::end() const noexcept {
    return ConstIterator(this, nullptr);
}

void SearchTree::swap(SearchTree& other) noexcept {
    std::swap(root_, other.root_);
    std::swap(size_, other.size_);
}
