#define _POSIX_C_SOURCE 200809L

#include <err.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
#include <getopt.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <fcntl.h>
#include "List.h"
#include "FileList.h"
#include "HTTPServer.h"
#include "File.h"
#include "ThreadArg.h"
#include <math.h>

#define BUFFER_SIZE 10000

pthread_mutex_t connection_mutex;
pthread_mutex_t balance_mutex;
pthread_mutex_t cache_mutex;
pthread_cond_t connection_signal;

/**
   Converts a string to an 16 bits unsigned integer.
   Returns 0 if the string is malformed or out of the range.
 */
uint16_t strtouint16(char number[]) {
  char *last;
  long num = strtol(number, &last, 10);
  if (num <= 0 || num > UINT16_MAX || *last != '\0') {
    return 0;
  }
  return num;
}

int create_client_socket(uint16_t port) {
  int clientfd = socket(AF_INET, SOCK_STREAM, 0);
  if (clientfd < 0) {
    return -1;
  }

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof addr);
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(port);
  if (connect(clientfd, (struct sockaddr*) &addr, sizeof addr)) {
    return -1;
  }
  return clientfd;
}

/**
   Creates a socket for listening for connections.
   Closes the program and prints an error message on error.
 */
int create_listen_socket(uint16_t port) {
  struct sockaddr_in addr;
  int listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if (listenfd < 0) {
    err(EXIT_FAILURE, "socket error");
  }

  memset(&addr, 0, sizeof addr);
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htons(INADDR_ANY);
  addr.sin_port = htons(port);
  if (bind(listenfd, (struct sockaddr*)&addr, sizeof addr) < 0) {
    err(EXIT_FAILURE, "bind error");
  }

  if (listen(listenfd, 500) < 0) {
    err(EXIT_FAILURE, "listen error");
  }

  return listenfd;
}

int isNumber(char s[])
{
	if (s == NULL)
		return 0;

	for (int i = 0; s[i] != '\0'; i++)
	{
		int ascii = (int) s[i];

		if (ascii < 48 || ascii > 57)
			return 0;
	}

	return 1;
}

int parsingMethod(int state)
{
	if ((state >= 1 && state <= 4) || (state >= 11 && state <= 13) || (state >= 15 && state <= 17) || state == -1)
		return 1;
	return 0;
}

int parseRequestLine(char buffer[], int n, int* dataState, int *crlfState, int *mode, char **file, char **method)
{
	for (int i = 0; i < n; ++i)
	{
		char c = buffer[i];
		int ascii = (int)c;

		if (c != '\r' && c != '\n')
			*crlfState = 0;

		if (*dataState == 69 && (ascii < 65 || ascii > 90) && (ascii < 97 || ascii > 122) &&
			(ascii < 48 || ascii > 57) && c != '.' && c != '_' && c != ' ')
			*dataState = -4;

		switch(c)
		{
			case 'H':
				*dataState = *dataState == 0 ? 1 :
					*dataState == 68 || *dataState == 69 ? 69 :
					*dataState == 70 ? 71 :
					*dataState == -3 || *dataState == -4 ? -4 :
					parsingMethod(*dataState) ? -1 : *dataState;
				break;
			case 'E':
				*dataState = *dataState == 1 ? 2 :
					*dataState == 11 ? 12 :
					*dataState == 68 || *dataState == 69 ? 69 :
					*dataState == -3 || *dataState == -4 ? -4 :
					parsingMethod(*dataState) ? -1 : *dataState;
				break;
			case 'A':
				*dataState = *dataState == 2 ? 3 :
					*dataState == 68 || *dataState == 69 ? 69 :
					*dataState == -3 || *dataState == -4 ? -4 :
					parsingMethod(*dataState) ? -1 : *dataState;
				break;
			case 'D':
				*mode = *dataState == 3 ? 2 : *mode;
				*dataState = *dataState == 3 ? 4 :
					*dataState == 68 || *dataState == 69 ? 69 :
					*dataState == -3 || *dataState == -4 ? -4 :
					parsingMethod(*dataState) ? -1 : *dataState;
				break;
			case ' ':
				*dataState = *dataState == 4 || *dataState == 13 || *dataState == 17 ? 18 :
					*dataState == 69 ? 70 :
					*dataState == -3 || *dataState == -4 ? -400 :
					parsingMethod(*dataState) ? -2 : 0;
				break;
			case 'G':
				*dataState = *dataState == 0 ? 11 :
					*dataState == 68 || *dataState == 69 ? 69 :
					*dataState == -3 || *dataState == -4 ? -4 :
					parsingMethod(*dataState) ? -1 : *dataState;
				break;
			case 'T':
				*mode = *dataState == 12 ? 0 :
					*dataState == 16 ? 1 : *mode;
				*dataState = *dataState == 12 ? 13 :
					*dataState == 16 ? 17 :
					*dataState == 68 || *dataState == 69 ? 69 :
					*dataState == -3 || *dataState == -4 ? -4 :
					*dataState == 71 ? 72 :
					*dataState == 72 ? 73 :
					parsingMethod(*dataState) ? -1 : *dataState;
				break;
			case 'P':
				*dataState = *dataState == 0 ? 15 :
					*dataState == 68 || *dataState == 69 ? 69 :
					*dataState == -3 || *dataState == -4 ? -4 :
					*dataState == 73 ? 74 :
					parsingMethod(*dataState) ? -1 : *dataState;
				break;
			case 'U':
				*dataState = *dataState == 15 ? 16 :
					*dataState == 68 || *dataState == 69 ? 69 :
					*dataState == -3 || *dataState == -4 ? -4 :
					parsingMethod(*dataState) ? -1 : *dataState;
				break;
			case '/':
				if (*dataState == 18 && *file != NULL)
					free(*file);
				*dataState = *dataState == 18 ? 68 :
					*dataState == 74 ? 75 :
					*dataState == -2 ? -3 :
					*dataState == -3 || *dataState == -4 ? -4 :
					parsingMethod(*dataState) ? -1 : *dataState;
				break;
			case '1':
				*dataState = *dataState == 68 || *dataState == 69 ? 69 :
					*dataState == -3 || *dataState == -4 ? -4 :
					*dataState == 75 ? 76 :
					*dataState == 77 ? 78 :
					parsingMethod(*dataState) ? -1 : *dataState;
				break;
			case '.':
				*dataState = *dataState == 68 || *dataState == 69 ? 69 :
					*dataState == -3 || *dataState == -4 ? -4 :
					*dataState == 76 ? 77 :
					parsingMethod(*dataState) ? -1 : *dataState;
				break;
			case '\r':
				*crlfState = 1;
				break;
			case '\n':
				*crlfState = *crlfState == 1 ? 2 : 0;
				break;
			default:
				*dataState = *dataState == 68 || *dataState == 69 ? 69 :
					*dataState == -3 || *dataState == -4 ? -4 :
					parsingMethod(*dataState) ? -1 : -400;
		}

		if (*dataState == 0)
			*dataState = -1;

		if (*crlfState == 2)
		{
			*dataState = *dataState > 0 ? 0 : -400;
			return i + 1;
		}

		if (*file != NULL && strlen(*file) >= 19 && *dataState == 69)
			*dataState = -4;

		if ((*dataState == 69 || *dataState == -4) && c != '\r')
		{
			int n1 = *file != NULL ? strlen(*file) : 0;
			char* temp = malloc(n1 + 2);
			for (int k = 0; k < n1; ++k)
				temp[k] = *((*file) + k);
			temp[n1] = buffer[i];
			temp[n1 + 1] = '\0';

			if (*file)
				free(*file);
			*file = temp;
		}

		if (parsingMethod(*dataState))
		{
			int n1 = *method != NULL ? strlen(*method) : 0;
			char* temp = malloc(n1 + 2);
			for (int k = 0; k < n1; ++k)
				temp[k] = *((*method) + k);
			temp[n1] = buffer[i];
			temp[n1 + 1] = '\0';

			if (*method)
				free(*method);
			*method = temp;
		}
	}

	return n;
}

