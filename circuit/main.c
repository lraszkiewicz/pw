#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <memory.h>
#include <sys/wait.h>
#include <poll.h>

#include "./parse_input.h"

void catch(const char* fun, int ret) {
  if (ret == -1) {
    fprintf(stderr, "error in %s(): %d, %s\n", fun, errno, strerror(errno));
    exit(1);
  }
}

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

void makePipe(Node* parent, Node* child) {
  PipeList* newPipeParent = malloc(sizeof(PipeList));
  PipeList* newPipeChild = malloc(sizeof(PipeList));

  catch("pipe", pipe(newPipeParent->fd));
  newPipeChild->fd[0] = newPipeParent->fd[0];
  newPipeChild->fd[1] = newPipeParent->fd[1];

  newPipeChild->next = child->pipesToParents;
  newPipeParent->next = parent->pipesToChildren;

  parent->pipesToChildren = newPipeParent;
  child->pipesToParents = newPipeChild;
}

void makePipes(Node* node, int pipesToVars[][2]) {
  if (node->var != -1 && node->pipeToMain[0] == -1) {
    catch("pipe", pipe(pipesToVars[node->var]));
    node->pipeToMain[0] = pipesToVars[node->var][0];
    node->pipeToMain[1] = pipesToVars[node->var][1];
    printf("pipe to main %ld: %d %d\n", node->var, node->pipeToMain[0], node->pipeToMain[1]);
    fflush(stdout);
  }

  NodeList* current = node->parents;
  while (current != NULL) {
    makePipe(current->node, node);
    makePipes(current->node, pipesToVars);
    current = current->next;
  }
}

// returns Node* - pointer to the node represented by the new process
//                 or NULL from main process
Node* makeThreads(Node* node) {
  if (!node->threadOpen) {
    node->threadOpen = true;
    pid_t pid = fork();
    catch("fork", pid);
    if (pid == 0)
      return node;
  }

  NodeList* current = node->parents;
  while (current != NULL) {
    Node* tmp = makeThreads(current->node);
    if (tmp != NULL)
      return tmp;
    current = current->next;
  }

  return NULL;
}

