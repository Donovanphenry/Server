#!/usr/bin/bash

port=1024
if (( "$#" == 1 )) && (( "$1" > 1023 )); then
	port="$1"
elif (( "$#" == 1 )); then
	echo "Warning: Port numbers less than 1024 are reserved. Defaulting to port 3010..."
elif [[ "$#" -ne 0 ]]; then
	echo "asgn0-test: Program takes up to 1 argument (port number). Exiting..."
	exit 1
fi

check_GET_Diff () {
	i=0
	while [ $i -lt $3 ]; do
		out=$(diff $1 <(timeout 5 curl -s localhost:$port/r"$(($2 + $i))".txt))

		if [ ! "$out" = "" ]; then
			printf "command: diff $1 <(curl -s localhost:$port/r$(($2 + $i)).txt)\n"
			break
		fi
		((++i))
	done
	
	if [ $? -ne 124 ]; then
		echo $out
	else
		echo timed out
	fi
}


#################################################################################################
#### Run C Test suite written by @Burning on discord. If you do not have their test script,  ####
#### this particular suite will not execute, but I highly encourage you to get them.         ####
#### These scripts, much like the asgn0 scripts, work best in tandem!                        ####
#################################################################################################

#if [ -f "testScript.c" ] && [ "$port" = "8080" ]; then
#	echo "====Running Burning's Tests===="
#	./testScript
#fi

printf "====Cleaning up files from previous runs if they exist, and clearing log_file====\n"
NUM_IN_FILES=7
iter=$(($NUM_IN_FILES+1))
while [ $iter -le $((3*$NUM_IN_FILES)) ]; do
	if [ -f r"$iter".txt ]; then
		rm -f r"$iter".txt
	fi
	((++iter))
done
> log_file

#### Create input files ####
iter=0
if [ ! -f "r1.txt" ]; then
	touch r1.txt
	echo "Creating smaller test txt file"
	while [ $iter -le 5 ];
	do
		cat httpproxy.c >> r1.txt
		((++iter))
	done
fi

if [ ! -f "r2.txt" ]; then
	touch r2.txt
	echo "Creating smaller binary test file"
	cat httpserver >> r2.txt
fi

iter=0
if [ ! -f "r3.txt" ]; then
	touch r3.txt
	echo "Creating smaller Makefile test file"
	while [ $iter -le 5 ];
	do
		cat Makefile >> r3.txt
		((++iter))
	done
fi

if [ ! -f "r4.txt" ]; then
	touch r4.txt
	echo "Downloading larger test file"
	curl -s ftp://ccg.epfl.ch/epd/current/epd.seq | tac | tac | head -qn 40000 > r4.txt
fi

iter=0
if [ ! -f "r5.txt" ]; then
	touch r5.txt
	echo "Creating second large test file"
	while [ $iter -le 13 ]
	do
		cat httpproxy.c >> r5.txt
		cat Makefile >> r5.txt
		((++iter))
	done
fi

if [ ! -f "r6.txt" ]; then
	touch r6.txt
	out=$(which head)
	cat $out >> r6.txt
fi

if [ ! -f "r7.txt" ]; then
	touch r7.txt
	echo "Creating larger mixed test file"
	cat r4.txt >> r7.txt
	cat r6.txt >> r7.txt
	cat r5.txt >> r7.txt
fi

testCase=1

#### Call GET on each test file and diff it with actual file ####
#### Tests 1-7                                               ####
echo "====Running GET tests===="

iter=1
while [ $iter -le $NUM_IN_FILES ]; do
	FILE=r"$iter".txt
	out=$(diff <(curl -s localhost:$port/$FILE) $FILE)
	#out=$(check_GET_Diff $FILE $iter 1)

	printf "Test $testCase: "
	if [ "$out" = "" ]; then
		printf "PASS\n"
	else
		printf "FAIL. command executed: diff <(curl -s localhost:$port/$FILE) $FILE\n"
	fi

	((++iter))
	((++testCase))
