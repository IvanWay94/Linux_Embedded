#!/bin/bash

test=`lsmod | grep encrypter`
size=${#test}
if [[ "${size}" -gt 0 ]]
then 
	echo "Removing module"
	rmmod encrypter
fi

echo "Inserting module"
#First var is number of rotations second is flag if module is enabled
insmod ./encrypter/encrypter.ko howmany=${1} ee=${2}

echo "Creating nod"
mknod /dev/encrypter_project c 60 0

echo "Starting application"
./application/bin/Release/application ${3} | tee log.txt

echo "Removing module"
rmmod encrypter

echo "Deleting nod"
rm -rf /dev/encrypter_project

exit 0

