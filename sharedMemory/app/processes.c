#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <pthread.h>
#define MAX_PROCESOS 1   

// Program variables
pthread_mutex_t mutex;
pthread_cond_t vacio, lleno;

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
        if (bigger) printf("El tiempo para %dMb fue de %ld μs usando memoria compartida\n", fileSize, times[i]);
        else printf("El tiempo para %dKb fue de %ld μs usando memoria compartida\n", fileSize, times[i]);

        if (fileSize < 100) fileSize = fileSize * 10;
        else {
            fileSize = 1;
            bigger = true;
        }
    }
}

void checkErrors(int processId, int mutexCheck, int vacioCheck, int llenoCheck){
    // Generating the corresponding error message for the mutex
    if (mutexCheck != 0) errorMessage("Error generating the mutex");
    if (vacioCheck != 0) errorMessage("Error generating the vacio condition");
    if (llenoCheck != 0) errorMessage("Error generating the lleno condition");

    // Generating the corresponding error message for the fork
    if (processId < 0) errorMessage("Error generating the fork");
}

void* createSharedMemory(size_t size){
    int protection = PROT_READ | PROT_WRITE; 
    int visibility = MAP_SHARED | MAP_ANONYMOUS;

    return mmap(NULL, size, protection, visibility, -1, 0);
}

void childrenProcess(char* sharedMemory){
    int check = 1, average = 5;

    for (int i = 1; i < 100001; i = i * 10){
        pthread_cond_wait(&lleno, &mutex);
        char* data;
        int dataSize = 1024 * i;

        // Get Data Package
        memcpy(&data, sharedMemory, sizeof(sharedMemory));

        // Sending check
        pthread_cond_signal(&vacio);

        // Restart sequence
        if (i == 100000 && average > 0) {
            average--;
            i = 1;
        }
    }
}

void parentProcess(char* sharedMemory){
    struct timeval start, end;
    long times [6] = {0, 0, 0, 0, 0, 0};
    int index = 0, average = 5;

    for (int i = 1; i < 1000001; i = i * 10){
        char* data = generateData(i);
        gettimeofday(&start, 0);

        // Sending Data Package to consumer
        memcpy(sharedMemory, data, sizeof(data));
        pthread_cond_signal(&lleno);
        pthread_cond_wait(&vacio, &mutex);

        // Get the time elapsed time
        free(data);
        gettimeofday(&end, 0);
        long newTime = ((end.tv_sec - start.tv_sec) * 1e6) + (end.tv_usec - start.tv_usec);
        if (newTime == 0) newTime++;
        if (average == 5) times[index] = newTime;
        else times[index] = ceil((times[index] + newTime)/2);
        index++;

        // Restart sequence
        if (index > 5 && average > 0) {
            average --;
            index = 0;
            i = 1;
        }
    }

    printTimes(times);
    pthread_mutex_destroy(&mutex);
}

void startProgram(){
    pid_t processId = 0;
    char* sharedMemory = createSharedMemory(102400000);
    int mutexCheck, vacioCheck, llenoCheck;
 

    // Creating emparented processes and shared memory
    mutexCheck = pthread_mutex_init(&mutex, NULL);
    vacioCheck = pthread_cond_init(&vacio, NULL);
    llenoCheck = pthread_cond_init(&lleno, NULL);
    processId = fork();
    checkErrors(processId, mutexCheck, vacioCheck, llenoCheck);

    // Spliting the code that witch process will execute
    pthread_mutex_lock(&mutex);
    if (processId == 0){
        childrenProcess(sharedMemory);
    }
    else if (processId > 0){
        parentProcess(sharedMemory);
    }
}

int main(int argc, char const *argv[]){
    startProgram();
    return 0;
}
