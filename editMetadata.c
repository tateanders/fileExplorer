#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "editMetadata.h"

int checkID3v2(FILE* file){
    char id3v2_header[3];
    fseek(file, 0, SEEK_SET);
    fread(id3v2_header, 1, 3, file);
    
    int has_id3v2 = 0;
    if (memcmp(id3v2_header, "ID3", 3) == 0) {
        printf("ID3v2 tag detected\n");
        return 1;
    }
    return 0;
}

int checkID3v1(FILE* file){
    char id3v1_header[3];
    fseek(file, -128, SEEK_END);
    fread(id3v1_header, 1, 3, file);

    if (memcmp(id3v1_header, "TAG", 3) == 0) {
        printf("ID3v1 tag detected\n");
        return 1;
    }
    return 0;
}

int addComment(struct dirent* entry, char* comment){
    //open the file with read and binary permissions
    FILE* file = fopen(entry->d_name, "r+b");
    if (!file) {
        printf("Error opening \"%s\"\n", entry->d_name);
        return 0;
    }
    printf("Opened: %s\n", entry->d_name);

    //check if it is ID3v1 or ID3v2
    int ID3v = 0;
    if (checkID3v1(file)) {
        ID3v = 1;
    } else if (checkID3v2(file)) {
        ID3v = 2;
    } else {
        printf("ID3 tag not found.\n");
        fclose(file);
        return 0;
    }






    fclose(file);
    return 1;
}