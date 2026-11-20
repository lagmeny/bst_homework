#include "tree.h"

#include <stdlib.h>

struct BstNode {
    int value;
    struct BstNode *left;
    struct BstNode *right;
    struct BstNode *parent;
};

static BstNode *leftmost(BstNode *node) {
    while (node != NULL && node->left != NULL) {
        node = node->left;
    }
    return node;
}

static BstNode *successor(BstNode *node) {
    BstNode *parent;
    if (node->right != NULL) {
        return leftmost(node->right);
    }
    parent = node->parent;
    while (parent != NULL && node == parent->right) {
        node = parent;
        parent = parent->parent;
    }
    return parent;
}

static BstStatus erase_at(BstNode **link, int value) {
    BstNode *node;
    BstNode *child;
    while (*link != NULL && (*link)->value != value) {
        link = value < (*link)->value ? &(*link)->left : &(*link)->right;
    }
    if (*link == NULL) {
        return BST_NOT_FOUND;
    }
    node = *link;
    if (node->left != NULL && node->right != NULL) {
        node->value = leftmost(node->right)->value;
        return erase_at(&node->right, node->value);
    }
    child = node->left != NULL ? node->left : node->right;
    if (child != NULL) {
        child->parent = node->parent;
    }
    *link = child;
    free(node);
    return BST_OK;
}

BstStatus bst_init(Bst *tree) {
    if (tree == NULL) {
        return BST_INVALID_ARGUMENT;
    }
    tree->root = NULL;
    tree->size = 0;
    return BST_OK;
}

void bst_destroy(Bst *tree) {
    BstNode *node;
    if (tree == NULL) {
        return;
    }
    node = tree->root;
    while (node != NULL) {
        if (node->left != NULL) {
            BstNode *left = node->left;
            node->left = left->right;
            left->right = node;
            node = left;
        } else {
            BstNode *next = node->right;
            free(node);
            node = next;
        }
    }
    tree->root = NULL;
    tree->size = 0;
}

BstStatus bst_insert(Bst *tree, int value, int *already_present) {
    BstNode **link;
    BstNode *parent = NULL;
    BstNode *node;
    if (tree == NULL || already_present == NULL) {
        return BST_INVALID_ARGUMENT;
    }
    *already_present = 0;
    link = &tree->root;
    while (*link != NULL) {
        parent = *link;
        if (value == parent->value) {
            *already_present = 1;
            return BST_OK;
        }
        link = value < parent->value ? &parent->left : &parent->right;
    }
    node = malloc(sizeof(*node));
    if (node == NULL) {
        return BST_NO_MEMORY;
    }
    node->value = value;
    node->left = NULL;
    node->right = NULL;
    node->parent = parent;
    *link = node;
    ++tree->size;
    return BST_OK;
}

int bst_contains(const Bst *tree, int value) {
    BstNode *node = tree != NULL ? tree->root : NULL;
    while (node != NULL) {
        if (value == node->value) {
            return 1;
        }
        node = value < node->value ? node->left : node->right;
    }
    return 0;
}

BstStatus bst_erase(Bst *tree, int value) {
    BstStatus status;
    if (tree == NULL) {
        return BST_INVALID_ARGUMENT;
    }
    status = erase_at(&tree->root, value);
    if (status == BST_OK) {
        --tree->size;
    }
    return status;
}

void bst_inorder(const Bst *tree, BstVisitor visitor, void *context) {
    BstNode *node;
    if (tree == NULL || visitor == NULL) {
        return;
    }
    node = leftmost(tree->root);
    while (node != NULL) {
        visitor(node->value, context);
        node = successor(node);
    }
}

BstStatus bst_min(const Bst *tree, int *result) {
    if (tree == NULL || result == NULL) {
        return BST_INVALID_ARGUMENT;
    }
    if (tree->root == NULL) {
        return BST_NOT_FOUND;
    }
    *result = leftmost(tree->root)->value;
    return BST_OK;
}

BstStatus bst_max(const Bst *tree, int *result) {
    BstNode *node;
    if (tree == NULL || result == NULL) {
        return BST_INVALID_ARGUMENT;
    }
    node = tree->root;
    if (node == NULL) {
        return BST_NOT_FOUND;
    }
    while (node->right != NULL) {
        node = node->right;
    }
    *result = node->value;
    return BST_OK;
}

size_t bst_height(const Bst *tree) {
    const BstNode *node = tree != NULL ? tree->root : NULL;
    const BstNode *previous = NULL;
    size_t depth = 0;
    size_t height = 0;
    while (node != NULL) {
        const BstNode *next;
        if (previous == node->parent) {
            ++depth;
            if (depth > height) {
                height = depth;
            }
            if (node->left != NULL) {
                next = node->left;
            } else if (node->right != NULL) {
                next = node->right;
            } else {
                next = node->parent;
                --depth;
            }
        } else if (previous == node->left && node->right != NULL) {
            next = node->right;
        } else {
            next = node->parent;
            --depth;
        }
        previous = node;
        node = next;
    }
    return height;
}

size_t bst_size(const Bst *tree) {
    return tree != NULL ? tree->size : 0;
}
