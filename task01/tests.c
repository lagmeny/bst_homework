#include "tree.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    const unsigned char *present;
    size_t count;
    int previous;
    int first;
} Check;

static void check_value(int value, void *context) {
    Check *check = context;
    assert(value >= -256 && value <= 256);
    assert(check->present[value + 256]);
    assert(check->first || check->previous < value);
    check->first = 0;
    check->previous = value;
    ++check->count;
}

static void insert(Bst *tree, int value) {
    int duplicate = 0;
    assert(bst_insert(tree, value, &duplicate) == BST_OK);
    assert(!duplicate);
}

int main(void) {
    Bst tree;
    unsigned char present[513] = {0};
    unsigned state = 11;
    size_t expected = 0;
    int result = 123;
    int duplicate;
    int step;
    assert(bst_init(NULL) == BST_INVALID_ARGUMENT);
    assert(bst_init(&tree) == BST_OK);
    assert(bst_min(&tree, &result) == BST_NOT_FOUND);
    assert(result == 123);
    assert(bst_max(&tree, NULL) == BST_INVALID_ARGUMENT);
    assert(bst_insert(NULL, 1, &duplicate) == BST_INVALID_ARGUMENT);
    assert(bst_insert(&tree, 1, NULL) == BST_INVALID_ARGUMENT);
    assert(bst_erase(NULL, 1) == BST_INVALID_ARGUMENT);
    assert(bst_erase(&tree, 1) == BST_NOT_FOUND);
    assert(bst_height(NULL) == 0 && bst_size(NULL) == 0);
    assert(!bst_contains(NULL, 0));
    bst_inorder(NULL, NULL, NULL);
    bst_inorder(&tree, NULL, NULL);
    bst_destroy(NULL);
    insert(&tree, INT_MIN);
    insert(&tree, INT_MAX);
    insert(&tree, 0);
    assert(bst_min(&tree, &result) == BST_OK && result == INT_MIN);
    assert(bst_max(&tree, &result) == BST_OK && result == INT_MAX);
    assert(bst_erase(&tree, INT_MIN) == BST_OK);
    assert(bst_erase(&tree, INT_MAX) == BST_OK);
    assert(bst_erase(&tree, 0) == BST_OK);
    for (step = 0; step < 50000; ++step) {
        int value;
        state = state * 1664525u + 1013904223u;
        value = (int)((state >> 8) % 513u) - 256;
        if ((state & 1u) != 0) {
            assert(bst_insert(&tree, value, &duplicate) == BST_OK);
            assert(duplicate == (int)present[value + 256]);
            if (!duplicate) {
                present[value + 256] = 1;
                ++expected;
            }
        } else {
            BstStatus status = bst_erase(&tree, value);
            assert(status == (present[value + 256] ? BST_OK : BST_NOT_FOUND));
            if (status == BST_OK) {
                present[value + 256] = 0;
                --expected;
            }
        }
        assert(bst_contains(&tree, value) == (int)present[value + 256]);
        assert(bst_size(&tree) == expected);
        if (step % 97 == 0) {
            Check check = {present, 0, 0, 1};
            bst_inorder(&tree, check_value, &check);
            assert(check.count == expected);
            assert(bst_height(&tree) <= expected);
        }
    }
    bst_destroy(&tree);
    assert(bst_size(&tree) == 0 && bst_height(&tree) == 0);
    bst_destroy(&tree);
    for (step = 0; step < 10000; ++step) {
        insert(&tree, step);
    }
    assert(bst_height(&tree) == 10000);
    bst_destroy(&tree);
    for (step = 10000; step > 0; --step) {
        insert(&tree, step);
    }
    assert(bst_height(&tree) == 10000);
    for (step = 10000; step > 0; --step) {
        assert(bst_erase(&tree, step) == BST_OK);
    }
    assert(bst_height(&tree) == 0);
    bst_destroy(&tree);
    puts("Task 01: all tests passed");
    return EXIT_SUCCESS;
}
