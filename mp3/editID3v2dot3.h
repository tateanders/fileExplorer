#ifndef __EDITTD3V2DOT3_H
#define __EDITTD3V2DOT3_H

struct ID3v2dot3Header;
struct ID3v2dot3ExtendedHeader;
struct ID3v2dot3Frame;
struct ID3v2dot3MetaData;

void freeDataV2dot3(struct ID3v2dot3MetaData* data);
struct ID3v2dot3MetaData* getMetaDataV2dot3(FILE* file);

#endif