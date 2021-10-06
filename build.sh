#!/bin/bash

cd ./encrypter
make clean
make
cd ../
cd ./application
make clean
make
cd ../

exit 0
