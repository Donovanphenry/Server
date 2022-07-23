#include "File.h"

typedef struct FileObj
{
		char *filename;
		char *fileContents;
		time_t modtime;
		int contentLength;
} FileObj;

// Constructs a new list object and returns it.
File newFile(char *filename, int m, time_t modtime)
{
		int fdr = open(filename, O_RDONLY, 0600);
		int contentLength;
		if (fdr <= 0)
		{
			close(fdr);
			return NULL;
		}

		struct stat sb;

		fstat(fdr, &sb);

		contentLength = sb.st_size;

		if (contentLength > m)
			return NULL;

    File file = (File)malloc(sizeof(FileObj));
		file->filename = filename;
		file->fileContents = malloc(sizeof(char) * contentLength);
		file->modtime = modtime;
		file->contentLength = contentLength;

		int bytes_read;
		char buffer[BUFFER_SIZE];

		while (contentLength > 0 && (bytes_read = read(fdr, buffer, BUFFER_SIZE)) > 0)
		{
			memcpy(file->fileContents, buffer, bytes_read);
			contentLength -= bytes_read;
		}

		close(fdr);
    return file;
}

void freeFile(File *p_file)
{
	if (p_file == NULL || *p_file == NULL)
		return;

	free((*p_file)->filename);
	(*p_file)->filename = NULL;
	free((*p_file)->fileContents);
	(*p_file)->fileContents = NULL;
	free(*p_file);
	*p_file = NULL;
}

// Access functions -----------------------------------------------------------
char* getFilename(File file)
{
	if (file == NULL)
	{
		printf("Error: Calling getFilename on a NULL file reference\n");
		exit(EXIT_FAILURE);
	}

	return file->filename;
}

char* getFileContents(File file)
{
	if (file == NULL)
	{
		printf("Error: Calling getFileContents on a NULL file reference\n");
		exit(EXIT_FAILURE);
	}

	return file->fileContents;
}

time_t getFileModtime(File file)
{
	if (file == NULL)
	{
		printf("Error: Calling getFileModtime on a NULL file reference\n");
		exit(EXIT_FAILURE);
	}

	return file->modtime;
}

int getContentLength(File file)
{
	if (file == NULL)
	{
		printf("Error: calling getContentLength on a NULL file reference\n");
		exit(EXIT_FAILURE);
	}

	return file->contentLength;
}

// Mutator functions ------------------------------------------------------
void setFileModtime(File file, time_t modtime)
{
	if (file == NULL)
	{
		printf("Error: calling setFileModtime on a NULL File reference\n");
		exit(EXIT_FAILURE);
	}

	file->modtime = modtime;
}
