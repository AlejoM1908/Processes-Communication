#include <unistd.h>
#include <stdio.h>

void errorMessage(const char *message){
    printf(message);
    exit(1);
}

void childrenProcess(){

}

void parentProcess(){

}

void startProgram(){
    pid_t processId = 0;

    // Creating emparented processes
    processId = fork();
    if (processId < 0) errorMessage("Error en el fork del proceso");

    // Splitting the code for each process
    if (processId == 0) childrenProcess();
    else parentProcess();
}

int main(int argc, char const *argv[]){
    startProgram();
    return 0;
}
