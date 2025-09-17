#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "readID3v2dot4.h"
#include "structures/dynarray.h"

/*-------------------------------------------------------------------------------------------------
    Structs
-------------------------------------------------------------------------------------------------*/

const char* ID3v2dot4ValidTags[] = {
    "AENC", "APIC", "ASPI", "COMM", "COMR", "ENCR", "EQU2", "ETCO",
    "GEOB", "GRID", "LINK", "MCDI", "MLLT", "OWNE", "PRIV", "PCNT",
    "POPM", "POSS", "RBUF", "RVA2", "RVRB", "SEEK", "SIGN", "SYLT",
    "SYTC", "TALB", "TBPM", "TCOM", "TCON", "TCOP", "TDEN", "TDLY", 
    "TDOR", "TDRC", "TDRL", "TDTG", "TENC", "TEXT", "TFLT", "TIME",
    "TIT1", "TIT2", "TIT3", "TKEY", "TLAN", "TLEN", "TMCL", "TMED",
    "TMOO", "TOAL", "TOFN", "TOLY", "TOPE", "TORY", "TOWN", "TPE1",
    "TPE2", "TPE3", "TPE4", "TPOS", "TPRO", "TPUB", "TRCK", "TRDA",
    "TRSN", "TRSO", "TSOA", "TSOP", "TSOT", "TSRC", "TSSE", "TSST",
    "TSIZ", "TXXX", "UFID", "USER", "USLT", "WCOM", "WCOP", "WOAF",
    "WOAR", "WOAS", "WORS", "WPAY", "WPUB", "WXXX", "AENC", "ASPI",
    "EQUA", "RVA2", "EQU2", "CHAP", "CTOC", "LINK", "SIGN", "SEEK",
    "GRID", "MLLT", "MCDI", "POSS", "PRIV", "OWNE", "COMR", "ENCR",
    "GEOB", "RBUF", "RVRB", "POSS"
};

const size_t ID3v24ValidFramesCount = sizeof(ID3v2dot4ValidTags) / sizeof(ID3v2dot4ValidTags[0]);

/*-------------------------------------------------------------------------------------------------
    Print Functions
-------------------------------------------------------------------------------------------------*/

void printFramev2dot4(struct ID3v2dot4Frame* frame) {
    printf("FRAME:\n");
    printf("TAG: %.4s ", frame->id);
    printf("Size: %i | Data: %.*s\n", frame->size, frame->size - 1, frame->data + 1);
}

void printHeaderv2dot4(struct ID3v2dot4Header* header) {
    printf("HEADER:\n");
    printf("Version: %i | Size: %i | uFlag: %i | eFlag: %i | xFlag: %i\n", header->version, header->size, header->uFlag, header->eFlag, header->xFlag);
}

void printMetaData4(struct ID3v2dot4MetaData* data) {
    printHeaderv2dot4(data->header);
    int i;
    int numFrames = dynarray_size(data->frames);
    for (i = 0; i < numFrames; i++) {
        printFramev2dot4(dynarray_get(data->frames, i));
    }
    printf("Padding: %zi\n\n", data->padding);
}

/*-------------------------------------------------------------------------------------------------
    Helper functions
-------------------------------------------------------------------------------------------------*/

int isValidID3v24Frame(char* tag) {
    size_t i;
    for (i = 0; i < ID3v24ValidFramesCount; i++) {
        if (memcmp(tag, ID3v2dot4ValidTags[i], 4) == 0) {
            return 1;
        }
    }
    return 0;
}

// Function to convert synchsafe integer to regular integer
uint32_t synchsafe_to_intv2dot4(const uint8_t* bytes) {
    return ((uint32_t)(bytes[0] & 0x7F) << 21)
         | ((uint32_t)(bytes[1] & 0x7F) << 14)
         | ((uint32_t)(bytes[2] & 0x7F) << 7)
         |  (uint32_t)(bytes[3] & 0x7F);
}

