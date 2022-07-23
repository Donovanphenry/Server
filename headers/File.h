#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <err.h>
#define _XOPEN_SOURCE
#include <time.h>

char *strptime(const char *s, const char *format, struct tm *tm);

#define BUFFER_SIZE 10000

#ifndef FILE_H
#define FILE_H

typedef struct FileObj *File;

// Constructors-Destructors ---------------------------------------------------
File newFile(char *filename, int m, time_t modtime);      // Creates and returns a new empty List.
void freeFile(File *p_file); // Frees all heap memory associated with *pL, and sets
                         // *pL to NULL.

// Access functions -----------------------------------------------------------
char* getFilename(File file);
char* getFileContents(File file);
time_t getFileModtime(File file);
int getContentLength(File file);
// Manipulation procedures ----------------------------------------------------
void updateFile(File file, char *filename);          // Delete the back element. Pre: length()>0
void setFileModtime(File file, time_t modtime);
#endif
