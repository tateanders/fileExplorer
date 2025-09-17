#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "readID3v2dot3.h"
#include "structures/dynarray.h"

/*-------------------------------------------------------------------------------------------------
    Structs
-------------------------------------------------------------------------------------------------*/

const char* ID3v2dot3ValidTags[] = {
    "AENC", "APIC", "COMM", "COMR", "ENCR", "EQUA", "ETCO",
    "GEOB", "GRID", "IPLS", "LINK", "MCDI", "MLLT", "OWNE",
    "PRIV", "PCNT", "POPM", "POSS", "RBUF", "RVAD", "RVRB",
    "SYLT", "SYTC", "TALB", "TBPM", "TCOM", "TCON", "TCOP",
    "TDAT", "TDLY", "TENC", "TEXT", "TFLT", "TIME", "TIT1",
    "TIT2", "TIT3", "TKEY", "TLAN", "TLEN", "TMED", "TOAL",
    "TOFN", "TOLY", "TOPE", "TORY", "TOWN", "TPE1", "TPE2",
    "TPE3", "TPE4", "TPOS", "TPUB", "TRCK", "TRDA", "TRSN",
    "TRSO", "TSIZ", "TSRC", "TSSE", "TYER", "TXXX", "UFID",
    "USER", "USLT", "WCOM", "WCOP", "WOAF", "WOAR", "WOAS",
    "WORS", "WPAY", "WPUB", "WXXX"
};

const size_t ID3v23ValidFramesCount = sizeof(ID3v2dot3ValidTags) / sizeof(ID3v2dot3ValidTags[0]);

/*-------------------------------------------------------------------------------------------------
    Print Functions
-------------------------------------------------------------------------------------------------*/

void printFrame(struct ID3v2dot3Frame* frame) {
    printf("FRAME:\n");
    printf("TAG: %.4s ", frame->id);
    printf("Size: %i | Data: %.*s\n", frame->size, frame->size - 1, frame->data + 1);
}

void printExHeader(struct ID3v2dot3ExtendedHeader* exHeader) {
    printf("EXTENDED HEADER:\n");
    printf("Size: %i | Padding: %i | CRC flag: %i\n", exHeader->size, exHeader->paddingSize, exHeader->crcFlag);
}

void printHeader(struct ID3v2dot3Header* header) {
    printf("HEADER:\n");
    printf("Version: %i | Size: %i | uFlag: %i | eFlag: %i | xFlag: %i\n", header->version, header->size, header->uFlag, header->eFlag, header->xFlag);
}

void printMetaData(struct ID3v2dot3MetaData* data) {
    printHeader(data->header);
    if (data->header->eFlag) {
        printExHeader(data->exHeader);
    }
    int i;
    int numFrames = dynarray_size(data->frames);
    for (i = 0; i < numFrames; i++) {
        printFrame(dynarray_get(data->frames, i));
    }
    printf("Padding: %zi\n\n", data->padding);
}

/*-------------------------------------------------------------------------------------------------
    Helper functions
-------------------------------------------------------------------------------------------------*/

int isValidID3v23Frame(char* tag) {
    size_t i;
    for (i = 0; i < ID3v23ValidFramesCount; i++) {
        if (memcmp(tag, ID3v2dot3ValidTags[i], 4) == 0) {
            return 1;
        }
    }
    return 0;
}

