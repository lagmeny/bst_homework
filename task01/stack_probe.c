#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct ProbeNode {
    int value;
    struct ProbeNode *left;
    struct ProbeNode *right;
} ProbeNode;

static volatile size_t released = 0;

static void destroy_recursively(ProbeNode *node) {
    if (node != NULL) {
        destroy_recursively(node->left);
        destroy_recursively(node->right);
        free(node);
        ++released;
    }
}

int main(int argc, char **argv) {
    ProbeNode *root = NULL;
    unsigned long long count;
    unsigned long long i;
    char *end;
    if (argc != 2 || argv[1][0] == '-') {
        return EXIT_FAILURE;
    }
    errno = 0;
    count = strtoull(argv[1], &end, 10);
    if (errno != 0 || *end != '\0' || end == argv[1] || count > INT_MAX) {
        return EXIT_FAILURE;
    }
    for (i = 0; i < count; ++i) {
        ProbeNode *node = malloc(sizeof(*node));
        if (node == NULL) {
            while (root != NULL) {
                ProbeNode *next = root->right;
                free(root);
                root = next;
            }
            return EXIT_FAILURE;
        }
        node->value = (int)(count - i - 1);
        node->left = NULL;
        node->right = root;
        root = node;
    }
    destroy_recursively(root);
    printf("Released recursively: %zu\n", released);
    return EXIT_SUCCESS;
}
