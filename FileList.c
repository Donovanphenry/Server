/****************************************************************************************
* Name: Donovan Henry
* CruzID: dphenry
* Assignment: PA1
* File: List.c
*****************************************************************************************/

#include "FileList.h"

// -------- Private Method Signatures -----------
int isEmptyQueue(FileList file_list);

// structs --------------------------------------------------------------------

// private NodeObj type
typedef struct FileNodeObj
{
    File data;
    struct FileNodeObj *next;
    struct FileNodeObj *prev;
} FileNodeObj;

// private Node type
typedef FileNodeObj *FileNode;

typedef struct FileListObj
{
    int count;
    int index;
    FileNode head;
    FileNode cursor;
    FileNode tail;
} FileListObj;

// ------- Node methods -----------

// Creates new node with the node's next &
// previous pointers being set to NULL. The data
// passed into the constructor is assigned to the
// node's data value. A new node object is returned.
// No preconditions.
FileNode newFileNode(File data)
{
    FileNode node = malloc(sizeof(FileNodeObj));
    node->next = NULL;
    node->prev = NULL;
    node->data = data;

    return (node);
}

// Frees the memory that *pN points to.
// Pre: pN != NULL && *pN != NULL.
void freeFileNode(FileNode *pN)
{
    if (pN == NULL || *pN == NULL)
    {
        return;
    }

    (*pN)->prev = NULL;
    (*pN)->next = NULL;
		freeFile(&((*pN)->data));
    free(*pN);
    *pN = NULL;
}

// Constructs a new list object and returns it.
FileList newFileList()
{
    FileList file_list = (FileList)malloc(sizeof(FileListObj));
    file_list->head = NULL;
    file_list->cursor = NULL;
    file_list->tail = NULL;
    file_list->count = 0;
    file_list->index = -1;
    return (file_list);
}

// frees the list held by the *pL. pL and
// *pL must both be non-NULL.
void freeFileList(FileList *p_file_list)
{
    if (p_file_list != NULL && *p_file_list != NULL)
    {
        while (isEmptyQueue(*p_file_list) == 0)
        {
            dequeue(*p_file_list);
        }

        (*p_file_list)->head = NULL;
        (*p_file_list)->cursor = NULL;
        (*p_file_list)->tail = NULL;
        free(*p_file_list);
        *p_file_list = NULL;
    }
}

// ------- Access functions -------
// Returns the number of elements in L.
int queueLength(FileList file_list)
{
    if (file_list == NULL)
    {
        printf("FileList Error: calling queueLength() on NULL FileList reference\n");
        exit(EXIT_FAILURE);
    }

    return file_list->count;
}

// Insert new element into L. If L is non-empty,
// insertion takes place after back element.
void enqueue(FileList file_list, File file)
{
    if (file_list == NULL)
    {
        printf("FileList Error: calling enqueue() on NULL FileList reference\n");
        exit(EXIT_FAILURE);
    }

    FileNode newTail = newFileNode(file);
    newTail->prev = file_list->tail;
    newTail->next = NULL;

    if (isEmptyQueue(file_list) == 1)
    {
        file_list->head = newTail;
    }
    else
    {
        file_list->tail->next = newTail;
    }

    file_list->tail = newTail;
    file_list->count++;
}

// Delete the front element. Pre: queueLength()>0
File dequeue(FileList file_list)
{
    if (file_list == NULL)
    {
        printf("FileList Error: calling dequeue() on NULL FileList reference\n");
        exit(EXIT_FAILURE);
    }

    if (queueLength(file_list) == 0)
			return NULL;

    if (file_list->head == file_list->tail)
    {
        file_list->tail = NULL;
    }

    FileNode kill = file_list->head;
		File killData = kill->data;
    file_list->head = file_list->head->next;
    if (file_list->head != NULL)
    {
        file_list->head->prev = NULL;
    }
    freeFileNode(&kill);
    file_list->count--;
		return killData;
}

void removeFile(FileList file_list, char *target)
{
	FileNode curr = file_list->head;

	while (curr != NULL)
	{
		if (strncmp(getFilename(curr->data), target, strlen(target)) == 0)
		{
			if (curr == file_list->tail)
				file_list->tail = curr->prev;
			if (curr == file_list->head)
				file_list->head = curr->next;

			if (curr->next != NULL)
				curr->next->prev = curr->prev;
			if (curr->prev != NULL)
				curr->prev->next = curr->next;
			freeFileNode(&curr);
			file_list->count--;
			return;
		}
		curr = curr->next;
	}
}

// Other operations -----------------------------------------------------------

File containsFile(FileList file_list, char *filename)
{
	FileNode curr = file_list->tail;

	while (curr != NULL)
	{
		if (strncmp(getFilename(curr->data), filename, strlen(filename)) == 0)
		{
			if (strlen(getFilename(curr->data)) == strlen(filename))
				return curr->data;
		}
		curr = curr->prev;
	}

	return NULL;
}

// Returns an integer mapping to a boolean value where
// a value of 1 represents true and a 0 represents false.
// Functions checks to see if a list is empty, ie has no
// nodes, and if it does, it returns 1, ie the statement
// that the list is empty is true. If the list is not empty,
// then a 0 is returned representing that the propositon "the list
// is empty" is false. I define a NULL list as empty, but this is
// convention. You could justifiably exit with an error code if you like.
int isEmptyQueue(FileList file_list)
{
    if (file_list == NULL || file_list->count == 0)
    {
        return 1;
    }

    return 0;
}

void printFiles(FileList file_list)
{
	if (file_list == NULL)
	{
		printf("FileList Error: calling printFiles() on a NULL FileList reference\n");
		exit(EXIT_FAILURE);
	}

	FileNode curr = file_list->head;

	printf("head -> ");
	while (curr != NULL)
	{
		printf("%s -> ", getFilename(curr->data));
		curr = curr->next;
	}

	printf("tail");
}