// Function to convert synchsafe integer to regular integer
uint32_t synchsafe_to_int(uint8_t* bytes) {
    return ((uint32_t)bytes[0] << 21)
         | ((uint32_t)bytes[1] << 14)
         | ((uint32_t)bytes[2] << 7)
         |  (uint32_t)bytes[3];
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
    //create header
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

void readID3v2dot3ExtendedHeader(FILE* file, struct ID3v2dot3ExtendedHeader* exHeader) {
    // Read 10 bytes first (size + flags + padding size)
    uint8_t buffer[10];
    fread(buffer, 1, 10, file);

    exHeader->size = (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];
    exHeader->flags = (buffer[4] << 8) | buffer[5];
    exHeader->paddingSize = (buffer[6] << 24) | (buffer[7] << 16) | (buffer[8] << 8) | buffer[9];
    exHeader->crcFlag = (exHeader->flags & 0x8000) ? 1 : 0;

    // Read optional CRC if present
    if (exHeader->crcFlag) {
        uint8_t crcBuf[4];
        if (fread(crcBuf, 1, 4, file) != 4) {
            printf("Failed to read CRC from extended header\n");
            return;
        }
        exHeader->crc = (crcBuf[0] << 24) | (crcBuf[1] << 16) | (crcBuf[2] << 8) | crcBuf[3];
    } else {
        exHeader->crc = 0; // No CRC present
    }
    // printExHeader(exHeader);
}

struct ID3v2dot3Frame* readFrame(FILE* file) {
    //read the tag
    char tagBuffer[4];
    long currentPos = ftell(file);
    fread(tagBuffer, 1, 4, file);
    if (!isValidID3v23Frame(tagBuffer)){
        fseek(file, currentPos, SEEK_SET);
        return NULL;
    }

    struct ID3v2dot3Frame* frame = (struct ID3v2dot3Frame*)calloc(1, sizeof(struct ID3v2dot3Frame));
    memcpy(frame->id, tagBuffer, 4);

    //read the size
    uint8_t sizeBuf[4];
    if (fread(sizeBuf, 1, 4, file) != 4) {
        free(frame);
        return NULL;
    }
    frame->size = ((uint32_t)sizeBuf[0] << 24) | ((uint32_t)sizeBuf[1] << 16) | ((uint32_t)sizeBuf[2] << 8) | (uint32_t)sizeBuf[3];
    if (frame->size > (1u << 24)) {
        free(frame);
        return NULL;
    }

    //read the flags
    uint8_t flagsBuf[2];
    if (fread(flagsBuf, 1, 2, file) != 2) { free(frame); return NULL; }
    frame->flags = (uint16_t)flagsBuf[0] << 8 | (uint16_t)flagsBuf[1];

    //read the data
    frame->data = (uint8_t*)calloc(1, (size_t)frame->size);
    if (!frame->data) { free(frame); return NULL; }
    if (fread(frame->data, 1, frame->size, file) != frame->size) {
        free(frame->data);
        free(frame);
        return NULL;
    }

    return frame;
}

struct dynarray* getFrames(FILE* file) {
    struct dynarray* arr = dynarray_create();
    struct ID3v2dot3Frame* frame = readFrame(file);
    while(frame) {
        dynarray_insert(arr, (void*)frame);
        frame = readFrame(file);
    }
    return arr;
}

long getpadding(FILE* file) {
    int c;
    long count = 0;

    while ((c = fgetc(file)) != EOF) {
        if (c == 0) {
            //increment padding
            count++;
            continue;
        } else {
            //return the padding
            if (ungetc(c, file) == EOF) {
                //seek back one byte
                long pos = ftell(file);
                if (pos == -1L) return -1L;
                if (fseek(file, pos - 1, SEEK_SET) != 0) return -1L;
            }
            return count;
        }
    }

    //if we got an EOF, return the count
    return count;
}

/*-------------------------------------------------------------------------------------------------
    Add comment functions for ID3v2.3
-------------------------------------------------------------------------------------------------*/

void freeDataV2dot3(struct ID3v2dot3MetaData* data) {
    free(data->header);
    free(data->exHeader);
    int arrSize = dynarray_size(data->frames);
    int i;
    for (i = 0; i < arrSize; i++) {
        struct ID3v2dot3Frame* frame = (struct ID3v2dot3Frame*)dynarray_get(data->frames, i);
        if (frame->data) {
            free(frame->data);
        }
        free(frame);
    }
    dynarray_free(data->frames);
    free(data);
    return;
}

struct ID3v2dot3MetaData* getMetaDataV2dot3(FILE* file) {
    struct ID3v2dot3MetaData* data = (struct ID3v2dot3MetaData*)calloc(1, sizeof(struct ID3v2dot3MetaData));
    fseek(file, 0, SEEK_SET);
    data->header = readID3v2dot3Header(file);
    if (!data->header) {
        return NULL;
    }
    data->exHeader = calloc(1, sizeof(struct ID3v2dot3ExtendedHeader));
    if (data->header->eFlag) {
        readID3v2dot3ExtendedHeader(file, data->exHeader);
    }
    data->frames = getFrames(file);
    long ws = getpadding(file);
    if (ws < 0){
        ws = 0;
    }
    data->padding = (ssize_t)ws;

    // printf("Data after reading\n");
    // printMetaData(data);

    return data;
}
