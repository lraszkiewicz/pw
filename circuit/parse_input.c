#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

#include "./parse_input.h"
#include "clear_memory.h"

void readFirstLine(long* n, long* k, long* v) {
  scanf("%ld%ld%ld", n, k, v);
}

void insertNode(Node* node, NodeList** list) {
  NodeList* newList = malloc(sizeof(NodeList));
  newList->node = node;
  newList->next = *list;
  *list = newList;
}

Node* createNode() {
  Node* node = malloc(sizeof(Node));
  node->var = -1;
  node->num = -1;
  node->parents = NULL;
  node->children = NULL;
  node->pipesToChildren = NULL;
  node->pipesToParents = NULL;
  node->pipeToMain[0] = -1;
  node->pipeToMain[1] = -1;
  node->threadOpen = false;
  return node;
}

Node* parseNode(char* s, long b, long e, Node** x, Node* currentNode,
                NodeList** listOfAllNodes) {
  Node* node;
  if (currentNode == NULL)
    node = createNode();
  else
    node = currentNode;

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
    insertNode(parseNode(s, mid + 1, e, x, NULL, listOfAllNodes),
               &node->parents);
    insertNode(node, &node->parents->node->children);
    insertNode(parseNode(s, b, mid - 1, x, NULL, listOfAllNodes),
               &node->parents);
    insertNode(node, &node->parents->node->children);
    if (currentNode == NULL)
      insertNode(node, listOfAllNodes);
    return node;
  }

  // check if MINUS
  if (s[b] == '-') {
    node->type = MINUS;
    insertNode(parseNode(s, b + 1, e, x, NULL, listOfAllNodes), &node->parents);
    insertNode(node, &node->parents->node->children);
    if (currentNode == NULL)
      insertNode(node, listOfAllNodes);
    return node;
  }

  // check if PNUM
  if ('0' <= s[b] && s[b] <= '9') {
    node->type = PNUM;
    sscanf(s + b, "%ld", &node->num);
    if (currentNode == NULL)
      insertNode(node, listOfAllNodes);
    return node;
  }

  // check if VAR
  if (s[b] == 'x') {
    long var;
    sscanf(s + b, "x[%ld]", &var);
    if (currentNode == NULL) {
      freeNode(node);
      return x[var];
    } else {
      insertNode(x[var], &node->parents);
      insertNode(node, &x[var]->children);
      return node;
    }
  }

  freeNode(node);
  return NULL;
}

// false if error
bool readEquation(Node** x, NodeList** listOfAllNodes) {
  char s[1000];
  long var;
  scanf("%*d x[%ld] = %[^\n]s", &var, s);
  if (x[var]->parents != NULL || x[var]->type != VAR)
    return false;

  parseNode(s, 0, (strlen(s)) - 1, x, x[var], listOfAllNodes);
  return true;
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
