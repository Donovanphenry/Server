#!/usr/bin/bash

port=8080
 if (( "$#" == 1 )) && (( "$1" > 1023 )); then
   port="$1"
 elif (( "$#" == 1 )); then
   echo "Warning: Port numbers less than 1024 are reserved. Defaulting to port 8080..."
 elif [[ "$#" -ne 0 ]]; then
   echo "asgn0-test: Program takes up to 1 argument (port number). Exiting..."
   exit 1
fi

for i in {1..100}
do
	#diff small <(curl -s localhost:$port/small) &
	diff r"$((($i%7) + 1))".txt <(curl -s localhost:$port/r"$(((i%7) + 1))".txt) &
done
wait

