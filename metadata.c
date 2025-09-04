#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include "metadata.h"

/*-------------------------------------------------------------------------------------------------
    Structs
-------------------------------------------------------------------------------------------------*/

struct mp3Tags {
    // ID3v1 + ID3v2 common text fields
    char* title;        // TIT2 / Title (30 bytes in ID3v1)
    char* artist;       // TPE1 / Artist (30 bytes in ID3v1)
    char* album;        // TALB / Album (30 bytes in ID3v1)
    char* year;         // TYER (ID3v2.3), TDRC (ID3v2.4), 4 bytes in ID3v1
    char* comment;      // COMM (ID3v2), 30 bytes in ID3v1
    char* genre;        // TCON (ID3v2), 1 byte index in ID3v1

    // Other common ID3v2 frames that Serato/players may use
    char* track;        // TRCK (track number/position in set)
    char* disc;         // TPOS (part of a set / disc number)
    char* composer;     // TCOM
    char* album_artist; // TPE2 (band/orchestra/accompaniment)
    char* publisher;    // TPUB
    char* bpm;          // TBPM (useful for DJs)
    char* key;          // TKEY (musical key, sometimes filled by Serato/Traktor)
    char* grouping;     // GRP1 (grouping / content group description)

    // Less common but valid
    char* copyright;    // TCOP
    char* encoder;      // TENC
    char* url;          // WXXX (user-defined URL)
    char* lyrics;       // USLT (unsynchronized lyrics)

    // Technical info (not always filled)
    int   id3_version;  // 1 = ID3v1, 2 = ID3v2.2, 3 = ID3v2.3, 4 = ID3v2.4
    int   tag_size;     // bytes of tag (for ID3v2)
};

/*-------------------------------------------------------------------------------------------------
    Function to read all of the tags
-------------------------------------------------------------------------------------------------*/

struct mp3Tags* readTags(FILE* file, int ID3v) {
    struct mp3Tags* tags = (struct mp3Tags*)calloc(1, sizeof(struct mp3Tags));
    if (ID3v == 1) {
        readV1Tags(file, &tags);
    }


    return tags;
}

/*-------------------------------------------------------------------------------------------------
    Check format functions
-------------------------------------------------------------------------------------------------*/

int getID3v(FILE* file) {
    char id3v2_header[6]; // "ID3" + major + minor + flags
    fseek(file, 0, SEEK_SET);
    fread(id3v2_header, 1, 6, file);
    
    if (memcmp(id3v2_header, "ID3", 3) == 0) {
        
        if (id3v2_header[3] == 3) {
            return 23;
        } else if (id3v2_header[3] == 4) {
            return 24;
        }
    }
    return 0; // No ID3v2 tag found
}

/*-------------------------------------------------------------------------------------------------
    Main Functions
-------------------------------------------------------------------------------------------------*/

struct mp3Tags* getTags(struct dirent* entry, char* comment){
    //open the file with read and binary permissions
    FILE* file = fopen(entry->d_name, "r+b");
    if (!file) {
        printf("Error opening \"%s\"\n", entry->d_name);
        return NULL;
    }
    printf("Opened: %s\n", entry->d_name);

    //check if it is ID3v1 or ID3v2
    int ID3v = getID3v(file);
    if (!ID3v) {
        return NULL;
    }

    //get the tags
    struct mp3Tags* tags = readTags(file, ID3v);


    fclose(file);
    return tags;
}

int addComment(struct dirent* entry, char* comment){
    //open the file with read and binary permissions
    FILE* file = fopen(entry->d_name, "r+b");
    if (!file) {
        printf("Error opening \"%s\"\n", entry->d_name);
        return 0;
    }

    //get the tag
    int ID3v = getID3v(file);
    if (!ID3v) {
        return 0;
    }

    //add the comment
    // if ((ID3v == 1) || (ID3v == 11)){
    //     addID3v1Comment(file, comment, ID3v);
    // }

    fclose(file);
    return ID3v;
}