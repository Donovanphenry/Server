/****************************************************************************************
* Name: Donovan Henry
* CruzID: dphenry
* Assignment: PA1
* File: List.c
*****************************************************************************************/

#include "HTTPServer.h"

typedef struct HTTPServerObj
{
		int port;
		int num_fails;
		int num_calls;
		int problematic;
		int server_num_fails;
		int server_num_calls;
} HTTPServerObj;

// Constructs a new list object and returns it.
HTTPServer newHTTPServer(int port)
{
    HTTPServer server = (HTTPServer)malloc(sizeof(HTTPServerObj));
		server->port = port;
		server->num_fails = 0;
		server->num_calls = 0;
		server->problematic = 0;
		server->server_num_fails = 0;
		server->server_num_calls = 0;

    return server;
}

// frees the list held by the *pL. pL and
// *pL must both be non-NULL.
void freeHTTPServer(HTTPServer *p_server)
{
    if (p_server != NULL && *p_server != NULL)
    {
        free(*p_server);
        *p_server = NULL;
    }
}

// ------- Access functions -------
// Returns the number of elements in L.
int getPort(HTTPServer server)
{
    if (server == NULL)
    {
        printf("HTTPServer Error: calling getPort() on NULL List reference\n");
        exit(EXIT_FAILURE);
    }

    return server->port;
}

int getNumFails(HTTPServer server)
{
    if (server == NULL)
    {
        printf("HTTPServer Error: calling getNumCalls() on NULL HTTPServer reference\n");
        exit(EXIT_FAILURE);
    }

    return server->num_fails;
}

int getNumCalls(HTTPServer server)
{
	if (server == NULL)
	{
		printf("HTTPServer Error: calling getNumCalls() on NULL HTTPServer reference\n");
		exit(EXIT_FAILURE);
	}

	return server->num_calls;
}

int getProblematic(HTTPServer server)
{
	if (server == NULL)
	{
		printf("HTTPServer Error: calling isProblematic() on NULL HTTPServer reference\n");
		exit(EXIT_FAILURE);
	}

	return server->problematic;
}

int getServerNumFails(HTTPServer server)
{
	if (server == NULL)
	{
		printf("HTTPServer Error: calling getServerNumFails on a NULL HTTPServer reference\n");
		exit(EXIT_FAILURE);
	}

	return server->server_num_fails;
}

int getServerNumCalls(HTTPServer server)
{
	if (server == NULL)
	{
		printf("HTTPServer Error: calling getServerNumCalls on a NULL HTTPServer reference\n");
		exit(EXIT_FAILURE);
	}

	return server->server_num_calls;
}

// Mutators -------------------------------------------------------

void setNumFails(HTTPServer server, int num_fails)
{
	if (server == NULL)
	{
		printf("HTTPServer Error: calling setNumFails on a NULL HTTPserver reference\n");
		exit(EXIT_FAILURE);
	}

	server->num_fails = num_fails;
}

void setNumCalls(HTTPServer server, int num_calls)
{
	if (server == NULL)
	{
		printf("HTTPServer Error: calling setNumCalls on a NULL HTTPServer reference\n");
		exit(EXIT_FAILURE);
	}

	server->num_calls = num_calls;
}

void setProblematic(HTTPServer server, int problematic)
{
	if (server == NULL)
	{
		printf("HTTPServer Error: calling setProblematic on a NULL HTTPServer reference\n");
		exit(EXIT_FAILURE);
	}

	if (problematic != 0 && problematic != 1)
	{
		printf("HTTPServer Error: problematic argument is bound to 0 or 1.\n");
		exit(EXIT_FAILURE);
	}

	server->problematic = problematic;
}

void setServerNumFails(HTTPServer server, int server_num_fails)
{
	if (server == NULL)
	{
		printf("HTTPServer Error: calling setServerNumCalls on a NULL HTTPServer reference\n");
		exit(EXIT_FAILURE);
	}

	server->server_num_calls = server_num_fails;
}

void setServerNumCalls(HTTPServer server, int server_num_calls)
{
	if (server == NULL)
	{
		printf("HTTPServer Error: calling setServerNumCalls on a NULL HTTPServer reference\n");
		exit(EXIT_FAILURE);
	}

	server->server_num_calls = server_num_calls;
}
