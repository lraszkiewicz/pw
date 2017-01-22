#ifndef CIRCUIT_PARSE_INPUT_H
#define CIRCUIT_PARSE_INPUT_H

typedef enum NodeType {
    SUM,
    MULTIPLY,
    MINUS,
    PNUM,
    VAR
} NodeType;

typedef struct Node {
    long value; // if type is PNUM or VAR
    struct NodeList* parents;
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

Node* readEquation();

InitializerList* readInitializerList();

#endif //CIRCUIT_PARSE_INPUT_H