int parseHeader(char header[], int start, int n, int* dataState, int* keyState, int *validHost, char** contentLengthStr, char **host)
{
	if (*dataState == -400)
		return -400;
	for (int i = start; i < n; ++i)
	{
		char c = header[i];

		if (c != '\r' && c != '\n')
			*keyState = 0;

		if (*dataState == 34 && ((int)c < 48 || (int)c > 57) &&
			c != '\r' && c != '\n')
		{
			*dataState = -400;
			return -400;
		}

		if (*dataState == 10 && c != ' ' && c != '\r' && c != '\n')
			*validHost = 1;

		switch(c)
		{
			case 'H':
				*dataState = *dataState == 10 ? 10 :
					*dataState == 0 ? 1 : 0;
				break;
			case ' ':
				if (*dataState == 10)
				{
					*validHost = 0;
					*dataState = -400;
					return -400;
				}

				if (*dataState == 33 && *contentLengthStr != NULL)
					free(*contentLengthStr);

				*dataState = *dataState == 9 ? 10 :
					*dataState == 33 ? 34 : 0;
				break;
			case 'o':
				*dataState = *dataState == 10 ? 10:
					*dataState == 1 ? 6 :
					*dataState == 19 ? 20 : 0;
				break;
			case 's':
				*dataState = *dataState == 10 ? 10 :
					*dataState == 6 ? 7 : 0;
				break;
			case 't':
				*dataState = *dataState == 10 ? 10:
					*dataState == 7 ? 8 :
					*dataState == 21 ? 22 :
					*dataState == 24 ? 25 :
					*dataState == 30 ? 31 : 0;
				break;
			case ':':
				*dataState = *dataState == 10 ? 10:
					*dataState == 8 ? 9 :
					*dataState == 32 ? 33 : 0;
				break;
			case 'C':
				*dataState = *dataState == 10 ? 10 :
					*dataState == 0 ? 19 : 0;
				break;
			case 'n':
				*dataState = *dataState == 10 ? 10 :
					*dataState == 20 ? 21 :
					*dataState == 23 ? 24 :
					*dataState == 28 ? 29 : 0;
				break;
			case 'e':
				*dataState = *dataState == 10 ? 10 :
					*dataState == 22 ? 23 :
					*dataState == 27 ? 28 : 0;
				break;
			case '-':
				*dataState = *dataState == 10 ? 10 :
					*dataState == 25 ? 26 : 0;
				break;
			case 'L':
				*dataState = *dataState == 10 ? 10 :
					*dataState == 26 ? 27 : 0;
				break;
			case 'g':
				*dataState = *dataState == 10 ? 10 :
					*dataState == 29 ? 30 : 0;
				break;
			case 'h':
				*dataState = *dataState == 10 ? 10 :
					*dataState == 31 ? 32 : 0;
				break;
			case '\r':
				*keyState = *keyState == 0 || *keyState == 2 ? *keyState + 1 : 0;
				break;
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				*dataState = *dataState == 34 ? 34 :
					*dataState == 10 ? 10 : 0;
				break;
			case '\n':
				*keyState = *keyState == 1 || *keyState == 3 ? *keyState + 1 : 0;
				break;
			default:
				*dataState = *dataState == 10 ? 10 : 0;
		}
		
		if (*keyState == 2)
			*dataState = 0;

		if (*keyState == 4)
			return i + 1;

		if (*dataState == 34 && c != ' ' && c != '\r' && c != '\n')
		{
			int n1 = *contentLengthStr != NULL ? strlen(*contentLengthStr) : 0;
			char* temp = malloc(n1 + 2);
			for (int k = 0; k < n1; ++k)
				temp[k] = *((*contentLengthStr) + k);
			temp[n1] = header[i];
			temp[n1 + 1] = '\0';

			if (*contentLengthStr)
				free(*contentLengthStr);
			*contentLengthStr = temp;
		}

		if (*dataState == 10 && c != ' ' && c != '\r' && c != '\n')
		{
			int n1 = *host != NULL ? strlen(*host) : 0;
			char *temp = malloc(n1 + 2);
			for (int k = 0; k < n1; ++k)
				temp[k] = *((*host) + k);
			temp[n1] = header[i];
			temp[n1 + 1] = '\0';

			if (*host)
				free(*host);
			*host = temp;
		}
	}

	return n;
}

