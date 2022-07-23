#big thanks to Donovan#7689, who created the file generation code

make build

if [ ! -f "r0.txt" ]; then
	echo "Creating very small txt file"
	cat Makefile >> r0.txt
fi

iter=0
if [ ! -f "r1.txt" ]; then
	echo "Creating smaller test txt file"
	while [ $iter -le 5 ];
	do
		cat httpproxy.c >> r1.txt
		((++iter))
	done
fi

if [ ! -f "r2.txt" ]; then
	echo "Creating smaller binary test file"
	cat httpproxy >> r2.txt
fi

iter=0
if [ ! -f "r3.txt" ]; then
	echo "Creating smaller Makefile test file"
	while [ $iter -le 5 ];
	do
		cat Makefile >> r3.txt
		((++iter))
	done
fi

if [ ! -f "r4.txt" ]; then
	echo "Downloading larger test file"
	curl -s ftp://ccg.epfl.ch/epd/current/epd.seq | tac | tac | head -qn 40000 > r4.txt
fi

iter=0
if [ ! -f "r5.txt" ]; then
	echo "Creating second large test file"
	while [ $iter -le 13 ]
	do
		cat httpproxy.c >> r5.txt
		cat Makefile >> r5.txt
		((++iter))
	done
fi

if [ ! -f "r6.txt" ]; then
	out=$(which head)
	cat $out >> r6.txt
fi

if [ ! -f "r7.txt" ]; then
	echo "Creating larger mixed test file"
	cat r4.txt >> r7.txt
	cat r6.txt >> r7.txt
	cat r5.txt >> r7.txt
fi