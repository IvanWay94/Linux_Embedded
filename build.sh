#!/bin/bash

cd ./encrypter
make clean
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf-
cd ../
cd ./application
make clean
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf-
cd ../

exit 0
