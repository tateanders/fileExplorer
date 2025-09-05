#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include "metadata.h"
#include "mp3/readID3v2dot3.h"

/*-------------------------------------------------------------------------------------------------
    Check format functions
-------------------------------------------------------------------------------------------------*/

int getID3v(FILE* file) {
    char id3v2_header[6]; // "ID3" + major + minor + flags
    fseek(file, 0, SEEK_SET);
    fread(id3v2_header, 1, 6, file);
    
    if (memcmp(id3v2_header, "ID3", 3) == 0) {
        
        if (id3v2_header[3] == 3) {
            return 23;
        } else if (id3v2_header[3] == 4) {
            return 24;
        }
    }
    return 0; // No ID3v2 tag found
}

/*-------------------------------------------------------------------------------------------------
    Main Functions
-------------------------------------------------------------------------------------------------*/

int addComment(struct dirent* entry, char* comment){
    //open the file with read and binary permissions
    FILE* file = fopen(entry->d_name, "r+b");
    if (!file) {
        printf("Error opening \"%s\"\n", entry->d_name);
        return 0;
    }

    //get the tag
    int ID3v = getID3v(file);
    if (!ID3v) {
        return 0;
    }

    //add the comment
    if (ID3v == 23) {
        struct ID3v2dot3MetaData* data = getMetaDataV2dot3(file);
        freeDataV2dot3(data);
    }

    fclose(file);
    return ID3v;
}