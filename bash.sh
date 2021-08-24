#!/bin/bash
echo ejecutando Processes communication

#ejecucion programas con archivos
cd Files
cd App
pwd

gcc processes.c -o Files
./Files
cd ..
cd ..

#ejecucion programa con paso de mensajes
cd messagePassing
cd app
pwd

gcc processes.c -o messagePassing
./messagePassing
cd ..
cd ..

#ejecucion programa con  Pipes
cd Pipes
cd app
pwd

gcc processes.c -o Pipes
./Pipes
cd ..
cd ..

#ejecucion programa con sharedMemory
cd sharedMemory
cd app
pwd

gcc processes.c -o sharedMemory
./sharedMemory
cd ..
cd ..
