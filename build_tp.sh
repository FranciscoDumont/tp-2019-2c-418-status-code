#!/bin/bash
set -e

if [[ $UID != 0  ]]; then
	echo "Please run this script with sudo:"
	echo "sudo $0 $*"
	exit 1
fi

git clone https://github.com/sisoputnfrba/so-commons-library.git
cd so-commons-library
sudo make install
cd ..

sudo apt update
yes | sudo apt install cmake

#Instalacion de AltaLibreria
cd ~/tp-2019-2c-418-status-code/altaLibreria

sudo ./build_biblioteca.sh

#Instalacion del planificador
cd ~/tp-2019-2c-418-status-code/Planificador

sudo ./build_planificador.sh


#Instalacion del FileSystem
cd ~/tp-2019-2c-418-status-code/'File System'

#Insertar script de FileSystem


#Instalacion de Memoria
cd ~/tp-2019-2c-418-status-code/Memoria

sudo ./build_memoria.sh
