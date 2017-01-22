#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

#include "./parse_input.h"

void readFirstLine(long* n, long* k, long* v) {
    scanf("%ld%ld%ld", n, k, v);
}

Node* parseNode(char* s, long b, long e) {
    Node* node = malloc(sizeof(Node));
    while (isspace(s[b]))
        ++b;
    while (isspace(s[e]))
        --e;
    while (s[b] == '(' && s[e] == ')') {
        ++b;
        --e;
    }

    // check if SUM or MULTIPLY
    long openBrackets = 0;
    long mid = -1;
    bool endFor = false;
    for (long i = b; i <= e && !endFor; ++i) {
        switch (s[i]) {
            case '(':
                ++openBrackets;
                break;
            case ')':
                --openBrackets;
                break;
            case '+':
                if (openBrackets == 0) {
                    node->type = SUM;
                    mid = i;
                    endFor = true;
                }
                break;
            case '*':
                if (openBrackets == 0) {
                    node->type = MULTIPLY;
                    mid = i;
                    endFor = true;
                }
                break;
            default:
                continue;
        }
    }
    if (endFor) {
        node->parents = malloc(sizeof(NodeList));
        node->parents->node = parseNode(s, b, mid - 1);
        node->parents->next = malloc(sizeof(NodeList));
        node->parents->next->node = parseNode(s, mid + 1, e);
        node->parents->next->next = NULL;
        return node;
    }

    // check if MINUS
    if (s[b] == '-') {
        node->type = MINUS;
        node->parents = malloc(sizeof(NodeList));
        node->parents->node = parseNode(s, b + 1, e);
        node->parents->next = NULL;
        return node;
    }

    // check if PNUM
    if (s[b] >= '0' && s[b] <= '9') {
        node->type = PNUM;
        node->parents = NULL;
        sscanf(s + b, "%ld", &node->value);
        return node;
    }

    // check if VAR
    if (s[b] == 'x') {
        node->type = VAR;
        node->parents = NULL;
        sscanf(s + b, "x[%ld]", &node->value);
        return node;
    }

    return NULL;
}

Node* readEquation() {
    char s[1000];
    long x;
    scanf("%*d x[%ld] = %[^\n]s", &x, s);
    // TODO: do something with x
    return parseNode(s, 0, ((long) strlen(s)) - 1);
}

InitializerList* parseInitializerList(char* s) {
    long var, num;
    if (sscanf(s, " x[%ld] %ld", &var, &num) >= 2) {
        int xs = 0;
        int i;
        for (i = 0; s[i] != 0 && s[i] != '\n'; ++i) {
            if (s[i] == 'x')
                ++xs;
            if (xs == 2)
                break;
        }
        InitializerList* initializerList = malloc(sizeof(InitializerList));
        initializerList->var = var;
        initializerList->num = num;
        initializerList->next = parseInitializerList(s + i);
        return initializerList;
    } else {
        return NULL;
    }
}

InitializerList* readInitializerList() {
    char s[1000];
    scanf(" %[^\n]s", s);
    long len = strlen(s);
    bool wasDigit = false;
    long i;
    for (i = 0; i < len; ++i)
        if ('0' <= s[i] && s[i] <= '9')
            wasDigit = true;
        else if (wasDigit && isspace(s[i]))
            break;
    return parseInitializerList(s + i);
}