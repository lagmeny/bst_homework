#include "tree.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

static void print_value(int value, void *context) {
    (void)context;
    printf("%d ", value);
}

static void insert(Bst *tree, int value) {
    int duplicate;
    BstStatus status = bst_insert(tree, value, &duplicate);
    if (status != BST_OK) {
        fprintf(stderr, "Insertion failed: %d\n", status);
        bst_destroy(tree);
        exit(EXIT_FAILURE);
    }
}

int main(void) {
    Bst tree;
    int result;
    int duplicate;
    int i;
    unsigned state = 11;
    const int values[] = {8, 3, 10, 1, 6, 14, 4, 7, 13};
    if (bst_init(&tree) != BST_OK) {
        return EXIT_FAILURE;
    }
    printf("Empty: size=%zu, height=%zu, min status=%d\n",
           bst_size(&tree), bst_height(&tree), bst_min(&tree, &result));
    insert(&tree, INT_MIN);
    printf("Single: found=%d, height=%zu\n",
           bst_contains(&tree, INT_MIN), bst_height(&tree));
    printf("Erase single: %d\n", bst_erase(&tree, INT_MIN));
    printf("Erase again: %d\n", bst_erase(&tree, INT_MIN));
    for (i = 0; i < 9; ++i) {
        insert(&tree, values[i]);
    }
    (void)bst_insert(&tree, 8, &duplicate);
    printf("Duplicate: %d\n", duplicate);
    printf("Erase leaf 1: %d\n", bst_erase(&tree, 1));
    printf("Erase one-child node 14: %d\n", bst_erase(&tree, 14));
    printf("Erase two-child root 8: %d\n", bst_erase(&tree, 8));
    bst_inorder(&tree, print_value, NULL);
    putchar('\n');
    (void)bst_min(&tree, &result);
    printf("Min: %d\n", result);
    (void)bst_max(&tree, &result);
    printf("Max: %d\n", result);
    bst_destroy(&tree);
    for (i = 0; i < 1000; ++i) {
        insert(&tree, i);
    }
    printf("Increasing 1000: size=%zu, height=%zu\n",
           bst_size(&tree), bst_height(&tree));
    bst_destroy(&tree);
    for (i = 0; i < 1000; ++i) {
        state = state * 1664525u + 1013904223u;
        insert(&tree, (int)(state % 100000u));
    }
    printf("Random 1000 attempts: size=%zu, height=%zu\n",
           bst_size(&tree), bst_height(&tree));
    bst_destroy(&tree);
    bst_destroy(&tree);
    return EXIT_SUCCESS;
}