done

### Check for invalid resource names, content-lengths, hosts among other things ####
#### Tests 38-50                                                                 ####
echo "====Running Bad Request tests===="
FILE1=r1.txt
FILE2=r2.txt

out=$(diff <(timeout 5 curl -s localhost:"$port"/"$FILE1" -H "Host: a a") <(echo Bad Request))
printf "Test $testCase: "

if [ "$out" = "" ]; then
	printf "PASS\n"
else
	printf "FAIL. Difference found. Command run: diff <(curl -s localhost:$port/$FILE -H \"Host: a a\") <(echo Bad Request)\n"
fi
((++testCase))

out=$(diff <(timeout 5 curl -sT $FILE1 localhost:"$port"/"$FILE2" -H "Host: a a") <(echo Bad Request))
printf "Test $testCase: "

if [ "$out" = "" ]; then
	printf "PASS\n"
else
	printf "FAIL. Difference found. Command run: diff <(curl -s -T r1.txt localhost:$port/$FILE -H \"Host: a a\") <(echo Bad Request)\n"
fi
((++testCase))

out=$(diff <(timeout 5 curl -sI localhost:"$port"/"$FILE1" -H "Host: a a")  <(printf "HTTP/1.1 400 Bad Request\r\nContent-Length: 12\r\n\r\n"))
printf "Test $testCase: "

if [ "$out" = "" ]; then
	printf "PASS\n"
else
	printf 'FAIL. Difference found. Command run: diff <(curl -sI localhost:%s/%s -H \"Host: a a\") <(printf \"HTTP/1.1 400 Bad Request\\r\\nContent-Length: 12\\r\\n\\r\\n\")\n' $port $FILE
fi
((++testCase))

out=$(diff <(curl -s localhost:$port/$FILE1 -X "TEAPOT /$FILE1 HTTP/1.1") <(printf "Not Implemented\n"))
printf "Test $testCase: "

if [ "$out" = "" ]; then
	printf "PASS\n"
else
	echo out is $out
	printf 'FAIL. Difference found. Command run: diff <(curl -s localhost:%s/%s -X \"TEAPOT /%s HTTP/1.1\") <(printf \"Not Implemented\\n\")\n' $port $FILE1 $FILE1
fi
((++testCase))

out=$(diff <(timeout 5 curl -s -T $FILE1 localhost:$port/"$FILE2" -H "Content-Length: a") <(echo Bad Request))
printf "Test $testCase: "

if [ "$out" = "" ]; then
	printf "PASS\n"
else
	printf "FAIL. Difference found. Command run: diff <(curl -s -T $FILE1 localhost:$port/$FILE2 -H \"Content-Length: a\") <(echo Bad Request)\n"
fi
((++testCase))

out=$(diff <(timeout 5 curl -sT $FILE1 localhost:"$port"/"$FILE2" -H "Content-Length: 333a") <(echo Bad Request))
printf "Test $testCase: "

if [ "$out" = "" ]; then
	printf "PASS\n"
else
	printf "FAIL. Difference found. Command run: diff <(curl -s -T $FILE1 localhost:$port/$FILE2 -H \"Content-Length: 333a\") <(echo Bad Request)\n"
fi
((++testCase))

out=$(diff <(timeout 5 curl -sT $FILE1 localhost:"$port"/"$FILE2" -H "Content-Length: a333a") <(echo Bad Request))
printf "Test $testCase: "

if [ "$out" = "" ]; then
	printf "PASS\n"
else
	printf "FAIL. Difference found. Command run: diff <(curl -s -T $FILE1 localhost:$port/$FILE2 -H \"Content-Length: a333a\") <(echo Bad Request)\n"
fi
((++testCase))

FILE=this_file_is_more_than_19_characters.txt

if [ ! -f $FILE ]; then
	touch $FILE
fi

