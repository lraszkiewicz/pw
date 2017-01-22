#include <stdio.h>
#include "./parse_input.h"

int main() {
    long n, k, v;
    readFirstLine(&n, &k, &v);
    int x[v];
    for (long i = 0; i < k; ++i) {
        Node* node = readEquation();
        printf("%d\n", node == NULL);
    }
    for (long i = k; i < n; ++i) {
        InitializerList* initializerList = readInitializerList();
    }
    return 0;
}