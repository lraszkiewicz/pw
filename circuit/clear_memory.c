#include "clear_memory.h"

void freeInitializerList(InitializerList *list) {
  if (list == NULL)
    return;
  freeInitializerList(list->next);
  free(list);
}

void freePipeList(PipeList *list) {
  if (list == NULL)
    return;
  freePipeList(list->next);
  free(list);
}

void freeNodeList(NodeList *list) {
  if (list == NULL)
    return;
  freeNodeList(list->next);
  free(list);
}

void freeNode(Node *node) {
  freeNodeList(node->parents);
  freeNodeList(node->children);
  freePipeList(node->pipesToParents);
  freePipeList(node->pipesToChildren);
  free(node);
}

void freeAllNodes(NodeList* list) {
  if (list == NULL)
    return;
  freeAllNodes(list->next);
  freeNode(list->node);
  free(list);
}
