#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "./parse_input.h"

bool dfsCycle(Node* node, int* visited) {
    if (node->var != -1) {
        if (visited[node->var] == 1)
            return true;
        visited[node->var] = 1;
    }
    NodeList* current = node->parents;
    while (current != NULL) {
        if (dfsCycle(current->node, visited))
            return true;
        current = current->next;
    }
    if (node->var != -1)
        visited[node->var] = 2;
    return false;
}

bool hasCycle(Node** x, long v) {
    int visited[v];
    for (int i = 0; i < v; ++i)
        visited[i] = 0;
    return dfsCycle(x[0], visited);
}

bool goodInit(Node* node, bool* initialized) {
    printf("goodInit %ld\n", node->var);
    if (node->type == PNUM || (node->var != -1 && initialized[node->var]))
        return true;
    if (node->parents == NULL)
        return false;
    NodeList* current = node->parents;
    while (current != NULL) {
        if (!goodInit(current->node, initialized))
            return false;
        current = current->next;
    }
    return true;
}

int main() {
    long n, k, v;
    readFirstLine(&n, &k, &v);
    Node* x[v];
    for (long i = 0; i < v; ++i) {
        x[i] = malloc(sizeof(Node));
        x[i]->num = -1;
        x[i]->var = i;
        x[i]->parents = NULL;
        x[i]->children = NULL;
        x[i]->type = VAR;
    }
    for (long i = 0; i < k; ++i) {
        bool ok = readEquation(x);
        if (!ok || hasCycle(x, v)) {
            printf("%ld F\n", i + 1);
            exit(0);
        }
        else
            printf("%ld P\n", i + 1);
    }

    if (x[0]->parents == NULL) {
        for (long i = k; i < n; ++i)
            printf("%ld F", i + 1);
        exit(0);
    }

    for (long i = k; i < n; ++i) {
        InitializerList* initializerList = readInitializerList();
        bool initialized[v];
        for (long j = 0; j < v; ++j)
            initialized[j] = false;
        InitializerList* current = initializerList;
        while (current != NULL) {
            initialized[current->var] = true;
            current = current->next;
        }
        if (!goodInit(x[0], initialized))
            printf("%ld F\n", i + 1);
        else
            printf("%ld P\n", i + 1);
    }
    return 0;
}