void setFlagsv2dot4(struct ID3v2dot4Header* header) {
    uint8_t flags = header->flags;
    header->uFlag = (flags & 0x80) ? 1 : 0;
    header->eFlag = (flags & 0x40) ? 1 : 0;
    header->xFlag = (flags & 0x20) ? 1 : 0;
    header->fFlag = (flags & 0x10) ? 1 : 0;
}

/*-------------------------------------------------------------------------------------------------
    Read functions
-------------------------------------------------------------------------------------------------*/

struct ID3v2dot4Header* readID3v2dot4Header(FILE* file) {
    //create header
    uint8_t headerData[10];
    if ((fread(headerData, 1, 10, file) != 10) || (memcmp(headerData, "ID3", 3) != 0)) {
        return NULL;
    }
    struct ID3v2dot4Header* header = calloc(1, sizeof(struct ID3v2dot4Header));
    
    memcpy(header->id, headerData, 3);
    header->version = headerData[3];
    header->revision = headerData[4];
    header->flags = headerData[5];
    header->size = synchsafe_to_intv2dot4(headerData + 6);
    setFlagsv2dot4(header);

    return header;
}

struct ID3v2dot4Frame* readFramev2dot4(FILE* file) {
    //read the tag
    char tagBuffer[4];
    long currentPos = ftell(file);
    fread(tagBuffer, 1, 4, file);
    if (!isValidID3v24Frame(tagBuffer)){
        fseek(file, currentPos, SEEK_SET);
        return NULL;
    }

    struct ID3v2dot4Frame* frame = (struct ID3v2dot4Frame*)calloc(1, sizeof(struct ID3v2dot4Frame));
    memcpy(frame->id, tagBuffer, 4);

    //read the size
    uint8_t sizeBuf[4];
    if (fread(sizeBuf, 1, 4, file) != 4) {
        free(frame);
        return NULL;
    }
    frame->size = synchsafe_to_intv2dot4(sizeBuf);

    //sanity check
    const uint32_t MAX_FRAME_SIZE = 64u * 1024u * 1024u;
    if (frame->size > MAX_FRAME_SIZE) {
        free(frame);
        return NULL;
    }

    //read the flags
    uint8_t flagsBuf[2];
    fread(flagsBuf, 1, 2, file);
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

struct dynarray* getFramesv2dot4(FILE* file) {
    struct dynarray* arr = dynarray_create();
    struct ID3v2dot4Frame* frame = readFramev2dot4(file);
    while(frame) {
        dynarray_insert(arr, (void*)frame);
        frame = readFramev2dot4(file);
    }
    return arr;
}

long getpaddingv2dot4(FILE* file) {
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

void freeDataV2dot4(struct ID3v2dot4MetaData* data) {
    free(data->header);
    int arrSize = dynarray_size(data->frames);
    int i;
    for (i = 0; i < arrSize; i++) {
        struct ID3v2dot4Frame* frame = (struct ID3v2dot4Frame*)dynarray_get(data->frames, i);
        if (frame->data) {
            free(frame->data);
        }
        free(frame);
    }
    dynarray_free(data->frames);
    free(data);
    return;
}

struct ID3v2dot4MetaData* getMetaDataV2dot4(FILE* file) {
    struct ID3v2dot4MetaData* data = (struct ID3v2dot4MetaData*)calloc(1, sizeof(struct ID3v2dot4MetaData));
    fseek(file, 0, SEEK_SET);
    data->header = readID3v2dot4Header(file);
    if ((!data->header) || (data->header->flags)) {
        free(data->header);
        free(data);
        return NULL;
    }
    data->frames = getFramesv2dot4(file);
    long ws = getpaddingv2dot4(file);
    if (ws < 0){
        ws = 0;
    }
    data->padding = (ssize_t)ws;

    // printf("Data after reading\n");
    // printMetaData4(data);

    return data;
}