void handleGet(int server_connfd, HTTPServer server, char *file, char *host, pthread_mutex_t *server_locks, int targeted_index, char *modtime_str)
{
	int port = getPort(server);
	int client_connfd = create_client_socket(port);

	if (client_connfd < 0)
	{
		printf("client_connfd lt zero: ccfd = %d\n", client_connfd);
		pthread_mutex_lock(server_locks + targeted_index);
		int new_num_calls = getNumCalls(server) + 1;
		int new_num_fails = getNumFails(server) + 1;
		setNumCalls(server, new_num_calls);
		setNumFails(server, new_num_fails);
		setProblematic(server, 1);
		pthread_mutex_unlock(server_locks + targeted_index);
		return;
	}

	char header[BUFFER_SIZE];
	int bytes_written = snprintf(header, BUFFER_SIZE, "GET /%s HTTP/1.1\r\nHost: localhost:%d\r\n\r\n", file, getPort(server));
	send(client_connfd, header, bytes_written, MSG_NOSIGNAL);

	int bytes_recvd;
	char buffer[BUFFER_SIZE];
	char *code = NULL;
	int start = -1, loop = 1, dataState = 0;
	while (loop && (bytes_recvd = recv(client_connfd, buffer, BUFFER_SIZE, MSG_NOSIGNAL)) > 0)
	{
		for (start = 0; start < bytes_recvd; ++start)
		{
			if (buffer[start] == '\n')
			{
				++start;
				loop = 0;
				break;
			}

			switch(buffer[start])
			{
				case 'H':
					dataState = dataState == 0 ? 1 :
						dataState == 9 ? 10 : dataState;
					break;
				case 'T':
					dataState = dataState == 1 || dataState == 2 ? dataState + 1 :
						dataState == 9 ? 10 : dataState;
					break;
				case 'P':
					dataState = dataState == 3 ? 4 :
						dataState == 9 ? 10 : dataState;
					break;
				case '/':
					dataState = dataState == 4 ? 5 :
						dataState == 9 ? 10 : dataState;
					break;
				case '1':
					dataState = dataState == 5 ? 6 :
						dataState == 7 ? 8 :
						dataState == 9 ? 10 : dataState;
					break;
				case '.':
					dataState = dataState == 6 ? 7 :
						dataState == 9 ? 10 : dataState;
					break;
				case ' ':
					dataState = dataState == 8 ? 9 : -1;
					break;
				default:
					dataState = dataState == 9 ? 10 : dataState;
			}

			if (dataState == 10)
			{
				int n_code = code == NULL ? 0 : strlen(code);

				char *temp = malloc(sizeof(char) * (n_code + 2));

				for (int k = 0; k < n_code; ++k)
					temp[k] = code[k];
				temp[n_code] = buffer[start];
				temp[n_code + 1] = '\0';

				if (code)
					free(code);
				code = temp;
			}
		}
	}

	int code_int = 0;
	if (code)
	{
		code_int = atoi(code);
		free(code);
	}

	char *response = NULL;
	
	if (code_int != 200)
	{
		if (code_int == 403)
			response = "HTTP/1.1 403 Forbidden\r\nContent-Length: 10\r\n\r\nForbidden\n";
		else if (code_int == 404)
			response = "HTTP/1.1 404 Not Found\r\nContent-Length: 10\r\n\r\nNot Found\n";

		pthread_mutex_lock(server_locks + targeted_index);
		int new_num_calls = getNumCalls(server) + 1;
		int new_num_fails = getNumFails(server) + 1;
		setNumCalls(server, new_num_calls);
		setNumFails(server, new_num_fails);
		pthread_mutex_unlock(server_locks + targeted_index);
	}
	else
		response = "HTTP/1.1 200 OK\r\n";
	send(server_connfd, response, strlen(response), MSG_NOSIGNAL);

	dataState = 0;
	int keyState = 0, validHost = 0;
	char *contentLengthStr = NULL, *host_dummy = NULL;
	int term = 0, cursor = 0;
	int dataState2 = 0;
	while (start > -1)
	{
		buffer[bytes_recvd] = '\0';

		for (int i = 0; i < bytes_recvd; ++i)
		{
			char c = buffer[i];
			switch(c)
			{
				case 'L':
					dataState2 = dataState2 == -2 || dataState2 == 0 ? 1 :
						dataState2 == 15 ? 16 : dataState2;
					break;
				case 'a':
					dataState2 = dataState2 == 1 ? 2 :
						dataState2 == 15 ? 16 : dataState2;
					break;
				case 's':
					dataState2 = dataState2 == 2 ? 3 :
						dataState2 == 15 ? 16 : dataState2;
					break;
				case 't':
					dataState2 = dataState2 == 3 ? 4 :
						dataState2 == 15 ? 16 : dataState2;
					break;
				case '-':
					dataState2 = dataState2 == 4 ? 5 :
						dataState2 == 15 ? 16 : dataState2;
					break;
				case 'M':
					dataState2 = dataState2 == 5 ? 6 :
						dataState2 == 15 ? 16 : dataState2;
					break;
				case 'o':
					dataState2 = dataState2 == 6 ? 7 :
						dataState2 == 15 ? 16 : dataState2;
					break;
				case 'd':
					dataState2 = dataState2 == 7 ? 8 :
						dataState2 == 12 ? 13 :
						dataState2 == 15 ? 16 : dataState2;
					break;
				case 'i':
					dataState2 = dataState2 == 8 ? 9 :
						dataState2 == 10 ? 11 :
						dataState2 == 15 ? 16 : dataState2;
					break;
				case 'f':
					dataState2 = dataState2 == 9 ? 10 :
						dataState2 == 15 ? 16 : dataState2;
					break;
				case 'e':
					dataState2 = dataState2 == 11 ? 12 :
						dataState2 == 15 ? 16 : dataState2;
					break;
				case ':':
					dataState2 = dataState2 == 13 ? 14 :
						dataState2 == 15 ? 16 : dataState2;
					break;
				case ' ':
					dataState2 = dataState2 == 14 ? 15 :
						dataState2 == 15 ? 16 : dataState2;
					break;
				case '\r':
					dataState2 = dataState2 == -2 ? -3 : -1;
					break;
				case '\n':
					dataState2 = dataState2 == -1 ? -2 : -4;
					break;
				default:
					dataState2 = dataState2 == 15 || dataState2 == 16 ? 16 : 0;
			}

			if (dataState2 == -4)
				break;

			if (dataState2 == 16)
			{
				modtime_str[cursor + 1] = '\0';
				modtime_str[cursor++] = buffer[i];
			}

			if (cursor > 30)
				dataState2 = -4;
		}

		term = parseHeader(buffer, start, bytes_recvd, &dataState, &keyState, &validHost, &contentLengthStr, &host_dummy);

		if (dataState == -400 || dataState2 == -400)
			break;
		send(server_connfd, buffer + start, term - start, MSG_NOSIGNAL);
		if (keyState == 4 || dataState2 == -4 || (bytes_recvd = recv(client_connfd, header, BUFFER_SIZE, 0)) < 0)
			break;
		start = 0;
	}

	modtime_str[30] = '\0';

	start = term;
	for (int i = 0, k = start; k < bytes_recvd; ++k, ++i)
		buffer[i] = buffer[k];
	bytes_recvd -= start;
	buffer[bytes_recvd] = '\0';
	int contentLength = atoi(contentLengthStr);

	while (1) 
	{
		send(server_connfd, buffer, bytes_recvd, MSG_NOSIGNAL);
		contentLength -= bytes_recvd;

		if (contentLength <= 0 || (bytes_recvd = recv(client_connfd, buffer, BUFFER_SIZE, 0)) <= 0)
			break;
	}

	if (contentLengthStr)
		free(contentLengthStr);
	if (host_dummy)
		free(host_dummy);

	close(client_connfd);
	//mutex
	pthread_mutex_lock(server_locks + targeted_index);
	int new_num_calls = getNumCalls(server) + 1;
	setNumCalls(server, new_num_calls);
	pthread_mutex_unlock(server_locks + targeted_index);
	//un-mutex
}

