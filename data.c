#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

#include "structures/list.h"
#include "structures/dynarray.h"
#include "metadata.h"

/*-------------------------------------------------------------------------------------------------
    Structs
-------------------------------------------------------------------------------------------------*/

struct directory {
    char* dirName;
    char* dirPath;
    struct list* directories;
    struct dynarray* songs;
};

struct song {
    char* songName;
    char* fileName;
    char* fileType;
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
        free(song->fileType);
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
        struct directory* dir2 = (struct directory*)list_pop(directory->directories);
        while(dir2){
            freeDirectory(dir2);
            dir2 = (struct directory*)list_pop(directory->directories);
        }
        list_free(directory->directories);
    }
    free(directory->dirName);
    free(directory->dirPath);
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

struct song* createSong(struct dirent* entry, char* dirPath){
    struct song* song = (struct song*) calloc(1, sizeof(struct song));
    //set the file name
    song->fileName = (char*) calloc(strlen(entry->d_name) + 1, sizeof(char));
    strcpy(song->fileName, entry->d_name);
    //get the songs name and null terminate it
    const char* dot = strrchr(song->fileName, '.');
    size_t len = (size_t) (dot - song->fileName);
    song->songName = (char*) calloc(len + 1, sizeof(char));
    strncpy(song->songName, song->fileName, len);
    song->songName[len] = '\0';
    //get the songs file extension
    len = (strlen(song->fileName) - strlen(song->songName));
    song->fileType = (char*) calloc(len, sizeof(char));
    strcpy(song->fileType, dot + 1);
    //return
    printf("working %s: %s\n",dirPath , song->songName);
    addComment(entry, dirPath);
    return song;
}

/*-------------------------------------------------------------------------------------------------
    Directory functions
-------------------------------------------------------------------------------------------------*/

void addPath(char* dirName, char** dirPath){
    size_t newLen;
    if (*dirPath){
        newLen = strlen(*dirPath) + strlen(dirName) + 2;
    } else {
        newLen = strlen(dirName) + 2;
    }
    char* newPath = (char*)calloc(newLen, sizeof(char));
    if (*dirPath){
        strcpy(newPath, *dirPath);
        strcat(newPath, dirName);
        strcat(newPath, "/");
        free(*dirPath);
    } else {
        strcpy(newPath, dirName);
        strcat(newPath, "/");
    }
    *dirPath = newPath;
}

struct directory* createDirectory(char* dirName, char* dirPath){
    //create the directory
    struct directory* directory = (struct directory*) calloc(1, sizeof(struct directory));
    //add the name to the directory
    directory->dirName = (char*) calloc(strlen(dirName) + 1, sizeof(char));
    strcpy(directory->dirName, dirName);
    //add the path to the directory
    if (dirPath) {
        directory->dirPath = (char*) calloc(strlen(dirPath) + 1, sizeof(char));
        strcpy(directory->dirPath, dirPath);
    } else {
        directory->dirPath = NULL;
    }
    addPath(dirName, &directory->dirPath);
    directory->directories = NULL;
    directory->songs = NULL;
    return directory;
}

struct directory* fillDirectory(DIR* dir, char* dirName, char* dirPath){
    struct directory* directory = createDirectory(dirName, dirPath);
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
                struct directory* newDir = fillDirectory(newDirectory, (char*)entry->d_name, directory->dirPath);
                list_insert(directory->directories, newDir);
                chdir("..");
                closedir(newDirectory);

            } else if (isDirectory(entry) == 0){ //if this entry is a song
                //if the songs array doesnt exist
                if (!directory->songs){
                    directory->songs = (struct dynarray*) dynarray_create();
                }
                //create and insert a new song
                struct song* song = createSong(entry, directory->dirPath);
                dynarray_insert(directory->songs, song);
            }
        }
    } while (entry);
    return directory;
}

/*-------------------------------------------------------------------------------------------------
    Print functions
-------------------------------------------------------------------------------------------------*/

void printSpaces(int numSpaces){
    int i;
    for (i = 0; i < numSpaces; i++){
        printf(" ");
    }
}

void printSongs(struct dynarray* songs, int numSpaces){
    printSpaces(numSpaces);
    printf("SONGS:\n");
    struct song* song;
    int i;
    for (i = 0; i < dynarray_size(songs); i++){
        printSpaces(numSpaces);
        song = (struct song*) dynarray_get(songs, i);
        printf("TITLE: %s | TYPE: %s\n", song->songName, song->fileType);
    }
}

void printDirectory(struct directory* directory, int numSpaces){
    printSpaces(numSpaces);
    printf("DIRECTORY: %s, PATH: %s\n", directory->dirName, directory->dirPath);
    numSpaces++;
    if(directory->directories) {
        int i;
        for (i = 0; i < list_getNumElements(directory->directories); i++){
            struct directory* dir2 = (struct directory*) list_getElement(directory->directories, i);
            printDirectory(dir2, numSpaces);
        }
    } else if (directory->songs) {
        printSongs(directory->songs, numSpaces);
    }
    numSpaces--;
}

/*-------------------------------------------------------------------------------------------------
    Main
-------------------------------------------------------------------------------------------------*/

int main(int argc, char* args[]) {
    //enter music directory
    chdir("..");
    char* musicDirName = "Music";
    DIR* musicDir = openMusicDir(musicDirName); 

    //get contents of music directory
    struct directory* music = fillDirectory(musicDir, musicDirName, NULL);
    closedir(musicDir);

    //print the contents
    int numSpaces = 0;
    printDirectory(music, numSpaces);
    
    //exit the program
    freeDirectory(music);
    return 0;
}