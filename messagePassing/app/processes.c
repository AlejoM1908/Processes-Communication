#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <stdarg.h>
#include <errno.h>
#include <math.h>
#include "../util/network_connection.c"

void errorMessage(const char *message, ...){
    va_list args;
    va_start(args, message);
    vfprintf(stderr, message, args);
    va_end(args);
    exit(1);
}

char *generateData(int kbNum){
    int bytes = 1000 * kbNum;
    char *data = malloc(bytes);

    if (data == NULL)
        errorMessage("failed to allocate %d bytes of memory\n", bytes);

    memset(data, '*', bytes);

    return data;
}

ssize_t multi_read(int socket, char *buffer, size_t nbytes){
    ssize_t nb = 0;
    size_t nleft = nbytes;
    ssize_t tbytes = 0;
    while (nleft > 0 && (nb = read(socket, buffer, nleft)) > 0)
    {
        tbytes += nb;
        buffer += nb;
        nleft  -= nb;
    }
    if (tbytes == 0)
        tbytes = nb;
    return tbytes;
}

ssize_t multi_send(int client, const char *buffer, size_t nbytes){
    ssize_t nb = 0;
    size_t nleft = nbytes;
    ssize_t tbytes = 0;
    while (nleft > 0 && (nb = send(client, buffer, nleft,0)) > 0)
    {
        tbytes += nb;
        buffer += nb;
        nleft  -= nb;
    }
    if (tbytes == 0)
        tbytes = nb;
    return tbytes;
}

void printTimes(long* times){
    int fileSize = 1;
    bool bigger = false;

    for (int i = 0; i < 6; i++){
        if (bigger) printf("El tiempo para %dMB fue de %ld μs usando paso de mensajes\n", fileSize, times[i]);
        else printf("El tiempo para %dKB fue de %ld μs usando paso de mensajes\n", fileSize, times[i]);

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

void childProcess(){
    int check = 1, socket = clientConnection(), average = 5, count = 0;

    for (int i = 1; i < 100001; ){
        size_t size = 1000 * i;
        char *data = malloc(size);

        ssize_t tbytes = 0;
        
        if (data == NULL) errorMessage("failed to allocate memory\n");

        // Get Data Package
        tbytes = multi_read(socket, data, size);
        if (tbytes  != (ssize_t)size) errorMessage("Read error in %s()\n", __func__);
        if (send(socket, &check, sizeof(check), 0) == -1) 
            errorMessage("Error sending check value error: %s\n",strerror(errno));
        i = i * 10;

        // Restart sequence
        if (i > 10000  && average > 0) {
            average--;
            i = 1;
        }
        count++;
    }
    printf("%d\n", count);
}

void parentProcess(){
    struct client_data client = serverConnection();
    struct timeval start, end;
    long times [6];
    int index = 0, average = 5; 
    long int returnValue;

    for (int i = 1; i < 100001; ){
        size_t size = 1000 * i;
        char *data = generateData(i);
        gettimeofday(&start, 0);
        int dataCheck;

        // Sending Data Package to consumer
        if (multi_send(client.clientfd, data, size) != (ssize_t)size)
            errorMessage("Write error in %s()\n", __func__);

        if (read(client.clientfd, &dataCheck, sizeof(dataCheck)) == -1) 
            errorMessage("Error recibing check confirmation, error: %s\n",strerror(errno));

        // Get the time elapsed time
        free(data);
        gettimeofday(&end, 0);
        long newTime = ((end.tv_sec - start.tv_sec) * 1e6) + end.tv_usec - start.tv_usec;
        if (newTime == 0) newTime = 1;
        if (average == 5) times[index] = newTime;
        else times[index] = (times[index] + newTime)/2;
        index++;
        i = i * 10;

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
        childProcess();
    }
    else if (processId > 0){
        parentProcess();
    }
}

int main(int argc, char const *argv[]){
    startProgram();
    return 0;
}
