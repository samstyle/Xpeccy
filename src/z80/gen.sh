#!/bin/bash
PREF=${2}
TPRF=${1}
rm test.c
touch test.c
PNM=0
for NUM in {0..255}
do
	printf "void %s%.2X(Z80* p) {}\n" ${PREF} ${NUM} >> test.c
	PNM=`expr ${PNM} + 1`;
	if [ ${PNM} -eq 8 ]; then
		printf "\n" >> test.c
		PNM=0
	fi
done

printf "//==================\n\nZOp %s[256]={\n" ${TPRF} >> test.c

for NUM in {0..255}
do
	printf "\tZOp(&%s%.2X,0,\"\")" ${PREF} ${NUM} >> test.c
	if [ ${NUM} -eq 255 ]; then
		printf "\n};\n" >> test.c
	else
		printf ",\n" >> test.c
	fi
	PNM=`expr ${PNM} + 1`;
	if [ ${PNM} -eq 8 ]; then
		printf "\n" >> test.c
		PNM=0
	fi
done

