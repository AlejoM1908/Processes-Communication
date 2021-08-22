#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/time.h>

char* generateData(int dataSize) {
    int bytes = dataSize * 1024;
    char* data = malloc(bytes);

    for (int i = 0; i<bytes; i++) data[i] = '*';

    return data;
}

void errorMessage(const char *message){
    printf(message);
    exit(1);
}

void printTimes(double* times){
    int fileSize = 1;
    bool bigger = false;

    for (int i = 0; i < 6; i++){
        if (bigger) printf("El tiempo para %dMb fue de %lf milisegundos usando tuberias\n", fileSize, times[i]);
        else printf("El tiempo para %dKb fue de %lf milisegundos usando tuberias\n", fileSize, times[i]);

        if (fileSize < 100) fileSize = fileSize * 10;
        else {
            fileSize = 1;
            bigger = true;
        }
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
        char* data;
        int dataSize = 1024 * i;

        // Get Data Package
        read(pipeRead, &data, sizeof(data));

        // Sending check
        write(pipeWrite, &check, sizeof(check));
    }
}

void parentProcess(int pipeWrite, int pipeRead){
    struct timeval start, end;
    double times [6];
    int index = 0;

    for (int i = 1; i < 100001; i = i * 10){
        char* data = generateData(i);
        gettimeofday(&start, 0);
        int dataCheck;

        // Sending Data Package to consumer
        write(pipeWrite, &data, sizeof(data));
        read(pipeRead, &dataCheck, sizeof(dataCheck));

        // Get the time elapsed time
        free(data);
        gettimeofday(&end, 0);
        times[index] = ((end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)*1e-6)*1000;
        index++;
    }

    printTimes(times);
}

void startProgram(){
    int pipePointers[2], parentPipes[2], childrenPipes[2];
    pid_t processId = 0;

    // Creating emparented processes and pipes
    pipePointers[0] = pipe(parentPipes);
    pipePointers[1] = pipe(childrenPipes);
    processId = fork();
    checkErrors(processId, pipePointers);

    // Spliting the code that witch process will execute
    if (processId == 0){
        close(childrenPipes[0]);
        close(parentPipes[1]);
        childrenProcess(childrenPipes[1], parentPipes[0]);
    }
    else if (processId > 0){
        close(childrenPipes[1]);
        close(parentPipes[0]);
        parentProcess(parentPipes[1], childrenPipes[0]);
    }
}

int main(int argc, char const *argv[]){
    startProgram();
    return 0;
}
