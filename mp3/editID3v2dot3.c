#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include "editID3v2dot3.h"
#include "structures/list.h"

/*-------------------------------------------------------------------------------------------------
    Structs
-------------------------------------------------------------------------------------------------*/

struct ID3v2dot3Header {
    //main data that gets read
    char id[3];
    uint8_t version;
    uint8_t revision;
    uint8_t flags;
    uint32_t size;
    //data after interpretation
    uint8_t uFlag;
    uint8_t eFlag;
    uint8_t xFlag;
};

/*-------------------------------------------------------------------------------------------------
    Helper functions
-------------------------------------------------------------------------------------------------*/

// Function to convert synchsafe integer to regular integer
uint32_t synchsafe_to_int(uint8_t* bytes) {
    return (bytes[0] << 21) | (bytes[1] << 14) | (bytes[2] << 7) | bytes[3];
}

// Function to convert regular integer to synchsafe integer
void int_to_synchsafe(uint32_t value, uint8_t* bytes) {
    bytes[0] = (value >> 21) & 0x7F;
    bytes[1] = (value >> 14) & 0x7F;
    bytes[2] = (value >> 7) & 0x7F;
    bytes[3] = value & 0x7F;
}

void setFlags(struct ID3v2dot3Header* header) {
    header->uFlag = 0;
    header->eFlag = 0;
    header->xFlag = 0;
    uint8_t flags = header->flags;
    header->uFlag = (header->flags & 0x80) ? 1 : 0;
    header->eFlag = (header->flags & 0x40) ? 1 : 0;
    header->xFlag = (header->flags & 0x20) ? 1 : 0;
}

/*-------------------------------------------------------------------------------------------------
    Read functions
-------------------------------------------------------------------------------------------------*/
struct ID3v2dot3Header* readID3v2dot3Header(FILE* file) {
    uint8_t headerData[10];
    if ((fread(headerData, 1, 10, file) != 10) || (memcmp(headerData, "ID3", 3) != 0)) {
        return NULL;
    }
    struct ID3v2dot3Header* header = calloc(1, sizeof(struct ID3v2dot3Header));
    
    memcpy(header->id, headerData, 3);
    header->version = headerData[3];
    header->revision = headerData[4];
    header->flags = headerData[5];
    header->size = synchsafe_to_int(headerData + 6);
    setFlags(header);
    
    return header;
}

void printHeader(struct ID3v2dot3Header* header) {
    printf("Version: %i | Size: %i | uFlag: %i | eFlag: %i | xFlag: %i\n", header->version, header->size, header->uFlag, header->eFlag, header->xFlag);
}

/*-------------------------------------------------------------------------------------------------
    Add comment functions for ID3v2.3
-------------------------------------------------------------------------------------------------*/
int addCommentV2dot3(FILE* file, char* comment){
    fseek(file, 0, SEEK_SET);
    struct ID3v2dot3Header* header = readID3v2dot3Header(file);
    if (!header) {
        return 0;
    }
    printHeader(header);

    return 1;
}