// /*-------------------------------------------------------------------------------------------------
//     Helper functions
// -------------------------------------------------------------------------------------------------*/

// // Structure to hold ID3v2 header info
// typedef struct {
//     uint8_t major_version;
//     uint8_t revision;
//     uint8_t flags;
//     uint32_t size;
// } ID3v2Header;

// // Function to convert synchsafe integer to regular integer
// uint32_t synchsafe_to_int(uint8_t* bytes) {
//     return (bytes[0] << 21) | (bytes[1] << 14) | (bytes[2] << 7) | bytes[3];
// }

// // Function to convert regular integer to synchsafe integer
// void int_to_synchsafe(uint32_t value, uint8_t* bytes) {
//     bytes[0] = (value >> 21) & 0x7F;
//     bytes[1] = (value >> 14) & 0x7F;
//     bytes[2] = (value >> 7) & 0x7F;
//     bytes[3] = value & 0x7F;
// }

// // Function to read ID3v2 header
// int readID3v2Header(FILE* file, ID3v2Header* header) {
//     uint8_t header_data[10];
    
//     fseek(file, 0, SEEK_SET);
//     if (fread(header_data, 1, 10, file) != 10) {
//         return 0;
//     }
    
//     if (memcmp(header_data, "ID3", 3) != 0) {
//         return 0;
//     }
    
//     header->major_version = header_data[3];
//     header->revision = header_data[4];
//     header->flags = header_data[5];
//     header->size = synchsafe_to_int(header_data + 6);
//     printf("%i | %i | %i | %i\n", header->major_version, header->revision, header->flags, header->size);
    
//     return 1;
// }

// void printHeader(ID3v2Header* header) {
//     printf("%i | %i | %i | %i\n", header->major_version, header->revision, header->flags, header->size);
// }

// // Function to create a COMM frame for Serato compatibility
// uint8_t* createCOMMFrame(char* comment, uint32_t* frame_size, uint8_t id3_version) {
//     size_t comment_len = strlen(comment);
    
//     // COMM frame structure:
//     // Frame header (10 bytes for v2.3/2.4, 6 bytes for v2.2)
//     // Text encoding (1 byte)
//     // Language (3 bytes)  
//     // Short description (variable length + null terminator)
//     // Actual comment text (variable length)
    
//     int header_size = (id3_version >= 3) ? 10 : 6;
//     uint32_t content_size = 1 + 3 + 1 + comment_len; // encoding + language + null + comment
//     *frame_size = header_size + content_size;
    
//     uint8_t* frame = malloc(*frame_size);
//     if (!frame) return NULL;
    
//     uint8_t* ptr = frame;
    
//     // Frame header
//     if (id3_version >= 3) {
//         // ID3v2.3/2.4: 4-byte frame ID + 4-byte size + 2-byte flags
//         memcpy(ptr, "COMM", 4);
//         ptr += 4;
        
//         // Frame size (excluding header) - use synchsafe for v2.4, regular for v2.3
//         if (id3_version == 4) {
//             int_to_synchsafe(content_size, ptr);
//         } else {
//             ptr[0] = (content_size >> 24) & 0xFF;
//             ptr[1] = (content_size >> 16) & 0xFF;
//             ptr[2] = (content_size >> 8) & 0xFF;
//             ptr[3] = content_size & 0xFF;
//         }
//         ptr += 4;
        
//         // Flags
//         ptr[0] = 0;
//         ptr[1] = 0;
//         ptr += 2;
//     } else {
//         // ID3v2.2: 3-byte frame ID + 3-byte size
//         memcpy(ptr, "COM", 3);
//         ptr += 3;
        
//         ptr[0] = (content_size >> 16) & 0xFF;
//         ptr[1] = (content_size >> 8) & 0xFF;
//         ptr[2] = content_size & 0xFF;
//         ptr += 3;
//     }
    
//     // Frame content
//     *ptr++ = 0; // Text encoding: ISO-8859-1 (most compatible with Serato)
    
//     // Language code
//     *ptr++ = 'e';
//     *ptr++ = 'n'; 
//     *ptr++ = 'g';
    
//     // Short description (empty for general comment)
//     *ptr++ = 0;
    
//     // Comment text
//     memcpy(ptr, comment, comment_len);
    
//     return frame;
// }

// // Function to remove existing COMM frames from ID3v2 tag
// int removeCOMMFrames(FILE* file, ID3v2Header* header) {
//     // This is a simplified approach - we'll rebuild the entire tag without COMM frames
    
//     uint8_t* tag_data = malloc(header->size);
//     if (!tag_data) return 0;
    
//     fseek(file, 10, SEEK_SET); // Skip header
//     if (fread(tag_data, 1, header->size, tag_data) != header->size) {
//         free(tag_data);
//         return 0;
//     }
    
//     uint8_t* new_tag = malloc(header->size);
//     if (!new_tag) {
//         free(tag_data);
//         return 0;
//     }
    
//     uint8_t* src = tag_data;
//     uint8_t* dst = new_tag;
//     uint32_t remaining = header->size;
//     uint32_t new_size = 0;
    
//     while (remaining > 0) {
//         if (remaining < ((header->major_version >= 3) ? 10 : 6)) break;
        
//         // Read frame header
//         int frame_header_size = (header->major_version >= 3) ? 10 : 6;
//         char frame_id[5] = {0};
        
//         if (header->major_version >= 3) {
//             memcpy(frame_id, src, 4);
//             uint32_t frame_size = (header->major_version == 4) ? 
//                 synchsafe_to_int(src + 4) : 
//                 (src[4] << 24) | (src[5] << 16) | (src[6] << 8) | src[7];
            
//             if (strcmp(frame_id, "COMM") != 0) {
//                 // Keep this frame
//                 uint32_t total_frame_size = frame_header_size + frame_size;
//                 memcpy(dst, src, total_frame_size);
//                 dst += total_frame_size;
//                 new_size += total_frame_size;
//             }
            
//             src += frame_header_size + frame_size;
//             remaining -= frame_header_size + frame_size;
//         } else {
//             // ID3v2.2
//             memcpy(frame_id, src, 3);
//             uint32_t frame_size = (src[3] << 16) | (src[4] << 8) | src[5];
            
//             if (strcmp(frame_id, "COM") != 0) {
//                 // Keep this frame
//                 uint32_t total_frame_size = frame_header_size + frame_size;
//                 memcpy(dst, src, total_frame_size);
//                 dst += total_frame_size;
//                 new_size += total_frame_size;
//             }
            
//             src += frame_header_size + frame_size;
//             remaining -= frame_header_size + frame_size;
//         }
//     }
    
//     // Write the new tag data back
//     fseek(file, 10, SEEK_SET);
//     fwrite(new_tag, 1, new_size, file);
    
//     // Update header size
//     header->size = new_size;
    
//     free(tag_data);
//     free(new_tag);
//     return 1;
// }

/*-------------------------------------------------------------------------------------------------
    Add comment functions for ID3v2
-------------------------------------------------------------------------------------------------*/