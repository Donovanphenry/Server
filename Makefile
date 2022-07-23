httpproxy	: httpproxy.o List.o HTTPServer.o File.o ThreadArg.o FileList.o
	gcc -std=c11 -Wall -Wshadow -Wpedantic -o httpproxy httpproxy.o List.o HTTPServer.o File.o ThreadArg.o FileList.o -lpthread

httpproxy.o	: httpproxy.c
	gcc -std=c11 -Wall -Wshadow -Wpedantic -c httpproxy.c -lpthread

List.o	: List.h List.c
	gcc -std=c11 -Wall -Wshadow -Wpedantic -c List.c

HTTPServer.o	: HTTPServer.h HTTPServer.c
	gcc -std=c11 -Wall -Wshadow -Wpedantic -c HTTPServer.c

File.o	: File.h File.c
	gcc -std=c11 -Wall -Wpedantic -Wshadow -c File.c

ThreadArg.o	: ThreadArg.h ThreadArg.c
	gcc -std=c11 -Wall -Wpedantic -Wshadow -c ThreadArg.c

FileList.o	: FileList.h FileList.c
	gcc -std=c11 -Wall -Wshadow -Wpedantic -c FileList.c
