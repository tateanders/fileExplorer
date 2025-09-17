#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>

#include "editID3v2dot4.h"
#include "structures/dynarray.h"

/*-------------------------------------------------------------------------------------------------
    Helper Functions (lowkey used AI)
-------------------------------------------------------------------------------------------------*/

static inline void write_u16_bev2dot4(uint8_t* buf, uint16_t value) {
    buf[0] = (value >> 8) & 0xFF;
    buf[1] =  value       & 0xFF;
}

void int_to_synchsafev2dot4(uint32_t value, uint8_t* bytes) {
    bytes[0] = (value >> 21) & 0x7F;
    bytes[1] = (value >> 14) & 0x7F;
    bytes[2] = (value >> 7) & 0x7F;
    bytes[3] = value & 0x7F;
}

/*-------------------------------------------------------------------------------------------------
    Build string to write functions
-------------------------------------------------------------------------------------------------*/

long addHeaderv2dot4(uint8_t* dataString, struct ID3v2dot4Header* header) {
    memcpy(dataString, header->id, 3);
    dataString[3] = header->version;
    dataString[4] = header->revision;
    dataString[5] = header->flags;
    int_to_synchsafev2dot4(header->size, dataString + 6);
    return 10;
}

long addFramev2dot4(uint8_t* dataString, struct ID3v2dot4Frame* frame) {
    //tag
    memcpy(dataString, frame->id, 4);

    //size
    int_to_synchsafev2dot4(frame->size, dataString + 4);

    //flags
    write_u16_bev2dot4(dataString + 8, frame->flags);

    //actual data
    if ((frame->size > 0) && frame->data) {
        memcpy(dataString + 10, frame->data, (size_t)frame->size);
    }

    return 10 + (long)frame->size;
}

/*-------------------------------------------------------------------------------------------------
    Calculate the size and add it to the header
-------------------------------------------------------------------------------------------------*/

void updateSizev2dot4(struct ID3v2dot4MetaData* data) {
    uint32_t size = 0;
    int i;
    int numFrames = dynarray_size(data->frames);
    for (i = 0; i < numFrames; i++) {
        struct ID3v2dot4Frame* frame = dynarray_get(data->frames, i);
        size += frame->size + 10;
    }
    size += (uint32_t)data->padding;
    data->header->size = size;
}

/*-------------------------------------------------------------------------------------------------
    write to file functions
-------------------------------------------------------------------------------------------------*/

void updateFilev2dot4(FILE* file, struct ID3v2dot4MetaData* data) {
    //recalculate the size for the header
    updateSizev2dot4(data);
    //get the total size and the string we will print
    uint32_t totalSize = data->header->size + 10;
    uint8_t* dataString = calloc(totalSize, sizeof(uint8_t));

    //get the current position and add header
    long currPos = addHeaderv2dot4(dataString, data->header);
    
    //add frames
    struct dynarray* frames = data->frames;
    int numFrames = dynarray_size(frames);
    int i;
    for (i = 0; i < numFrames; i++) {
        struct ID3v2dot4Frame* frame = dynarray_get(frames, i);
        currPos += addFramev2dot4(dataString + currPos, frame);
    }

    //write the data
    fseek(file, 0, SEEK_SET);
    fwrite(dataString, 1, (size_t)totalSize, file);
    fflush(file);

    free(dataString);
}

//-------------------------------------------------------------------------------------------------

void addpaddingv2dot4(FILE* file, struct ID3v2dot4MetaData* data){
    long totalSize = (long)data->header->size + 10;

    //compute the remaining size
    fseek(file, 0, SEEK_END);
    long fileEnd = ftell(file);
    long remainingSize = fileEnd - totalSize;
    if (remainingSize <= 0){
        printf("error allocating padding\n");
        return;
    }

    //allocate the buffer
    uint8_t *musicDat = NULL;
    musicDat = malloc((size_t)remainingSize);
    fseek(file, totalSize, SEEK_SET);

    //read the data
    fread(musicDat, 1, (size_t)remainingSize, file);
    fseek(file, totalSize, SEEK_SET);

    //insert the zeros
    ssize_t padSize = 512;
    uint8_t *pad = calloc((size_t)padSize, 1);
    fwrite(pad, 1, (size_t)padSize, file);
    free(pad);

    //write the remaining data
    fwrite(musicDat, 1, (size_t)remainingSize, file);
    fflush(file);

    //update the structs
    data->padding += padSize;

    //free the buffer
    free(musicDat);
}

