#include <stdlib.h>
#include <stdio.h>

#ifndef HTTPSERVER_H
#define HTTPSERVER_H

typedef struct HTTPServerObj *HTTPServer;

// Constructors-Destructors ---------------------------------------------------
HTTPServer newHTTPServer(int port);      // Creates and returns a new empty List.
void freeHTTPServer(HTTPServer *p_server); // Frees all heap memory associated with *pL, and sets
                         // *pL to NULL.

// Access functions -----------------------------------------------------------
int getPort(HTTPServer server);
int getNumFails(HTTPServer server);
int getNumCalls(HTTPServer server);
int getProblematic(HTTPServer server);
int getServerNumFails(HTTPServer server);
int getServerNumCalls(HTTPServer server);

// Manipulation procedures ----------------------------------------------------
void setNumFails(HTTPServer server, int num_fails);          // Delete the back element. Pre: length()>0
void setProblematic(HTTPServer server, int problematic);
void setNumCalls(HTTPServer server, int num_calls);
void setServerNumFails(HTTPServer server, int server_num_fails);
void setServerNumCalls(HTTPServer server, int server_num_calls);

#endif

