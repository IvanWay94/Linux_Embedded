#!/bin/bash

if [ $# -lt 2 ]
then
	echo "Invalid number of arguments"
	echo "Script arguments must be:"
	echo "   1) \"test\" or number of data for default application mode"
	echo "   2) flag (1/0) - encryption enable or not"
	echo "   3) optional, number of rotation if encryption enabled"
	exit 1
elif [ "$#" -eq 2 ] && [ $2 -eq 1 ]
then
	echo "Number of rotation will be by default 1"
fi

test=`lsmod | grep encrypter`
size=${#test}
if [[ "${size}" -gt 0 ]]
then 
	echo "Removing module"
	rmmod encrypter
fi

echo "Inserting module"
#First var is number of rotations second is flag if encryption is enabled
insmod ./encrypter/encrypter.ko ee=$2 howmany=${3:-1}

echo "Creating nod"
mknod /dev/encrypter_project c 60 0

echo "Starting application"
./application/bin/Release/application $1 | tee log.txt

echo "Removing module"
rmmod encrypter

echo "Deleting nod"
rm -rf /dev/encrypter_project

exit 0

