#ifndef CIRCUIT_CLEAR_MEMORY_H
#define CIRCUIT_CLEAR_MEMORY_H

#include <stdlib.h>
#include "parse_input.h"

void freeInitializerList(InitializerList* list);

void freeNode(Node *node);

void freeAllNodes(NodeList* list);

#endif //CIRCUIT_CLEAR_MEMORY_H
