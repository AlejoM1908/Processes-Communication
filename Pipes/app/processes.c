#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#define RAND_MAX 10

char* generateData(int kbNum) {
    int bytes = 1024 * kbNum;
    char* data = malloc(bytes);

    for (int i = 0; i<bytes; i++) data[i] = (char) (rand() + 97);

    return data;
}

void errorMessage(const char *message){
    printf(message);
    exit(1);
}

void printTimes(clock_t* times){
    int fileSize = 1;
    bool bigger = false;

    for (int i = 0; i < 6; i++){
        if (bigger) printf("El tiempo para %dMb fue de %l", fileSize, times[i]);
        else printf("El tiempo para %dKb fue de %l", fileSize, times[i]);

        if (fileSize < 100) fileSize = fileSize * 10;
        else fileSize = 1;
    }
}

void checkErrors(int processId, int pipesPointers[2]){
    for (int i=0; i<2; i++){
        if (pipesPointers[i] < 0){
            // Generating the corresponding error message for pipe error
            char message[] = "Error starting the pipe ";
            strcat(message, (char*) &i);
            errorMessage(message);
        }
    }

    // Generating the corresponding error message for the fork
    if (processId < 0) errorMessage("Error generating the fork");
}

void childrenProcess(int pipeWrite, int pipeRead){
    int check = 1;

    for (int i = 0; i < 100001; i = i * 10){
        char* data = malloc(1024 * i);

        // Get Data Package
        read(pipeRead, data, sizeof(data));

        // Sending check
        write(pipeWrite, &check, sizeof(int));
        free(data);
    }
}

void parentProcess(int pipeWrite, int pipeRead){
    clock_t times [6];
    int index = 0;

    for (int i = 1; i < 100001; i = i * 10){
        char* data = generateData(i);
        clock_t startingTime = clock();
        int dataCheck;

        // Sending Data Package to consumer
        write(pipeWrite, data, sizeof(data));
        read(pipeRead, &dataCheck, sizeof(dataCheck));

        // Get the time elapsed time
        clock_t finishingTime = clock();
        times[index] = finishingTime - startingTime;
        index++;
        free(data);
    }

    printTimes(times);
}

void startProgram(){
    int pipePointers[2], parentPipes[2], childrenPipes[2];
    pid_t processId = 0;

    // Creating emparented processes and pipes
    processId = fork();
    pipePointers[0] = pipe(parentPipes);
    pipePointers[1] = pipe(childrenPipes);
    checkErrors(processId, pipePointers);

    // Splitting the code for each process
    if (processId == 0){ 
        close(childrenPipes[0]);
        close(parentPipes[1]);
        childrenProcess(childrenPipes[1], parentPipes[0]);
    } else {
        close(childrenPipes[1]);
        close(parentPipes[0]);
        parentProcess(parentPipes[1], childrenPipes[0]);
    }
}

int main(int argc, char const *argv[]){
    startProgram();
    return 0;
}
