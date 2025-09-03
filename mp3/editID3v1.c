#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include "editID3v1.h"

void readV1Tags(FILE* file, struct mp3Tags** tags){
    
}

/*-------------------------------------------------------------------------------------------------
    Add comment functions for ID3v1
-------------------------------------------------------------------------------------------------*/

    // ID3v1 tag structure (128 bytes total):
    // TAG (3) + Title (30) + Artist (30) + Album (30) + Year (4) + Comment (30) + Genre (1)
    // Comment field starts at byte 97 from the beginning of the tag

// int addCommentID3v1(FILE* file, char* comment) {
//     // ID3v1 tag structure (128 bytes total):
//     // TAG (3) + Title (30) + Artist (30) + Album (30) + Year (4) + Comment (30) + Genre (1)
//     // We'll force ID3v1.1-style comment: use 28 bytes for comment, byte 28 = 0x00 marker, byte 29 = track

//     if (!file || !comment) {
//         printf("Error: Invalid file or comment\n");
//         return 0;
//     }

//     // Seek to the comment field (128 bytes from end, then 97 bytes into the tag)
//     fseek(file, -128 + 97, SEEK_END);

//     // Prepare comment field (30 bytes, null-padded)
//     unsigned char comment_field[30];
//     memset(comment_field, 0, 30); // Fill with null bytes

//     // Copy comment (truncate if longer than 28 chars to leave space for ID3v1.1 marker+track)
//     size_t comment_len = strlen(comment);
//     if (comment_len > 28) {
//         comment_len = 28;
//         printf("Warning: Comment truncated to 28 characters for ID3v1.1\n");
//     }

//     memcpy(comment_field, comment, comment_len);

//     // Enforce ID3v1.1: next-to-last byte = 0x00 marker, last byte = track number (0 = no track)
//     comment_field[28] = 0x00; // marker that a track byte follows
//     comment_field[29] = 0x00; // track number (0 = none). change if you want to set a track.

//     // Write the comment field
//     size_t written = fwrite(comment_field, 1, 30, file);
//     if (written != 30) {
//         printf("Error: Failed to write comment to ID3v1 tag\n");
//         return 0;
//     }

//     printf("ID3v1.1 comment updated successfully\n");
//     return 1;
// }