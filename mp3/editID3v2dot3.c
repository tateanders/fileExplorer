#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/types.h>
#include "editID3v2dot3.h"
#include "structures/dynarray.h"

/*-------------------------------------------------------------------------------------------------
    Helper Functions
-------------------------------------------------------------------------------------------------*/

static inline void write_u32_be(uint8_t* buf, uint32_t value) {
    buf[0] = (value >> 24) & 0xFF;
    buf[1] = (value >> 16) & 0xFF;
    buf[2] = (value >>  8) & 0xFF;
    buf[3] =  value        & 0xFF;
}

static inline void write_u16_be(uint8_t* buf, uint16_t value) {
    buf[0] = (value >> 8) & 0xFF;
    buf[1] =  value       & 0xFF;
}

// Function to convert regular integer to synchsafe integer
void int_to_synchsafe(uint32_t value, uint8_t* bytes) {
    bytes[0] = (value >> 21) & 0x7F;
    bytes[1] = (value >> 14) & 0x7F;
    bytes[2] = (value >> 7) & 0x7F;
    bytes[3] = value & 0x7F;
}

/*-------------------------------------------------------------------------------------------------
    Build string to write functions
-------------------------------------------------------------------------------------------------*/

long addHeader(uint8_t* dataString, struct ID3v2dot3Header* header) {
    memcpy(dataString, header->id, 3);
    dataString[3] = header->version;
    dataString[4] = header->revision;
    dataString[5] = header->flags;
    int_to_synchsafe(header->size, dataString + 6);
    return 10;
}

long addExHeader(uint8_t* dataString, struct ID3v2dot3ExtendedHeader* exHeader) {
    // Size (4 bytes)
    write_u32_be(dataString, exHeader->size);
    // Flags (2 bytes)
    write_u16_be(dataString + 4, exHeader->flags);
    // Padding size (4 bytes)
    write_u32_be(dataString + 6, exHeader->paddingSize);

    long written = 10;
    // Optional CRC
    if (exHeader->crcFlag) {
        write_u32_be(dataString + 10, exHeader->crc);
        written += 4;
    }

    return written;
}

long addFrame(uint8_t* dataString, struct ID3v2dot3Frame* frame) {
    // Frame ID: exactly 4 bytes (your struct stores 4 chars, not null-terminated)
    memcpy(dataString, frame->id, 4);

    // Size: 4 bytes big-endian (ID3v2.3 uses non-synchsafe 32-bit BE here)
    write_u32_be(dataString + 4, frame->size);

    // Flags: 2 bytes big-endian
    write_u16_be(dataString + 8, frame->flags);

    // Payload: copy frame->size bytes (if any)
    if ((frame->size > 0) && frame->data) {
        memcpy(dataString + 10, frame->data, (size_t)frame->size);
    }

    return 10 + (long)frame->size;
}

/*-------------------------------------------------------------------------------------------------
    write to file functions
-------------------------------------------------------------------------------------------------*/

void updateFile(FILE* file, struct ID3v2dot3MetaData* data) {
    uint32_t totalSize = data->header->size + 10;
    uint8_t* dataString = calloc(totalSize, sizeof(uint8_t));

    long currPos = addHeader(dataString, data->header);

    if (data->header->eFlag == 1) {
        currPos += addExHeader(dataString + currPos, data->exHeader);
    }
    
    struct dynarray* frames = data->frames;
    int numFrames = dynarray_size(frames);
    int i;
    for (i = 0; i < numFrames; i++) {
        struct ID3v2dot3Frame* frame = dynarray_get(frames, i);
        currPos += addFrame(dataString + currPos, frame);
    }

    fseek(file, 0, SEEK_SET);
    fwrite(dataString, 1, (size_t)currPos, file);
    fflush(file);

    //printf("updated file\n");

    free(dataString);
}

//-------------------------------------------------------------------------------------------------