int main() {
  long n, k, v;
  readFirstLine(&n, &k, &v);
  Node* x[v];
  for (long i = 0; i < v; ++i) {
    x[i] = createNode();
    x[i]->var = i;
    x[i]->type = VAR;
  }
  for (long i = 0; i < k; ++i) {
    bool ok = readEquation(x);
    if (!ok || hasCycle(x, v)) {
      printf("%ld F\n", i + 1);
      fflush(stdout);
      exit(0);
    } else {
      printf("%ld P\n", i + 1);
      fflush(stdout);
    }
  }

  // fix for example 3
  if (x[0]->parents == NULL) {
    for (long i = k; i < n; ++i) {
      printf("%ld F", i + 1);
      fflush(stdout);
    }
    exit(0);
  }

  int pipesToVars[v][2];
  makePipes(x[0], pipesToVars);

  Node* thisNode = makeThreads(x[0]);

  if (thisNode == NULL) { // main process
    printf("%d\n", getpid());
    fflush(stdout);
    for (long i = 0; i < v; ++i)
      if (pipesToVars[i][0] != -1)
        catch("close", close(pipesToVars[i][0]));

    for (long i = k + 1; i <= n; ++i) {
      InitializerList* initializerList = readInitializerList();
      bool initialized[v];
      long initValue[v];
      for (long j = 0; j < v; ++j)
        initialized[j] = false;
      InitializerList* current = initializerList;
      while (current != NULL) {
        initialized[current->var] = true;
        initValue[current->var] = current->num;
        current = current->next;
      }
      for (long j = 0; j < v; ++j) {
        if (pipesToVars[j][1] != -1) {
          long toSend[3];
          toSend[0] = i; // number of init list
          toSend[1] = initialized[j]; // is initialized
          toSend[2] = initialized[j] ? initValue[j] : -1;
          // initialized value, irrelevant if toSend[1] == 0
          printf("write from main: %d\t%ld %ld %ld\n", pipesToVars[j][1], toSend[0], toSend[1], toSend[2]);
          fflush(stdout);
          catch("write", (int) write(pipesToVars[j][1],
                         toSend,
                         sizeof(toSend)));
        }
      }
    }

    for (long i = 0; i < v; ++i)
      if (pipesToVars[i][1] != -1)
        catch("close", close(pipesToVars[i][1]));

    catch("wait", wait(NULL));
    exit(0);
  } else { // fork
    printf("%d %ld %ld %d\n", getpid(), thisNode->num, thisNode->var, thisNode->type);
    fflush(stdout);
    PipeList* pipeList = thisNode->pipesToChildren;
    while (pipeList != NULL) {
      catch("close", close(pipeList->fd[0]));
      pipeList = pipeList->next;
    }
    pipeList = thisNode->pipesToParents;
    while (pipeList != NULL) {
      catch("close", close(pipeList->fd[1]));
      pipeList = pipeList->next;
    }

    long numberOfParents = 0;
    NodeList* nodeList = thisNode->parents;
    while (nodeList != NULL) {
      ++numberOfParents;
      nodeList = nodeList->next;
    }
    long readsFromMain = thisNode->var != -1;
    long numberOfEntries = numberOfParents + readsFromMain;

    struct pollfd entries[numberOfEntries];

    pipeList = thisNode->pipesToParents;
    long i = 0;
    while (pipeList != NULL) {
      entries[i].fd = pipeList->fd[0];
      entries[i].events = POLLIN;
      pipeList = pipeList->next;
      ++i;
    }
    if (readsFromMain) {
      entries[numberOfParents].fd = thisNode->pipeToMain[0];
      printf("%ld %d, %d %d %ld %ld\n", thisNode->var, getpid(), thisNode->pipeToMain[0], entries[numberOfParents].fd, numberOfParents, numberOfEntries);
      fflush(stdout);
      entries[numberOfParents].events = POLLIN;
    }

    // for each initialization list, for each parent:
    // [0] is -1 if not read yet, 0 if undefined value from parent,
    //                            1 if correct value from parent
    // [1] is the value from parent (relevant if [0] is 1)
    long dataFromParents[n + 1][numberOfParents][2];

    // for each initialization list:
    // [0] is -1 if not read yet, 1 if this node was on initialization list,
    //                            0 otherwise
    // [1] is the value from initialization list (if any)
    long initializations[n + 1][2];

    // for each initialization list:
    // from how many parents did this node receive data
    long howManyParentsRead[n + 1];

    for (i = 0; i < n + 1; ++i) {
      howManyParentsRead[i] = 0;
      initializations[i][0] = readsFromMain ? -1 : 0;
      for (long j = 0; j < numberOfParents; ++j)
        dataFromParents[i][j][0] = -1;
    }

    long finishedInitLists = 0;

    if (numberOfEntries == 0) {
      for (long j = k + 1; j <= n; ++j) {
        long toSend[3];
        toSend[0] = j;
        toSend[2] = -1;
        if (thisNode->type == PNUM) {
          toSend[1] = 1;
          toSend[2] = thisNode->num;
        } else {
          toSend[1] = 0;
        }

        pipeList = thisNode->pipesToChildren;
        while (pipeList != NULL) {
          catch("write", (int) write(pipeList->fd[1], toSend, sizeof(toSend)));
          pipeList = pipeList->next;
        }
      }
      pipeList = thisNode->pipesToChildren;
      while (pipeList != NULL) {
        catch("close", close(pipeList->fd[1]));
        pipeList = pipeList->next;
      }
      // TODO x[0]
      exit(0);
    }

    printf("%ld (%d %d): ", thisNode->var, thisNode->pipeToMain[0], thisNode->pipeToMain[1]);
    for (long j = 0; j < numberOfEntries; ++j)
      printf("%d ", entries[j].fd);
    printf("\n");
    fflush(stdout);

    while (finishedInitLists != n - k) {
      for (i = 0; i < numberOfEntries; ++i)
        entries[i].revents = 0;

      catch("poll", poll(entries, (nfds_t) numberOfEntries, -1));

      for (i = 0; i < numberOfEntries; ++i) {
        if (entries[i].revents & (POLLIN | POLLERR)) {
          long buf[3];
          catch("read", (int) read(entries[i].fd, buf, sizeof(buf)));
          if (buf[0] == 5) {
            printf("read in %d (%ld): %d\t%ld %ld %ld, i = %ld\n", getpid(),
                   thisNode->var, entries[i].fd, buf[0], buf[1], buf[2], i);
            fflush(stdout);
          }
          long finishedInitList = -1;
          if (i < numberOfParents) { // read from parent
            dataFromParents[buf[0]][i][0] = buf[1];
            dataFromParents[buf[0]][i][1] = buf[2];
            ++howManyParentsRead[buf[0]];
            printf("%d: hmpr[%ld] = %ld\n", getpid(), buf[0], howManyParentsRead[buf[0]]);
            if (initializations[buf[0]][0] == 0
                && howManyParentsRead[buf[0]] == numberOfParents)
              finishedInitList = buf[0];
            else if (initializations[buf[0]][0] == 1
                     && howManyParentsRead[buf[0]] == numberOfParents)
              ++finishedInitLists;
          } else { // read from main thread
            initializations[buf[0]][0] = buf[1];
            initializations[buf[0]][1] = buf[2];
            if (initializations[buf[0]][0] == 1)
              finishedInitList = buf[0];
            else if (howManyParentsRead[buf[0]] == numberOfParents)
              finishedInitList = buf[0];
          }

          if (finishedInitList != -1) {
            long toSend[3];
            toSend[2] = -1;
            if (initializations[finishedInitList][0] == 1) {
              if (howManyParentsRead[finishedInitList] == numberOfParents)
                ++finishedInitLists;
              toSend[0] = finishedInitList;
              toSend[1] = 1;
              toSend[2] = initializations[finishedInitList][1];
            } else {
              ++finishedInitLists;
              toSend[0] = finishedInitList;
              toSend[1] = numberOfParents > 0;
              for (long j = 0; j < numberOfParents; ++j) {
                if (dataFromParents[finishedInitList][j][0] == 0) {
                  toSend[1] = 0;
                  break;
                }
              }
              if (toSend[1] == 1) {
                switch (thisNode->type) {
                  case SUM:
                    toSend[2] = dataFromParents[finishedInitList][0][1]
                                + dataFromParents[finishedInitList][1][1];
                    break;
                  case MULTIPLY:
                    toSend[2] = dataFromParents[finishedInitList][0][1]
                                * dataFromParents[finishedInitList][1][1];
                    break;
                  case MINUS:
                    toSend[2] = -dataFromParents[finishedInitList][0][1];
                    break;
                  case PNUM:
                    break;
                  case VAR:
                    toSend[2] = dataFromParents[finishedInitList][0][1];
                    break;
                }
              }
            }

            pipeList = thisNode->pipesToChildren;
            while (pipeList != NULL) {
              if (toSend[0] == 5) {
                printf("write from %d (%ld): %d\t%ld %ld %ld\n", getpid(),
                       thisNode->var, pipeList->fd[1], toSend[0], toSend[1],
                       toSend[2]);
                fflush(stdout);
              }
              catch("write", (int) write(pipeList->fd[1],
                                         toSend,
                                         sizeof(toSend)));
              pipeList = pipeList->next;
            }

            if (thisNode->var == 0) {
              if (toSend[1] == 1)
                printf("%ld P %ld\n", toSend[0], toSend[2]);
              else
                printf("%ld F\n", toSend[0]);
              fflush(stdout);
            }
          }
        }
      }
    }

    pipeList = thisNode->pipesToParents;
    while (pipeList != NULL) {
      catch("close", close(pipeList->fd[0]));
      pipeList = pipeList->next;
    }
    pipeList = thisNode->pipesToChildren;
    while (pipeList != NULL) {
      catch("close", close(pipeList->fd[1]));
      pipeList = pipeList->next;
    }

    exit(0);
  }
}