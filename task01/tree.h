#ifndef TASK01_TREE_H
#define TASK01_TREE_H

#include <stddef.h>

typedef struct BstNode BstNode;

typedef struct {
    BstNode *root;
    size_t size;
} Bst;

typedef enum {
    BST_OK,
    BST_INVALID_ARGUMENT,
    BST_NO_MEMORY,
    BST_NOT_FOUND
} BstStatus;

typedef void (*BstVisitor)(int value, void *context);

BstStatus bst_init(Bst *tree);
void bst_destroy(Bst *tree);
BstStatus bst_insert(Bst *tree, int value, int *already_present);
int bst_contains(const Bst *tree, int value);
BstStatus bst_erase(Bst *tree, int value);
void bst_inorder(const Bst *tree, BstVisitor visitor, void *context);
BstStatus bst_min(const Bst *tree, int *result);
BstStatus bst_max(const Bst *tree, int *result);
size_t bst_height(const Bst *tree);
size_t bst_size(const Bst *tree);

#endif