void addWhiteSpace(FILE* file, struct ID3v2dot3MetaData* data){
    long totalSize = (long)data->header->size + 10;
    //compute the remaining size
    fseek(file, 0, SEEK_END);
    long fileEnd = ftell(file);
    long remainingSize = fileEnd - totalSize;

    //allocate the buffer
    uint8_t *musicDat = NULL;
    musicDat = malloc((size_t)remainingSize);
    fseek(file, totalSize, SEEK_SET);

    //read the data
    fread(musicDat, 1, (size_t)remainingSize, file);
    fseek(file, totalSize, SEEK_SET);

    //insert the zeros
    ssize_t WSSize = 512;
    uint8_t *WS = calloc((size_t)WSSize, 1);
    fwrite(WS, 1, (size_t)WSSize, file);
    free(WS);

    //write the remaining data
    fwrite(musicDat, 1, (size_t)remainingSize, file);
    fflush(file);

    //update the structs
    data->whiteSpace += WSSize;

    //free the buffer
    free(musicDat);
}

/*-------------------------------------------------------------------------------------------------
    Add Comment Functions
-------------------------------------------------------------------------------------------------*/

struct ID3v2dot3Frame* popCommentFrame(struct dynarray* arr) {
    int arrSize = dynarray_size(arr);
    int i;
    for (i = 0; i < arrSize; i++) {
        struct ID3v2dot3Frame* frame = (struct ID3v2dot3Frame*) dynarray_get(arr, i);
        if (memcmp(frame->id, "COMM", 4) == 0) {
            dynarray_remove(arr, i);
            return frame;
        }
    }
    return NULL;
}

int removeCommentIfExists(struct dynarray* arr) {
    struct ID3v2dot3Frame* comment = popCommentFrame(arr);
    int frameSize = 0;
    if (comment) {
        frameSize = 10 + (int)comment->size;
        free(comment->data);
        free(comment);
    }
    return frameSize;
}

//-------------------------------------------------------------------------------------------------

char* buildCOMMStr(char* comment, uint32_t* size) {
    char* data = (char*)calloc(1, 5 + strlen(comment));
    memcpy(data, "\0eng\0", 5);
    memcpy(data + 5, comment, strlen(comment));
    *size = 5 + strlen(comment);
    return data;
}

struct ID3v2dot3Frame* createCommentFrame(char* comment) {
    //create the frame
    struct ID3v2dot3Frame* frame = (struct ID3v2dot3Frame*)calloc(1, sizeof(struct ID3v2dot3Frame));
    //add the string and size
    char* data = buildCOMMStr(comment, &frame->size);
    frame->data = (uint8_t*)data;
    //add the tag
    strncpy(frame->id, "COMM", 4);
    return frame;
}

int insertCommentFrame(char* comment, struct dynarray* arr) {
    struct ID3v2dot3Frame* commentFrame = createCommentFrame(comment);
    int frameSize = 10 + commentFrame->size;
    dynarray_insert(arr, commentFrame);
    return frameSize;
}

/*-------------------------------------------------------------------------------------------------
    Calculate the size and add it to the header
-------------------------------------------------------------------------------------------------*/

void updateSize(struct ID3v2dot3MetaData* data) {
    uint32_t size = 0;
    if (data->exHeader) {
        size += data->exHeader->size;
    }
    int i;
    int numFrames = dynarray_size(data->frames);
    for (i = 0; i < numFrames; i++) {
        struct ID3v2dot3Frame* frame = dynarray_get(data->frames, i);
        size += frame->size + 10;
    }
    size += (uint32_t)data->whiteSpace;
    data->header->size = size;
}

/*-------------------------------------------------------------------------------------------------
    Main function
-------------------------------------------------------------------------------------------------*/

int addCommentV2dot3(FILE* file, char* comment, struct ID3v2dot3MetaData* data){
    //add the comment
    int bytesRemoved = removeCommentIfExists(data->frames);
    int bytesAdded = insertCommentFrame(comment, data->frames);
    //update the whitespace
    data->whiteSpace = bytesRemoved - bytesAdded;
    if (data->whiteSpace < 0) {
        addWhiteSpace(file, data);
    }
    //recalculate the size for the header
    updateSize(data);
    return 1;
}
