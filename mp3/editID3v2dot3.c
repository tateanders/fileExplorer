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


char* buildCOMMData(char* comment, uint32_t* size) {
    char* data = (char*)calloc(1, 5 + strlen(comment));
    memcpy(data, "\0eng\0", 5);
    memcpy(data + 5, comment, strlen(comment));
    *size = 5 + strlen(comment);
    return data;
}

struct ID3v2dot3Frame* createComment(char* comment) {
    //create the frame
    struct ID3v2dot3Frame* frame = (struct ID3v2dot3Frame*)calloc(1, sizeof(struct ID3v2dot3Frame));
    char* data = buildCOMMData(comment, &frame->size);
    // if (!data) {
    //     free(frame);
    //     return NULL;
    // }
    frame->data = (uint8_t*)data;
    strncpy(frame->id, "COMM", 4);
    return frame;
}

ssize_t updateComment(struct ID3v2dot3Frame* frame, char* comment) {
    uint32_t oldSize = frame->size;
    uint32_t newSize = 0;
    char* data = buildCOMMData(comment, &newSize);
    // if (!data) {
    //     return NULL;
    // }
    free(frame->data);
    frame->data = (uint8_t*)data;
    frame->size = (uint32_t)newSize;
    return (ssize_t)oldSize - (ssize_t)newSize;
}

struct ID3v2dot3Frame* commentExists(struct dynarray* arr) {
    int arrSize = dynarray_size(arr);
    int i;
    for (i = 0; i < arrSize; i++) {
        struct ID3v2dot3Frame* frame = (struct ID3v2dot3Frame*) dynarray_get(arr, i);
        if (memcmp(frame->id, "COMM", 4) == 0) {
            return frame;
        }
    }
    return NULL;
}

int addCommentV2dot3(FILE* file, char* comment, struct ID3v2dot3MetaData* data){
    struct ID3v2dot3Frame* commentFrame = commentExists(data->frames);
    if (commentFrame) {
        data->whiteSpace += updateComment(commentFrame, comment);

    } else {
        commentFrame = createComment(comment);
        // if (!commentFrame){
        //     return -1;
        // }
        data->whiteSpace -= commentFrame->size;
        dynarray_insert(data->frames, (void*)commentFrame);
    }
    //updateFile(file, data);
    return 0;
}