#ifndef __READID3V2DOT4_H
#define __READID3V2DOT4_H

#include <stdint.h>

struct ID3v2dot4Header {
    //main data that gets read
    char id[3];
    uint8_t version;
    uint8_t revision;
    uint8_t flags;
    uint32_t size;
    //data after interpretation
    uint8_t uFlag; //unsynchronized
    uint8_t eFlag; //extended header
    uint8_t xFlag; //experimental
    uint8_t fFlag; //footer
};
struct ID3v2dot4Frame {
    char id[4];
    uint32_t size; //syncsafe
    uint16_t flags;
    uint8_t* data;
};
struct ID3v2dot4MetaData {
    struct ID3v2dot4Header* header;
    struct dynarray* frames;
    ssize_t padding;
};

void freeDataV2dot4(struct ID3v2dot4MetaData* data);
struct ID3v2dot4MetaData* getMetaDataV2dot4(FILE* file);
void printMetaData4(struct ID3v2dot4MetaData* data);

#endif