#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/time.h>
#include <sys/socket.h>
#include "../util/network_connection.c"

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
        if (bigger) printf("El tiempo para %dMb fue de %lf milisegundos usando paso de mensajes\n", fileSize, times[i]);
        else printf("El tiempo para %dKb fue de %lf milisegundos usando paso de mensajes\n", fileSize, times[i]);

        if (fileSize < 100) fileSize = fileSize * 10;
        else {
            fileSize = 1;
            bigger = true;
        }
    }
}

void checkErrors(int processId){
    // Generating the corresponding error message for the fork
    if (processId < 0) errorMessage("Error generating the fork");
}

void childrenProcess(){
    int check = 1, socket = clientConnection();

    for (int i = 0; i < 100001; i = i * 10){
        char* data;
        int dataSize = 1024 * i;

        // Get Data Package
        read(socket, &data, sizeof(data));

        // Sending check
        send(socket, &check, sizeof(check), 0);
    }
}

void parentProcess(){
    struct client_data client = serverConnection();
    struct timeval start, end;
    double times [6];
    int index = 0, average = 5;

    for (int i = 1; i < 100001; i = i * 10){
        char* data = generateData(i);
        gettimeofday(&start, 0);
        int dataCheck;

        // Sending Data Package to consumer
        send(client.clientfd, &data, sizeof(data), 0);
        read(client.clientfd, &dataCheck, sizeof(dataCheck));

        // Get the time elapsed time
        free(data);
        gettimeofday(&end, 0);
        double newTime = ((end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)*1e-6)*1000;
        if (average == 5) times[index] = newTime;
        else times[index] = (times[index] + newTime)/2;
        index++;

        // Restart sequence
        if (index == 6 && average > 0){
            average --;
            index = 0;
            i = 1;
        }
    }

    printTimes(times);
}

void startProgram(){
    pid_t processId = 0;

    // Creating emparented processes
    processId = fork();
    checkErrors(processId);

    // Spliting the code that witch process will execute
    if (processId == 0){
        childrenProcess();
    }
    else if (processId > 0){
        parentProcess();
    }
}

int main(int argc, char const *argv[]){
    startProgram();
    return 0;
}