int partition(HTTPServer **servers, int p, int r)
{
	int i = p - 1;

	int prob_r = getProblematic((*servers)[r]);
	int numCalls_r = getNumCalls((*servers)[r]);
	int numFails_r = getNumFails((*servers)[r]);

	for (int j = p; j < r; ++j)
	{
		int prob_j = getProblematic((*servers)[j]);
		int numCalls_j = getNumCalls((*servers)[j]);
		int numFails_j = getNumFails((*servers)[j]);
		
		if (prob_j < prob_r || (prob_j == prob_r && numCalls_j < numCalls_r) ||
			(prob_j == prob_r && numCalls_j == numCalls_r && numFails_j < numFails_r))
		{
			++i;
			HTTPServer temp = (*servers)[i];
			(*servers)[i] = (*servers)[j];
			(*servers)[j] = temp;
		}
	}

	HTTPServer temp = (*servers)[i + 1];
	(*servers)[i + 1] = (*servers)[r];
	(*servers)[r] = temp;

	return i + 1;
}

void qSort(HTTPServer **servers, int p, int r)
{
	if (p < r)
	{
		int q = partition(servers, p, r);
		qSort(servers, p, q - 1);
		qSort(servers, q + 1, r);
	}
}

void balance_load(HTTPServer **servers, pthread_mutex_t *server_locks, int num_servers)
{
	for (int i = 0; i < num_servers; ++i)
	{
		int port = getPort((*servers)[i]);
		int client_connfd = create_client_socket(port);
		
		if (client_connfd < 0)
		{
			printf("client_connfd lt zero: ccfd = %d on port %d\n", client_connfd, port);
			pthread_mutex_lock(server_locks + i);
			setProblematic((*servers)[i], 1);
			pthread_mutex_unlock(server_locks + i);
			continue;
		}
		else
		{
			pthread_mutex_lock(server_locks + i);
			setProblematic((*servers)[i], 0);
			pthread_mutex_unlock(server_locks + i);
		}

		char header[BUFFER_SIZE];
		int bytes_written = snprintf(header, BUFFER_SIZE, "GET /healthcheck HTTP/1.1\r\nHost: localhost:%d\r\n\r\n", getPort((*servers)[i]));
		header[bytes_written] = '\0';
		send(client_connfd, header, bytes_written, MSG_NOSIGNAL);

		char *code = NULL;
		int bytes_recvd;
		char buffer[BUFFER_SIZE];
		int start = -1, loop = 1, dataState = 0;
		while (loop && (bytes_recvd = recv(client_connfd, buffer, BUFFER_SIZE, MSG_NOSIGNAL)) > 0)
		{
			for (start = 0; start < bytes_recvd; ++start)
			{
				if (buffer[start] == '\n')
				{
					++start;
					loop = 0;
					break;
				}

				switch(buffer[start])
				{
					case 'H':
						dataState = dataState == 0 ? 1 :
							dataState == 9 ? 10 : dataState;
						break;
					case 'T':
						dataState = dataState == 1 || dataState == 2 ? dataState + 1 :
							dataState == 9 ? 10 : dataState;
						break;
					case 'P':
						dataState = dataState == 3 ? 4 :
							dataState == 9 ? 10 : dataState;
						break;
					case '/':
						dataState = dataState == 4 ? 5 :
							dataState == 9 ? 10 : dataState;
						break;
					case '1':
						dataState = dataState == 5 ? 6 :
							dataState == 7 ? 8 :
							dataState == 9 ? 10 : dataState;
						break;
					case '.':
						dataState = dataState == 6 ? 7 :
							dataState == 9 ? 10 : dataState;
						break;
					case ' ':
						dataState = dataState == 8 ? 9 : -1;
						break;
					default:
						dataState = dataState == 9 ? 10 : dataState;
				}

				if (dataState == 10)
				{
					int n_code = code == NULL ? 0 : strlen(code);

					char *temp = malloc(sizeof(char) * (n_code + 2));

					for (int k = 0; k < n_code; ++k)
						temp[k] = code[k];
					temp[n_code] = buffer[start];
					temp[n_code + 1] = '\0';

					if (code)
						free(code);
					code = temp;
				}
			}
		}

		int code_int = 0;
		if (code)
		{
			code_int = atoi(code);
			free(code);
		}

		if (code_int != 200)
		{
			pthread_mutex_lock(server_locks + i);
			setProblematic((*servers)[i], 1);
			pthread_mutex_unlock(server_locks + i);
			close(client_connfd);
			continue;
		}
		else
		{
			pthread_mutex_lock(server_locks + i);
			setProblematic((*servers)[i], 0);
			pthread_mutex_unlock(server_locks + i);
		}

		dataState = 0;
		int keyState = 0, validHost = 0;
		char *contentLengthStr = NULL, *host_dummy = NULL;
		int term = 0;
		while (start > -1)
		{
			buffer[bytes_recvd] = '\0';
			term = parseHeader(buffer, start, bytes_recvd, &dataState, &keyState, &validHost, &contentLengthStr, &host_dummy);

			if (dataState == -400 || keyState == 4 || (bytes_recvd = recv(client_connfd, header, BUFFER_SIZE, 0)) < 0)
				break;
			start = 0;
		}

		start = term;
		for (int j = 0, k = start; k < bytes_recvd; ++k, ++j)
			buffer[j] = buffer[k];
		bytes_recvd -= start;
		buffer[bytes_recvd] = '\0';
		int contentLength = atoi(contentLengthStr);

		while (1) 
		{
			contentLength -= bytes_recvd;

			if (contentLength <= 0 || (bytes_recvd = recv(client_connfd, buffer, BUFFER_SIZE, 0)) <= 0)
				break;
			buffer[bytes_recvd] = '\0';
		}

		int num_fails, num_calls;
		dataState = 0;

		for (int k = 0; k < strlen(buffer); ++k)
		{
			int ascii = (int)buffer[k];
			if (buffer[k] == '\n')
				++dataState;
			else if ((dataState == 0 || dataState == 1) && (ascii < 48 || ascii > 57))
			{
				dataState = 3;
				break;
			}
		}

		if (dataState != 2)
			setProblematic((*servers)[i], 1);
		else
		{
			pthread_mutex_lock(server_locks + i);
			sscanf(buffer, "%d\n%d\n", &num_fails, &num_calls);
			setNumFails((*servers)[i], num_fails);
			setNumCalls((*servers)[i], num_calls);
			setProblematic((*servers)[i], 0);
			pthread_mutex_unlock(server_locks + i);
		}

		if (contentLengthStr)
			free(contentLengthStr);
		if (host_dummy)
			free(host_dummy);

		close(client_connfd);
	}
}

