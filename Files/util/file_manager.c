#include <stdio.h>

void checkError(FILE* file){
    if (file == NULL) {
        perror("Error while creating the file pointer");
        exit(1);
    }
}

void saveToFile(char* route, char* data){
    FILE* file = fopen(route, "w");

    checkError(file);
    fprintf(file, "%s", data);
}

char* loadFromFile(char* route){
    FILE* file = fopen(route, "r");
    char buffer[102400000];

    checkError(file);
    fgets(buffer, 102400000, file);
}