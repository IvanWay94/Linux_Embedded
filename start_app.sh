#!/bin/bash


test=`lsmod | grep encrypter`
size=${#test}
if [[ "${size}" -gt 0 ]]
then 
	echo "Removing module"
	echo ${size}
	rmmod encrypter
	sleep 5
fi

echo "Creating nod"
echo `ls -al /dev/encrypter`
mknod /dev/encrypter c 60 0
echo `ls -al /dev/encrypter`

echo "Inserting module"
#First var is number of rotations second is flag if module is enabled
insmod encrypter.ko howmany=${1} ee=${2}



echo "Starting application"
./vezba_2c/bin/Release/vezba_2c ${3}

echo "Removing module"
rmmod encrypter

echo "Deleting nod"
rm -rf /dev/encrypter

