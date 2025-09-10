#ifndef __READTD3V2DOT4_H
#define __READTD3V2DOT4_H

#include <stdint.h>

struct ID3v2dot4Header {
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
struct ID3v2dot4ExtendedHeader {
    //main data that gets read
    uint32_t size;
    uint16_t flags;
    uint32_t paddingSize;
    uint32_t crc;
    //data after interpretation
    uint8_t crcFlag;
};
struct ID3v2dot4Frame {
    char id[4];
    uint32_t size;
    uint16_t flags;
    uint8_t* data;
    // size_t totalSize;
};
struct ID3v2dot4MetaData {
    struct ID3v2dot4Header* header;
    struct ID3v2dot4ExtendedHeader* exHeader;
    struct dynarray* frames;
    // size_t size;
    ssize_t padding;
    //long endHeader;
};

// size_t getTotalSpace(struct ID3v2dot4MetaData* data);
void freeDataV2dot3(struct ID3v2dot4MetaData* data);
struct ID3v2dot4MetaData* getMetaDataV2dot3(FILE* file);
void printMetaData(struct ID3v2dot4MetaData* data);

#endif