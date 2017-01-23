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

typedef struct PipeList {
  int fd[2];
  struct PipeList* next;
} PipeList;

typedef struct Node {
  long num; // used only if type is PNUM
  long var; // i from x[i] if applies, -1 in other cases
  NodeType type;
  NodeList* parents;
  NodeList* children;
  PipeList* pipesToParents;
  PipeList* pipesToChildren;
  int pipeToMain[2];
  bool threadOpen;
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

Node* createNode();

void readFirstLine(long* n, long* k, long* v);

bool readEquation(Node** x);

InitializerList* readInitializerList();

#endif //CIRCUIT_PARSE_INPUT_H