out=$(diff <(timeout 5 curl -s localhost:"$port"/"$FILE") <(echo Bad Request))
printf "Test $testCase: "

if [ "$out" = "" ]; then
	printf "PASS\n"
else
	printf "FAIL. Difference found. Command run: diff <(curl -s localhost:$port/$FILE) <(echo Bad Request)\n"
fi
((++testCase))

out=$(diff <(timeout 5 curl -sT $FILE1 localhost:"$port"/"$FILE") <(echo Bad Request))
printf "Test $testCase: "

if [ "$out" = "" ]; then
	printf "PASS\n"
else
	printf "FAIL. Difference found. Command run: diff <(curl -sT $FILE1 localhost:$port/$FILE) <(echo Bad Request)\n"
fi
((++testCase))

out=$(diff <(timeout 5 curl -sI localhost:"$port"/"$FILE") <(printf "HTTP/1.1 400 Bad Request\r\nContent-Length: 12\r\n\r\n"))
printf "Test $testCase: "
if [ "$out" = "" ]; then
	printf "PASS\n"
else
	printf 'FAIL. Difference found. Command run: diff <(curl -sI localhost:%s/%s) <(printf \"HTTP/1.1 400 Bad Request\\r\\nContent-Length: 12\\r\\n\\r\\n\")\n' $port $FILE
fi

((++testCase))
fn="this\$@^%()"
FILE="$fn".txt

out=$(diff <(timeout 5 curl -s localhost:"$port"/"$FILE") <(echo Bad Request))
printf "Test $testCase: "

if [ "$out" = "" ]; then
	printf "PASS\n"
else
	printf "FAIL. Difference found. Command run: diff <(curl -s localhost:$port/%s.txt) <(echo Bad Request)\n" $fn
fi
((++testCase))

printf "Test $testCase: "
out=$(diff <(timeout 5 curl -sT $FILE1 localhost:"$port"/"$FILE") <(echo Bad Request))

if [ "$out" = "" ]; then
	printf "PASS\n"
else
	printf "FAIL. Difference found. Command run: diff <(curl -sT $FILE1 localhost:$port/%s.txt) <(echo Bad Request)\n" $fn
fi
((++testCase))

out=$(diff <(timeout 5 curl -sI localhost:"$port"/"$FILE") <(printf "HTTP/1.1 400 Bad Request\r\nContent-Length: 12\r\n\r\n"))
printf "Test $testCase: "

if [ "$out" = "" ]; then
	printf "PASS\n"
else
	printf 'FAIL. Difference found. Command run: diff <(curl -sI localhost:%s/%s.txt) <(printf \"HTTP/1.1 400 Bad Request\\r\\nContent-Length: 12\\r\\n\\r\\n\")\n' $port $fn
fi
((++testCase))

#### Checking for a file that does not exist ####
#### Tests 51-52                             ####
echo ====Running File Not Found tests====
FILE=non_existent.txt

printf "Test $testCase: "
out=$(diff <(timeout 5 curl -s localhost:"$port"/"$FILE") <(echo Not Found))

if [ "$out" = "" ]; then
	printf "PASS\n"
else
	printf "FAIL. diff <(curl -s localhost:$port/$FILE) <(echo Not Found)\n"
fi
((++testCase))

#### Checking on a file that you do not have access to ####
#### Tests 53-55                                       ####
echo ====Running Forbidden tests====
FILE=forbidden.txt

if [ ! -f "forbidden.txt" ]; then
	touch forbidden.txt
	chmod -rw forbidden.txt
fi

printf "Test $testCase: "
out=$(diff <(timeout 5 curl -s localhost:"$port"/"$FILE") <(echo Forbidden))

if [ "$out" = "" ]; then
	printf "PASS\n"
else
	printf "FAIL. diff <(curl -s localhost:$port/$FILE) <(echo Forbidden)\n"
fi
((++testCase))

printf "====All Done====\n"
