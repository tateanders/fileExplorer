#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include "list.h"
#include "dynarray.h"

/*-------------------------------------------------------------------------------------------------
    Structs
-------------------------------------------------------------------------------------------------*/

struct directory {
    char* dirName;
    struct list* directories;
    struct dynarray* songs;
};

struct song {
    char* songName;
    char* fileName;
};

/*-------------------------------------------------------------------------------------------------
    Free functions
-------------------------------------------------------------------------------------------------*/
void freeSongs(struct dynarray* songs){
    struct song* song;
    int i;
    for (i = 0; i < dynarray_size(songs); i++){
        song = (struct song*)dynarray_get(songs, i);
        free(song->fileName);
        free(song->songName);
        free(song);
    }
    dynarray_free(songs);
    return;
}

void freeDirectory(struct directory* directory){
    if(directory->songs){
        freeSongs(directory->songs);
    }
    if(directory->directories){
        struct directory* dir2 = (struct directory*)list_remove_head(directory->directories);
        while(dir2){
            freeDirectory(dir2);
            dir2 = (struct directory*)list_remove_head(directory->directories);
        }
        list_free(directory->directories);
    }
    free(directory->dirName);
    free(directory);
    return;
}

/*-------------------------------------------------------------------------------------------------
    Helper functions
-------------------------------------------------------------------------------------------------*/

DIR* openMusicDir(char* music){
    DIR* musicDir = opendir(music);
    if (!musicDir) {
        perror("no \"music\" directory found on desktop\n");
        exit(1);
    }
    chdir(music);
    return musicDir; 
}

int isDirectory(struct dirent* entry){
    if (entry->d_type == DT_DIR) {
        return 1;
    } else if (entry->d_type == DT_REG) {
        return 0;
    }
    printf("Invalid format for '%s' (retruning -1)\n", entry->d_name);
    return -1;
}

/*-------------------------------------------------------------------------------------------------
    Song functions
-------------------------------------------------------------------------------------------------*/

struct song* createSong(struct dirent* entry){
    struct song* song = (struct song*) calloc(1, sizeof(struct song));
    song->fileName = (char*) calloc(strlen(entry->d_name) + 1, sizeof(char));
    strcpy(song->fileName, entry->d_name);
    //get the songs name and null terminate it
    const char* dot = strrchr(song->fileName, '.');
    size_t len = (size_t) (dot - song->fileName);
    song->songName = (char*) calloc(len + 1, sizeof(char));
    strncpy(song->songName, song->fileName, len);
    song->songName[len] = '\0';
    return song;
}

/*-------------------------------------------------------------------------------------------------
    Directory functions
-------------------------------------------------------------------------------------------------*/

struct directory* createDirectory(char* dirName){
    //create the directory
    struct directory* directory = (struct directory*) calloc(1, sizeof(struct directory));
    directory->dirName = (char*) calloc(strlen(dirName) + 1, sizeof(char));
    strcpy(directory->dirName, dirName);
    //populate the user
    directory->directories = NULL;
    directory->songs = NULL;
    return directory;
}

struct directory* fillDirectory(DIR* dir, char* dirName){
    struct directory* directory = createDirectory(dirName);
    struct dirent* entry;
    do {
        //read the entry
        entry = readdir(dir);
        //if the entry exists and is not a hidden file
        if (entry && (entry->d_name[0] != '.')){
            if(isDirectory(entry)){ //if this entry is a directory
                //if a directories list doesnt already exist
                if (!directory->directories){
                    directory->directories = (struct list*) list_create();
                }
                //create and insert a new directory
                DIR* newDirectory = opendir(entry->d_name);
                chdir(entry->d_name);
                struct directory* newDir = fillDirectory(newDirectory, (char*)entry->d_name);
                list_insert(directory->directories, newDir);
                chdir("..");
                closedir(newDirectory);

            } else if (isDirectory(entry) == 0){ //if this entry is a song
                //if the songs array doesnt exist
                if (!directory->songs){
                    directory->songs = (struct dynarray*) dynarray_create();
                }
                //create and insert a new song
                struct song* song = createSong(entry);
                dynarray_insert(directory->songs, song);
            }
        }
    } while (entry);
    return directory;
}

/*-------------------------------------------------------------------------------------------------
    Main
-------------------------------------------------------------------------------------------------*/

int main(int argc, char* args[]) {
    //enter music directory
    chdir("..");
    char* musicDirName = "music";
    DIR* musicDir = openMusicDir(musicDirName); 

    //get contents of music directory
    struct directory* music = fillDirectory(musicDir, musicDirName);
    closedir(musicDir);
    
    //exit the program
    freeDirectory(music);
    return 0;
}