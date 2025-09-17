#ifndef __READID3V2DOT3_H
#define __READID3V2DOT3_H

#include <stdint.h>

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
struct ID3v2dot3ExtendedHeader {
    //main data that gets read
    uint32_t size;
    uint16_t flags;
    uint32_t paddingSize;
    uint32_t crc;
    //data after interpretation
    uint8_t crcFlag;
};
struct ID3v2dot3Frame {
    char id[4];
    uint32_t size;
    uint16_t flags;
    uint8_t* data;
};
struct ID3v2dot3MetaData {
    struct ID3v2dot3Header* header;
    struct ID3v2dot3ExtendedHeader* exHeader;
    struct dynarray* frames;
    ssize_t padding;
};

// size_t getTotalSpace(struct ID3v2dot3MetaData* data);
void freeDataV2dot3(struct ID3v2dot3MetaData* data);
struct ID3v2dot3MetaData* getMetaDataV2dot3(FILE* file);
void printMetaData(struct ID3v2dot3MetaData* data);

#endif