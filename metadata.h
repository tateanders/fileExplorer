#ifndef __EDITMETADATA_H
#define __EDITMETADATA_H

#include "ID3v2dot3/readID3v2dot3.h"
#include "ID3v2dot3/editID3v2dot3.h"
#include "ID3v2dot4/readID3v2dot4.h"
#include "ID3v2dot4/editID3v2dot4.h"

int addComment(struct dirent*, char*);

#endif