/*-------------------------------------------------------------------------------------------------
    Add Comment Functions
-------------------------------------------------------------------------------------------------*/

struct ID3v2dot4Frame* popCommentFramev2dot4(struct dynarray* arr) {
    int arrSize = dynarray_size(arr);
    int i;
    for (i = 0; i < arrSize; i++) {
        struct ID3v2dot4Frame* frame = (struct ID3v2dot4Frame*) dynarray_get(arr, i);
        if (memcmp(frame->id, "COMM", 4) == 0) {
            dynarray_remove(arr, i);
            return frame;
        }
    }
    return NULL;
}

//remove comment frame and return how many bytes were removed
int removeCommentIfExistsv2dot4(struct dynarray* arr) {
    struct ID3v2dot4Frame* comment = popCommentFramev2dot4(arr);
    int frameSize = 0;
    if (comment) {
        frameSize = 10 + (int)comment->size;
        free(comment->data);
        free(comment);
    }
    return frameSize;
}

//-------------------------------------------------------------------------------------------------

char* buildCOMMStrv2dot4(char* comment, uint32_t* size) {
    char* data = (char*)calloc(1, 5 + strlen(comment));
    memcpy(data, "\0eng\0", 5);
    memcpy(data + 5, comment, strlen(comment));
    *size = 5 + strlen(comment);
    return data;
}

struct ID3v2dot4Frame* createCommentFramev2dot4(char* comment) {
    //create the frame
    struct ID3v2dot4Frame* frame = (struct ID3v2dot4Frame*)calloc(1, sizeof(struct ID3v2dot4Frame));
    //add the string and size
    char* data = buildCOMMStrv2dot4(comment, &frame->size);
    frame->data = (uint8_t*)data;
    //add the tag
    strncpy(frame->id, "COMM", 4);
    return frame;
}

//add comment frame and return how many bytes were added
int insertCommentFramev2dot4(char* comment, struct dynarray* arr) {
    struct ID3v2dot4Frame* commentFrame = createCommentFramev2dot4(comment);
    int frameSize = 10 + commentFrame->size;
    dynarray_push(arr, commentFrame);
    return frameSize;
}

// int checkDupCommentV2dot4(struct dynarray* arr, char* comment) {
//     int i;
//     for (i = 0; i < dynarray_size(arr); i++) {
//         struct ID3v2dot4Frame* frame = dynarray_get(arr, i);

//         if (memcmp(frame->id, "COMM", 4) != 0) {
//             continue;
//         }

//         char* existingComment = (char*)(frame->data);
//         if (strstr(existingComment, comment - 1) != 0) {
//             return 1;
//         }
//     }
//     return 0;
// }

/*-------------------------------------------------------------------------------------------------
    Main function
-------------------------------------------------------------------------------------------------*/

int addCommentV2dot4(FILE* file, char* comment, struct ID3v2dot4MetaData* data){
    // if (checkDupCommentV2dot4(data->frames, comment)) {
    //     printf("skipping comment\n");
    //     return 1;
    // }
    //add the comment
    int bytesRemoved = removeCommentIfExistsv2dot4(data->frames);
    int bytesAdded = insertCommentFramev2dot4(comment, data->frames);
    //update the padding
    data->padding += bytesRemoved - bytesAdded;
    if (data->padding < 0) {
        addpaddingv2dot4(file, data);
    }
    printf("Data before writing\n");
    printMetaData4(data);
    updateFilev2dot4(file, data);
    return 1;
}
