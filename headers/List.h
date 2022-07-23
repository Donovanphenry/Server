#include <stdlib.h>
#include <stdio.h>
#include "HTTPServer.h"

#ifndef LIST_H
#define LIST_H

typedef struct ListObj *List;

// Constructors-Destructors ---------------------------------------------------
List newList();      // Creates and returns a new empty List.
void freeList(List *pL); // Frees all heap memory associated with *pL, and sets
                         // *pL to NULL.

// Access functions -----------------------------------------------------------
int length(List L);         // Returns the number of elements in L.

// Manipulation procedures ----------------------------------------------------
void prepend(List L, int* x);      // Insert new element into L. If L is non-empty,
                                  // insertion takes place before front element.
void append(List L, int* x);       // Insert new element into L. If L is non-empty,

int* deleteFront(List L);         // Delete the front element. Pre: length()>0

int* deleteBack(List L);          // Delete the back element. Pre: length()>0

void printList(FILE *out, List L);
#endif
