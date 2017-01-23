#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "./parse_input.h"

bool dfs(Node* node, bool* visited) {
    if (node->var != -1) {
        if (visited[node->var])
            return true;
        visited[node->var] = true;
    }
    NodeList* list = node->parents;
    while (list != NULL) {
        if (dfs(list->node, visited))
            return true;
        list = list->next;
    }
    return false;
}

bool hasCycle(Node** x, long v) {
    bool visited[v];
    for (int i = 0; i < v; ++i)
        visited[i] = false;
    return dfs(x[0], visited);
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
    }
    for (long i = k; i < n; ++i) {
        InitializerList* initializerList = readInitializerList();
    }
    return 0;
}