long headRequestModtime(char *filename, int port, pthread_mutex_t *server_lock, HTTPServer server)
{
	int client_connfd = create_client_socket(port);

	if (client_connfd < 0)
	{
		printf("client_connfd lt zero: ccfd = %d\n", client_connfd);
		return 0;
	}

	pthread_mutex_lock(server_lock);
	int new_num_calls = getNumCalls(server) + 1;
	setNumCalls(server, new_num_calls);
	pthread_mutex_unlock(server_lock);

	char *code_str = NULL;
	char header[BUFFER_SIZE];
	int bytes_written = snprintf(header, BUFFER_SIZE, "HEAD /%s HTTP/1.1\r\nHost: localhost:%d\r\n\r\n", filename, port);
	send(client_connfd, header, bytes_written, MSG_NOSIGNAL);

	int bytes_recvd;
	char buffer[BUFFER_SIZE];
	int start = -1, loop = 1, dataState = 0;
	while (loop && (bytes_recvd = recv(client_connfd, buffer, BUFFER_SIZE, MSG_NOSIGNAL)) > 0)
	{
		for (start = 0; start < bytes_recvd; ++start)
		{
			if (buffer[start] == '\n')
			{
				++start;
				loop = 0;
				break;
			}

			switch(buffer[start])
			{
				case 'H':
					dataState = dataState == 0 ? 1 :
						dataState == 9 ? 10 : dataState;
					break;
				case 'T':
					dataState = dataState == 1 || dataState == 2 ? dataState + 1 :
						dataState == 9 ? 10 : dataState;
					break;
				case 'P':
					dataState = dataState == 3 ? 4 :
						dataState == 9 ? 10 : dataState;
					break;
				case '/':
					dataState = dataState == 4 ? 5 :
						dataState == 9 ? 10 : dataState;
					break;
				case '1':
					dataState = dataState == 5 ? 6 :
						dataState == 7 ? 8 :
						dataState == 9 ? 10 : dataState;
					break;
				case '.':
					dataState = dataState == 6 ? 7 :
						dataState == 9 ? 10 : dataState;
					break;
				case ' ':
					dataState = dataState == 8 ? 9 : -1;
					break;
				default:
					dataState = dataState == 9 ? 10 : dataState;
			}

			if (dataState == 10)
			{
				int n_code = code_str == NULL ? 0 : strlen(code_str);

				char *temp = malloc(sizeof(char) * (n_code + 2));

				for (int k = 0; k < n_code; ++k)
					temp[k] = code_str[k];
				temp[n_code] = buffer[start];
				temp[n_code + 1] = '\0';

				if (code_str)
					free(code_str);
				code_str = temp;
			}
		}
	}


	dataState = 0;
	char modtime_str[30];
	int cursor = 0;

	while (start > -1)
	{
		buffer[bytes_recvd] = '\0';

		for (int i = 0; i < bytes_recvd; ++i)
		{
			char c = buffer[i];
			switch(c)
			{
				case 'L':
					dataState = dataState == -2 || dataState == 0 ? 1 :
						dataState == 15 ? 16 : dataState;
					break;
				case 'a':
					dataState = dataState == 1 ? 2 :
						dataState == 15 ? 16 : dataState;
					break;
				case 's':
					dataState = dataState == 2 ? 3 :
						dataState == 15 ? 16 : dataState;
					break;
				case 't':
					dataState = dataState == 3 ? 4 :
						dataState == 15 ? 16 : dataState;
					break;
				case '-':
					dataState = dataState == 4 ? 5 :
						dataState == 15 ? 16 : dataState;
					break;
				case 'M':
					dataState = dataState == 5 ? 6 :
						dataState == 15 ? 16 : dataState;
					break;
				case 'o':
					dataState = dataState == 6 ? 7 :
						dataState == 15 ? 16 : dataState;
					break;
				case 'd':
					dataState = dataState == 7 ? 8 :
						dataState == 12 ? 13 :
						dataState == 15 ? 16 : dataState;
					break;
				case 'i':
					dataState = dataState == 8 ? 9 :
						dataState == 10 ? 11 :
						dataState == 15 ? 16 : dataState;
					break;
				case 'f':
					dataState = dataState == 9 ? 10 :
						dataState == 15 ? 16 : dataState;
					break;
				case 'e':
					dataState = dataState == 11 ? 12 :
						dataState == 15 ? 16 : dataState;
					break;
				case ':':
					dataState = dataState == 13 ? 14 :
						dataState == 15 ? 16 : dataState;
					break;
				case ' ':
					dataState = dataState == 14 ? 15 :
						dataState == 15 ? 16 : dataState;
					break;
				case '\r':
					dataState = dataState == -2 ? -3 : -1;
					break;
				case '\n':
					dataState = dataState == -1 ? -2 : -4;
					break;
				default:
					dataState = dataState == 15 || dataState == 16 ? 16 : 0;
			}

			if (dataState == -4)
				break;

			if (dataState == 16)
			{
				modtime_str[cursor + 1] = '\0';
				modtime_str[cursor++] = buffer[i];
			}

			if (cursor > 30)
				dataState = -4;
		}

		if (dataState == -4 || (bytes_recvd = recv(client_connfd, header, BUFFER_SIZE, 0)) < 0)
			break;
		start = 0;
	}

	int code = 0;
	if (code_str)
	{
		code = atoi(code_str);
		free(code_str);
	}

	modtime_str[30] = '\0';
	struct tm time_obj = {0};
	strptime(modtime_str, "%a, %d %b %Y %H:%M:%S", &time_obj);
	time_t modtime = mktime(&time_obj);

	close(client_connfd);
	
	if (code == 200)
		return modtime;
	
	pthread_mutex_lock(server_lock);
	int new_num_fails = getNumFails(server) + 1;
	setNumFails(server, new_num_fails);
	pthread_mutex_unlock(server_lock);
	return -1 * code;
}

