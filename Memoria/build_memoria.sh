#!/bin/bash
set -e

if [[ $UID != 0  ]]; then
	echo "Please run this script with sudo:"
	echo "sudo $0 $*"
	exit 1
fi

cd src/Muse/
mkdir -p cmake-build-debug
cd cmake-build-debug
cmake ..
sudo make clean all
cd ../../..

cd src/libMuse/
mkdir -p cmake-build-debug
cd cmake-build-debug
cmake ..
sudo make clean install
cd ../../..

cd test
mkdir -p cmake-build-debug
cd cmake-build-debug
cmake ..
sudo make clean all
cd ../..
