#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include <stdarg.h>

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

void printTimes(long *times){
    int fileSize = 1;
    bool bigger = false;

    for (int i = 0; i < 6; i++){
        if (bigger) printf("El tiempo para %dMB fue de %ld μs usando tuberias\n", fileSize, times[i]);
        else printf("El tiempo para %dKB fue de %ld μs usando tuberias\n", fileSize, times[i]);

        if (fileSize < 100) fileSize = fileSize * 10;
        else{
            fileSize = 1;
            bigger = true;
        }
    }

    printf("\n\n");
}

void checkErrors(int processId, int pipesPointers[2]){
    for (int i = 0; i < 2; i++)
    {
        if (pipesPointers[i] < 0)
        {
            // Generating the corresponding error message for pipe error
            char message[] = "Error starting the pipe ";
            strcat(message, (char *)&i);
            errorMessage(message);
        }
    }

    // Generating the corresponding error message for the fork
    if (processId < 0)
        errorMessage("Error generating the fork");
}

ssize_t multi_read(int fd, char *buffer, size_t nbytes){
    ssize_t nb = 0;
    size_t nleft = nbytes;
    ssize_t tbytes = 0;
    while (nleft > 0 && (nb = read(fd, buffer, nleft)) > 0)
    {
        tbytes += nb;
        buffer += nb;
        nleft  -= nb;
    }
    if (tbytes == 0)
        tbytes = nb;
    return tbytes;
}

ssize_t multi_write(int fd, const char *buffer, size_t nbytes){
    ssize_t nb = 0;
    size_t nleft = nbytes;
    ssize_t tbytes = 0;
    while (nleft > 0 && (nb = write(fd, buffer, nleft)) > 0)
    {
        tbytes += nb;
        buffer += nb;
        nleft  -= nb;
    }
    if (tbytes == 0)
        tbytes = nb;
    return tbytes;
}

void childProcess(int pipeWrite, int pipeRead){
    int check = 1, average = 5, count = 0;

    for (int i = 1; i < 100001; ){
        size_t size = 1000 * i;
        char *data = malloc(size);

        if (data == NULL)
            errorMessage("failed to allocate memory\n");


        // Get Data Package
        if (multi_read(pipeRead, data, size) != (ssize_t)size)
            errorMessage("Read error in %s()\n", __func__);

        // Sending check
        if (write(pipeWrite, &check, sizeof(check)) != sizeof(check))
            errorMessage("Write error in %s()\n", __func__);
        count++;
        i = i * 10;

        // Restart sequence
        if (count > 5 && average > 0){
            average--;
            count = 0;
            i = 1;
        }
    }
}

void parentProcess(int pipeWrite, int pipeRead){
    struct timeval start, stop;
    long times[6];
    int index = 0, average = 5;

    for (int i = 1; i < 100001; ){
        size_t size = 1000 * i;
        char *data = generateData(i);
        gettimeofday(&start, NULL);
        int check;

        // Sending Data Package to consumer
        if (multi_write(pipeWrite, data, size) != (ssize_t)size)
            errorMessage("Write error in %s()\n", __func__);

        // Getting the check confirmation
        if (read(pipeRead, &check, sizeof(check)) != sizeof(check))
            errorMessage("Read error in %s()\n", __func__);

        // Get the time elapsed time
        gettimeofday(&stop, NULL);
        long newTime = ((stop.tv_sec - start.tv_sec) * 1e6) + (stop.tv_usec - start.tv_usec);
        if (newTime == 0) newTime = 1;
        if (average == 5) times[index] = newTime;
        else times[index] = ceil((times[index] + newTime) / 2);
        index++;
        i = i * 10;
        free(data);

        // Restart sequence
        if (index > 5 && average > 0){
            average--;
            index = 0;
            i = 1;
        }
    }

    printTimes(times);
}

void startProgram(void){
    int pipePointers[2], parentPipes[2], childrenPipes[2];
    pid_t processId = 0;

    // Creating emparented processes and pipes
    pipePointers[0] = pipe(parentPipes);
    pipePointers[1] = pipe(childrenPipes);
    processId = fork();
    checkErrors(processId, pipePointers);

    // Spliting the code that witch process will execute
    if (processId == 0)
    {
        close(childrenPipes[0]);
        close(parentPipes[1]);
        childProcess(childrenPipes[1], parentPipes[0]);
        close(childrenPipes[1]);
        close(parentPipes[0]);
    }
    else if (processId > 0)
    {
        close(childrenPipes[1]);
        close(parentPipes[0]);
        parentProcess(parentPipes[1], childrenPipes[0]);
        close(childrenPipes[0]);
        close(parentPipes[1]);
    }
}

int main(int argc, char const *argv[]){
    startProgram();
    return 0;
}
