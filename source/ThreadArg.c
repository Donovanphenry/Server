#include "ThreadArg.h"

typedef struct ThreadArgObj
{
		List conn_queue;
		FileList cache_queue;
		int m;
		int R;
		int *curr_request;
		int s;
		HTTPServer *servers;
		pthread_mutex_t *server_locks;
		int num_servers;
} ThreadArgObj;

// Constructs a new list object and returns it.
ThreadArg newThreadArg(List conn_queue, FileList cache_queue, int m, int R, int *curr_request, int s, HTTPServer *servers, pthread_mutex_t *server_locks, int num_servers)
{
    ThreadArg ta = (ThreadArg)malloc(sizeof(ThreadArgObj));
		ta->conn_queue = conn_queue;
		ta->cache_queue = cache_queue;
		ta->m = m;
		ta->R = R;
		ta->curr_request = curr_request;
		ta->s = s;
		ta->servers = servers;
		ta->server_locks = server_locks;
		ta->num_servers = num_servers;

    return (ta);
}

// frees the list held by the *pL. pL and
// *pL must both be non-NULL.
void freeThreadArg(ThreadArg *p_ta)
{
    if (p_ta != NULL && *p_ta != NULL)
    {
			free(*p_ta);
			*p_ta = NULL;
    }
}

// Accessors

List getConnQueue(ThreadArg ta)
{
	if (ta == NULL)
	{
		printf("ThreadArg Error: Calling getConnQueue on a NULL ThreadArg reference\n");
		exit(EXIT_FAILURE);
	}

	return ta->conn_queue;
}

FileList getCacheQueue(ThreadArg ta)
{
	if (ta == NULL)
	{
		printf("ThreadArg Error: Calling getCacheQueue on a NULL ThreadArg reference\n");
		exit(EXIT_FAILURE);
	}

	return ta->cache_queue;
}

int getM(ThreadArg ta)
{
	if (ta == NULL)
	{
		printf("List Error: calling getLogOffset on NULL List reference\n");
		exit(EXIT_FAILURE);
	}

	return ta->m;
}

int getR(ThreadArg ta)
{
	if (ta == NULL)
	{
		printf("List Error: calling getFd on NULL List reference\n");
		exit(EXIT_FAILURE);
	}

	return ta->R;
}

int* getCurrRequest(ThreadArg ta)
{
	if (ta == NULL)
	{
		printf("List Error: calling getFd on NULL List reference\n");
		exit(EXIT_FAILURE);
	}

	return ta->curr_request;
}

int getS(ThreadArg ta)
{
	if (ta == NULL)
	{
		printf("List Error: calling getNumHandled on NULL List reference\n");
		exit(EXIT_FAILURE);
	}

	return ta->s;
}

HTTPServer* getServers(ThreadArg ta)
{
	if (ta == NULL)
	{
		printf("List Error: calling getNumHandled on NULL List reference\n");
		exit(EXIT_FAILURE);
	}

	return ta->servers;
}

pthread_mutex_t* getServerLocks(ThreadArg ta)
{
	if (ta == NULL)
	{
		printf("ThreadArg Error: calling getServerLocks on NULL TA reference\n");
		exit(EXIT_FAILURE);
	}
	return ta->server_locks;
}

int getNumServers(ThreadArg ta)
{
	if (ta == NULL)
	{
		printf("List Error: calling getNumHandled on NULL List reference\n");
		exit(EXIT_FAILURE);
	}

	return ta->num_servers;
}
