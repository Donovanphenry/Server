#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "List.h"
#include "FileList.h"

#ifndef THREADARG_H
#define THREADARG_H

typedef struct ThreadArgObj *ThreadArg;

// Constructors-Destructors ---------------------------------------------------
ThreadArg newThreadArg(List conn_queue, FileList cache_queue, int m, int R, int *curr_request, int s, HTTPServer *servers, pthread_mutex_t *server_locks, int num_servers);      // Creates and returns a new empty List.
void freeThreadArg(ThreadArg *p_ta); // Frees all heap memory associated with *pL, and sets

// Access functions -----------------------------------------------------------
List getConnQueue(ThreadArg ta);
FileList getCacheQueue(ThreadArg ta);
char*** getPCache(ThreadArg ta);
int getM(ThreadArg ta);					// Returns the file descriptor of the list L.
int getR(ThreadArg ta);					// Returns the file descriptor of the list L.
int* getCurrRequest(ThreadArg ta);					// Returns the file descriptor of the list L.
int getS(ThreadArg ta);					// Returns the file descriptor of the list L.
HTTPServer* getServers(ThreadArg ta);		// Returns the log offset of the list L.
pthread_mutex_t* getServerLocks(ThreadArg ta);					// Returns the file descriptor of the list L.
int getNumServers(ThreadArg ta);					// Returns the file descriptor of the list L.

#endif
