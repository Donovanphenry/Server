#include <stdlib.h>
#include <stdio.h>
#include "File.h"

#ifndef FILE_LIST_H
#define FILE_LIST_H

typedef struct FileListObj *FileList;

// Constructors-Destructors ---------------------------------------------------
FileList newFileList();      // Creates and returns a new empty List.
void freeFileList(FileList *p_file_list); // Frees all heap memory associated with *pL, and sets
                         // *pL to NULL.

// Access functions -----------------------------------------------------------
int queueLength(FileList file_list);         // Returns the number of elements in L.

// Manipulation procedures ----------------------------------------------------
void enqueue(FileList file_list, File file);       // Insert new element into L. If L is non-empty,

File dequeue(FileList file_list);         // Delete the front element. Pre: length()>0
void removeFile(FileList file_list, char *target);         // Delete the front element. Pre: length()>0

// Other procedures ---------------------------------------------------------
File containsFile(FileList file_list, char *filename);
void printFiles(FileList file_list);

#endif