HTTPServer findServer(HTTPServer *servers, int port_num, int num_servers)
{
	for (int i = 0; i < num_servers; ++i)
	{
		if (getPort(servers[i]) == port_num)
			return servers[i];
	}

	return NULL;
}

int targetServer(HTTPServer *servers, int num_servers)
{
	int min_index = -1, min_errs = INT_MAX, min_calls = INT_MAX;
	for (int i = 0; i < num_servers; ++i)
	{
		int num_calls_i = getNumCalls(servers[i]), num_errs_i = getNumFails(servers[i]), problematic_i = getProblematic(servers[i]);
		if (!problematic_i && (num_calls_i < min_calls || (num_calls_i == min_calls && num_errs_i < min_errs)))
		{
			min_index = i;
			min_calls = num_calls_i;
			min_errs = num_errs_i;
		}
	}

	return min_index;
}

void* handle_connection(int *p_connfd, FileList cache_queue, int m, int R, int *curr_request, int s, HTTPServer **servers, pthread_mutex_t *server_locks, int num_servers) {
	int connfd = *((int*)p_connfd);
	free(p_connfd);
	int targeted_index = targetServer(*servers, num_servers);
	if (targeted_index == -1)
		return NULL;
	HTTPServer target_server = (*servers)[targeted_index];
	int target_port = getPort(target_server);

	/*
	printf("handling_connection called: servers = [");
	for (int j = 0; j < num_servers; ++j)
		printf("(%d, %d, %d), ", getPort((*servers)[j]), getNumCalls((*servers)[j]), getProblematic((*servers)[j]));
	printf("end]\n");
	printf("targeted server: %d\n\n", target_port);*/

	char header[BUFFER_SIZE];
	int mode = 3, validHost = 0;
	char* filename = NULL, *contentLengthStr = NULL, *host = NULL, *method = NULL;
	int nullIndex, term = 0, dataState = 0, keyState = 0, new_num_calls = 0, new_num_fails = 0;

	// Parse the request line
	while(dataState != -400 && keyState != 2 && (nullIndex = recv(connfd, header, BUFFER_SIZE, 0)) > 0)
		term = parseRequestLine(header, nullIndex, &dataState, &keyState, &mode, &filename, &method);

	// Parse the header fields
	while(1)
	{
		parseHeader(header, term, nullIndex, &dataState, &keyState, &validHost, &contentLengthStr, &host);
		if (keyState == 4 || dataState == -400 || (nullIndex = recv(connfd, header, BUFFER_SIZE, 0)) < 0)
			break;
		term = 0;
	}

	char* dummy;
	if (contentLengthStr != NULL && !isNumber(contentLengthStr))
		free(contentLengthStr);
	long contentLength = contentLengthStr == NULL ? 0 : strtol(contentLengthStr, &dummy, 10);
	if ((contentLengthStr == NULL && mode == 1) || contentLength == LONG_MIN || contentLength == LONG_MAX ||
		validHost == 0 || dataState == -400 || filename == NULL || host == NULL)
	{
		char *res = mode == 3 ? "HTTP/1.1 501 Not Implemented\r\nContent-Length: 16\r\n\r\nNot Implemented\n" : 
			"HTTP/1.1 400 Bad Request\r\nContent-Length: 12\r\n\r\nBad Request\n";

		send(connfd, res, strlen(res), MSG_NOSIGNAL);
		if (filename != NULL)
			free(filename);
		if (contentLengthStr != NULL)
			free(contentLengthStr);
		if (host != NULL)
			free(host);
		if (method != NULL)
			free(method);
		close(connfd);
		return NULL;
	}

	char* res;
	int n;
	long server_modtime = 0, cache_modtime = 0;

	char modtime_str[30];
	File file = NULL;
	switch(mode)
	{
		case 0:
			file = containsFile(cache_queue, filename);

			if (file)
			{
				cache_modtime = getFileModtime(file);
				//printf("'%s' is in the cache. Cache contents: ", filename);
				server_modtime = headRequestModtime(filename, target_port, server_locks + targeted_index, target_server);
				//printf("server_modtime is %ld\n", server_modtime);

				if (server_modtime < 0)
				{
					if (server_modtime == -403)
						res = "HTTP/1.1 403 Forbidden\r\nContent-Length: 10\r\n\r\nForbidden\n";
					else if (server_modtime == -404)
						res = "HTTP/1.1 404 Not Found\r\nContent-Length: 10\r\n\r\nNot Found\n";
					else
						res = "HTTP/1.1 501 Not Implemented\r\nContent-Length: 16\r\n\r\nNot Implemented\n";

					send(connfd, res, strlen(res), MSG_NOSIGNAL);
					break;
				}

				if (server_modtime <= cache_modtime)
				{
					int file_contentLength = getContentLength(file);
					n = snprintf(NULL, 0, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\n", file_contentLength);
					res = malloc(sizeof(char)*(n + 1));
					snprintf(res, n + 1, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\n", file_contentLength);

					send(connfd, res, strlen(res), MSG_NOSIGNAL);
					send(connfd, getFileContents(file), file_contentLength, MSG_NOSIGNAL);

					if (res)
						free(res);

					break;
				}
			}
		
			handleGet(connfd, target_server, filename, host, server_locks, targeted_index, modtime_str);
			struct tm time_obj = {0};
			strptime(modtime_str, "%a, %d %b %Y %H:%M:%S", &time_obj);
			server_modtime = mktime(&time_obj);

			file = newFile(filename, m, server_modtime);
			if (file != NULL)
			{
				//printf("'%s' does not have a valid copy in the cache. Cache contents after adding it with server_modtime %ld: ", filename, server_modtime);

				pthread_mutex_lock(&cache_mutex);
				if (queueLength(cache_queue) == s)
					dequeue(cache_queue);
				enqueue(cache_queue, file);
				pthread_mutex_unlock(&cache_mutex);
			}

			//printFiles(cache_queue);
			//printf("\n");
			break;
		default:
			pthread_mutex_lock(server_locks + targeted_index);
			new_num_calls = getNumCalls(target_server) + 1;
			new_num_fails = getNumFails(target_server) + 1;
			setNumCalls(target_server, new_num_calls);
			setNumFails(target_server, new_num_fails);
			pthread_mutex_unlock(server_locks + targeted_index);
			res = "HTTP/1.1 501 Not Implemented\r\nContent-Length: 16\r\n\r\nNot Implemented\n";
			send(connfd, res, strlen(res), MSG_NOSIGNAL);
			if (filename != NULL)
					free(filename);
			break;
	}

	pthread_mutex_lock(&balance_mutex);
	*curr_request += 1;
	if (*curr_request % R == 0)
	{
		balance_load(servers, server_locks, num_servers);
		*curr_request = 0;
	}
	pthread_mutex_unlock(&balance_mutex);

	
	if (contentLengthStr != NULL)
		free(contentLengthStr);

	if (host != NULL)
		free(host);

	if (method != NULL)
		free(method);

	close(connfd);
	return NULL;
}

void* thread_function(void* p_ta)
{
	ThreadArg ta = *((ThreadArg*)p_ta);
	List conn_queue = getConnQueue(ta);

	while (1)
	{
		int* p_connfd;
		pthread_mutex_lock(&connection_mutex);
		if ((p_connfd = deleteFront(conn_queue)) == NULL)
		{
			pthread_cond_wait(&connection_signal, &connection_mutex); // Mutex as an arg to release lock while thread is suspended
			p_connfd = deleteFront(conn_queue);
		}
		pthread_mutex_unlock(&connection_mutex);

		if (p_connfd != NULL)
		{
			HTTPServer *servers = getServers(ta);
			handle_connection(p_connfd, getCacheQueue(ta), getM(ta), getR(ta), getCurrRequest(ta), getS(ta), &servers, getServerLocks(ta), getNumServers(ta));
		}
	}
}

int main(int argc, char *argv[]) {
  int listenfd;
  uint16_t port = 0;
	int nthreads = 5, m = 1024, s = 3, R = 5;
	int curr_args = 0;
	int *ports = NULL, num_servers = 0;

	int option;

	while (curr_args < argc)
	{
		option = getopt(argc, argv, "N:m:s:R:");
		switch (option)
		{
			case 'N':
				if (isNumber(optarg) == 0)
					errx(EXIT_FAILURE, "invalid number of threads: %s", optarg);
				nthreads = atoi(optarg);
				break;
			case 'm':
				if (isNumber(optarg) == 0)
					errx(EXIT_FAILURE, "invalid max number of bytes to be cached: %s", optarg);
				m = atoi(optarg);
				break;
			case 's':
				if (isNumber(optarg) == 0)
					errx(EXIT_FAILURE, "invalid number of files to be cached: %s", optarg);
				s = atoi(optarg);
				break;
			case 'R':
				if (isNumber(optarg) == 0)
					errx(EXIT_FAILURE, "invalid healthcheck frequency: %s", optarg);
				R = atoi(optarg);
				break;
			default:
				if (optind < argc && isNumber(argv[optind]) && atoi(argv[optind]) > 0 && atoi(argv[optind]) < 65536)
				{
					int *temp = malloc(sizeof(int) * (num_servers + 1));

					for (int i = 0; i < num_servers; ++i)
						temp[i] = ports[i];
					if (ports != NULL)
						free(ports);
					ports = temp;

					ports[num_servers] = atoi(argv[optind]);
					++num_servers;
					optind++;
				}
				else if (optind < argc)
					errx(EXIT_FAILURE, "invalid port number: %s", argv[optind]);
		}
		++curr_args;
	}

  if (argc < 2) {
    errx(EXIT_FAILURE, "wrong arguments: %d", argc);
  }
  if (num_servers <= 1) {
    errx(EXIT_FAILURE, "invalid number of HTTP servers: %d", num_servers);
  }

	--num_servers;
	pthread_mutex_t *server_locks = malloc(sizeof(pthread_mutex_t) * num_servers);
	for (int i = 0; i < num_servers; ++i)
		pthread_mutex_init(server_locks + i, NULL);

	HTTPServer *servers = malloc(sizeof(HTTPServer) * num_servers);
	for (int i = 0; i < num_servers; ++i)
		servers[i] = newHTTPServer(ports[i + 1]);

	int curr_request = 0;
	List conn_queue = newList();
	FileList cache_queue = newFileList();
	ThreadArg ta = newThreadArg(conn_queue, cache_queue, m, R, &curr_request, s, servers, server_locks, num_servers);
	pthread_t* thread_pool = malloc(sizeof(pthread_t) * nthreads);
	pthread_mutex_init(&connection_mutex, NULL);
	pthread_mutex_init(&balance_mutex, NULL);
	pthread_mutex_init(&cache_mutex, NULL);
	pthread_cond_init(&connection_signal, NULL);

	balance_load(&servers, server_locks, num_servers);

	for (int i = 0; i < nthreads; ++i)
		pthread_create(thread_pool + i, NULL, thread_function, &ta);

  port = ports[0];
  listenfd = create_listen_socket(port);

  while(1) {
    int connfd = accept(listenfd, NULL, NULL);
    if (connfd < 0) {
      warn("accept error");
      continue;
    }


		int *p_connfd = malloc(sizeof(int));
		*p_connfd = connfd;

		pthread_mutex_lock(&connection_mutex);
		append(conn_queue, p_connfd);
		pthread_cond_signal(&connection_signal);
		pthread_mutex_unlock(&connection_mutex);
  }

	free(ports);
	freeList(&conn_queue);
	freeFileList(&cache_queue);
	free(thread_pool);
	free(server_locks);
	for (int i = 0; i < num_servers; ++i)
		freeHTTPServer(servers + i);
	free(servers);
  return EXIT_SUCCESS;
}
