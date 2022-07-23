#!/usr/bin/bash

port=1024
if (( "$#" == 1 )) && (( "$1" > 1023 )); then
	port="$1"
elif (( "$#" -ne 2 )); then
	echo "Warning: Port numbers less than 1024 are reserved. Defaulting to port 3010..."
elif [[ "$#" -ne 0 ]]; then
	echo "asgn0-test: Program takes up to 1 argument (port number). Exiting..."
	exit 1
fi

curl localhost:$port/fileC
curl localhost:$port/fileB
curl localhost:$port/fileA
curl localhost:$port/fileB
curl localhost:$port/fileA
curl localhost:$port/fileD
curl localhost:$port/fileD
curl localhost:$port/fileB
curl localhost:$port/fileC
curl localhost:$port/fileA
curl localhost:$port/fileB
curl localhost:$port/fileC
curl localhost:$port/fileA
curl localhost:$port/fileB
curl localhost:$port/fileC
curl localhost:$port/fileD
curl localhost:$port/fileB
curl localhost:$port/fileC
curl localhost:$port/fileD
curl localhost:$port/fileB
curl localhost:$port/fileC
curl localhost:$port/fileD
curl localhost:$port/fileA
