/****************************************************************************************
* Name: Donovan Henry
* CruzID: dphenry
* Assignment: PA1
* File: List.c
*****************************************************************************************/

#include "List.h"

// -------- Private Method Signatures -----------
int isEmpty(List L);

// structs --------------------------------------------------------------------

// private NodeObj type
typedef struct NodeObj
{
    int* data;
    struct NodeObj *next;
    struct NodeObj *prev;
} NodeObj;

// private Node type
typedef NodeObj *Node;

typedef struct ListObj
{
    int count;
    int index;
    Node head;
    Node cursor;
    Node tail;
} ListObj;

// ------- Node methods -----------

// Creates new node with the node's next &
// previous pointers being set to NULL. The data
// passed into the constructor is assigned to the
// node's data value. A new node object is returned.
// No preconditions.
Node newNode(int* data)
{
    Node node = malloc(sizeof(NodeObj));
    node->next = NULL;
    node->prev = NULL;
    node->data = data;

    return (node);
}

// Frees the memory that *pN points to.
// Pre: pN != NULL && *pN != NULL.
void freeNode(Node *pN)
{
    if (pN == NULL || *pN == NULL)
    {
        return;
    }

    (*pN)->prev = NULL;
    (*pN)->next = NULL;
    free(*pN);
    *pN = NULL;
}

// Constructs a new list object and returns it.
List newList()
{
    List list = (List)malloc(sizeof(ListObj));
    list->head = NULL;
    list->cursor = NULL;
    list->tail = NULL;
    list->count = 0;
    list->index = -1;
    return (list);
}

// frees the list held by the *pL. pL and
// *pL must both be non-NULL.
void freeList(List *pL)
{
    if (pL != NULL && *pL != NULL)
    {
        while (isEmpty(*pL) == 0)
        {
            deleteFront(*pL);
        }

        (*pL)->head = NULL;
        (*pL)->cursor = NULL;
        (*pL)->tail = NULL;
        free(*pL);
        *pL = NULL;
    }
}

// ------- Access functions -------
// Returns the number of elements in L.
int length(List L)
{
    if (L == NULL)
    {
        printf("List Error: calling length() on NULL List reference\n");
        exit(EXIT_FAILURE);
    }

    return L->count;
}

// Insert new element into L. If L is non-empty,
// insertion takes place before front element.
void prepend(List L, int* x)
{
    if (L == NULL)
    {
        printf("List Error: calling prepend() on NULL List reference\n");
        exit(EXIT_FAILURE);
    }

    Node newHead = newNode(x);
    newHead->next = L->head;
    newHead->prev = NULL;

    if (isEmpty(L) == 1)
    {
        L->tail = newHead;
    }
    else
    {
        L->head->prev = newHead;
    }

    L->head = newHead;
    L->count++;
}

// Insert new element into L. If L is non-empty,
// insertion takes place after back element.
void append(List L, int* x)
{
    if (L == NULL)
    {
        printf("List Error: calling append() on NULL List reference\n");
        exit(EXIT_FAILURE);
    }

    Node newTail = newNode(x);
    newTail->prev = L->tail;
    newTail->next = NULL;

    if (isEmpty(L) == 1)
    {
        L->head = newTail;
    }
    else
    {
        L->tail->next = newTail;
    }

    L->tail = newTail;
    L->count++;
}

// Delete the front element. Pre: length()>0
int* deleteFront(List L)
{
    if (L == NULL)
    {
        printf("List Error: calling deleteFront() on NULL List reference\n");
        exit(EXIT_FAILURE);
    }

    if (length(L) == 0)
			return NULL;

    if (L->head == L->tail)
    {
        L->tail = NULL;
    }

    Node kill = L->head;
		int* killData = kill->data;
    L->head = L->head->next;
    if (L->head != NULL)
    {
        L->head->prev = NULL;
    }
    freeNode(&kill);
    L->count--;
		return killData;
}

// Delete the back element. Pre: length()>0
int* deleteBack(List L)
{
    if (L == NULL)
    {
        printf("List Error: calling delete() on NULL List reference\n");
        exit(EXIT_FAILURE);
    }

    if (isEmpty(L) == 1)
			return NULL;

    if (L->tail == L->head)
    {
        L->head = NULL;
    }

    Node kill = L->tail;
		int* killData = kill->data;
    L->tail = L->tail->prev;
    if (L->tail != NULL)
    {
        L->tail->next = NULL;
    }
    freeNode(&kill);
    L->count--;
		return killData;
}

// Other operations -----------------------------------------------------------

// Returns an integer mapping to a boolean value where
// a value of 1 represents true and a 0 represents false.
// Functions checks to see if a list is empty, ie has no
// nodes, and if it does, it returns 1, ie the statement
// that the list is empty is true. If the list is not empty,
// then a 0 is returned representing that the propositon "the list
// is empty" is false. I define a NULL list as empty, but this is
// convention. You could justifiably exit with an error code if you like.
int isEmpty(List l)
{
    if (l == NULL || l->count == 0)
    {
        return 1;
    }

    return 0;
}

// Prints to the file pointed to by out, a
// string representation of L consisting
// of a space separated sequence of integers,
void printList(FILE *out, List L)
{
    if (out == NULL)
    {
        printf("ERROR: File open unsuccessful.");
        exit(EXIT_FAILURE);
    }
    Node curr = L->head;

    while (curr != NULL)
    {
        fprintf(out, "%d ", *(curr->data));
        curr = curr->next;
    }
}

