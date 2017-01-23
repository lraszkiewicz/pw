#ifndef CIRCUIT_PARSE_INPUT_H
#define CIRCUIT_PARSE_INPUT_H

typedef enum NodeType {
    SUM,
    MULTIPLY,
    MINUS,
    PNUM,
    VAR
} NodeType;

typedef struct NodeList NodeList;

typedef struct Node {
    long num; // used only if type is PNUM
    long var; // i from x[i] if applies, -1 in other cases
    NodeList* parents;
    NodeList* children;
    NodeType type;
} Node;

typedef struct NodeList {
    Node* node;
    struct NodeList* next;
} NodeList;

typedef struct InitializerList {
    long var;
    long num;
    struct InitializerList* next;
} InitializerList;

void readFirstLine(long* n, long* k, long* v);

bool readEquation(Node** x);

InitializerList* readInitializerList();

#endif //CIRCUIT_PARSE_INPUT_H
