#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/time.h>
#include <pthread.h>
#include <math.h>
#include "../util/file_manager.c"

pthread_mutex_t mutex;
pthread_cond_t escrito, leido;

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

void printTimes(long* times){
    int fileSize = 1;
    bool bigger = false;

    for (int i = 0; i < 6; i++){
        if (bigger) printf("El tiempo para %dMB fue de %ld μs usando archivos\n", fileSize, times[i]);
        else printf("El tiempo para %dKB fue de %ld μs usando archivos\n", fileSize, times[i]);

        if (fileSize < 100) fileSize = fileSize * 10;
        else {
            fileSize = 1;
            bigger = true;
        }
    }

    printf("\n\n");
}

void checkErrors(int processId, int mutexCheck, int escritoCheck, int leidoCheck){
    // Generating the corresponding error message for the mutex
    if (mutexCheck != 0) errorMessage("Error generating the mutex");
    if (escritoCheck != 0) errorMessage("Error generating the escrito mutex condition");
    if (leidoCheck != 0) errorMessage("Error generating the leido mutex condition");

    // Generating the corresponding error message for the fork
    if (processId < 0) errorMessage("Error generating the fork");
}

void childrenProcess(char* route){
    int average = 5;

    for (int i = 0; i < 100001; ){
        pthread_cond_wait(&escrito, &mutex);
        char* data;
        int dataSize = 1024 * i;

        // Get Data Package
        loadFromFile(route);
        pthread_cond_signal(&leido);
        i = i * 10;

        // Restart sequence
        if (i == 100000 && average > 0) {
            average--;
            i = 1;
        }
    }
}

void parentProcess(char* route){
    struct timeval start, end;
    long times [6];
    int index = 0, average = 5;

    for (int i = 1; i < 100001; ){
        char* data = generateData(i);
        gettimeofday(&start, 0);
        int dataCheck;

        // Sending Data Package to consumer
        saveToFile(route, data);
        pthread_cond_signal(&escrito);
        pthread_cond_wait(&leido, &mutex);

        // Get the time elapsed time
        free(data);
        gettimeofday(&end, 0);
        long newTime = ((end.tv_sec - start.tv_sec) * 1e6) + (end.tv_usec - start.tv_usec);
        if (newTime == 0) newTime = 1;
        if (average == 5) times[index] = newTime;
        else times[index] = ceil((times[index] + newTime)/2);
        index++;
        i = i * 10;

        // Restart sequence
        if (index == 6 && average > 0){
            average--;
            index = 0;
            i = 1;
        }
    }

    printTimes(times);
    pthread_mutex_destroy(&mutex);
}

void startProgram(){
    pid_t processId = 0;
    char* route = "../data/data.txt";

    // Creating emparented processes
    int mutexCheck = pthread_mutex_init(&mutex, NULL);
    int escritoCheck = pthread_cond_init(&escrito, NULL);
    int leidoCheck = pthread_cond_init(&leido, NULL);
    pthread_mutex_lock(&mutex);
    processId = fork();
    checkErrors(processId, mutexCheck, escritoCheck, leidoCheck);

    // Spliting the code that witch process will execute
    if (processId == 0){
        childrenProcess(route);
    }
    else if (processId > 0){
        parentProcess(route);
    }
}

int main(int argc, char const *argv[]){
    startProgram();
    return 0;
}
