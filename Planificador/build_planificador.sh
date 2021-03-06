#!/bin/bash
set -e

if [[ $UID != 0  ]]; then
	echo "Please run this script with sudo:"
	echo "sudo $0 $*"
	exit 1
fi

cd ~

git clone https://github.com/sisoputnfrba/hilolay.git

cd hilolay
make
sudo make install

cd ~/tp-2019-2c-418-status-code/Planificador/src/Suse/

mkdir -p cmake-build-debug
cd cmake-build-debug
cmake ..
sudo make clean all
cd ../../..

cd src/libSuse/
mkdir -p cmake-build-debug
cd cmake-build-debug
cmake ..
sudo make install
cd ../../..
