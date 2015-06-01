//
//  dir_win.c
//  snibbetracker
//
//  Created by Harry Lundstrom on 12/03/15.
//  Copyright (c) 2015 D. All rights reserved.
//

#include "dir_win.h"
#include "CAllocator.h"
#include <windows.h>
#include <string.h>


/* bool ListDirectoryContents(const char *sDir)
{
    WIN32_FIND_DATA fdFile;
    HANDLE hFind = NULL;

    char sPath[2048];

    //Specify a file mask. *.* = We want everything!
    sprintf(sPath, "%s\\*.*", sDir);

    if((hFind = FindFirstFile(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
    {
        printf("Path not found: [%s]\n", sDir);
        return false;
    }

    do
    {
        //Find first file will always return "."
        //    and ".." as the first two directories.
        if(strcmp(fdFile.cFileName, ".") != 0
                && strcmp(fdFile.cFileName, "..") != 0)
        {
            //Build up our file path using the passed in
            //  [sDir] and the file/foldername we just found:
            sprintf(sPath, "%s\\%s", sDir, fdFile.cFileName);

            //Is the entity a File or Folder?
            if(fdFile.dwFileAttributes &FILE_ATTRIBUTE_DIRECTORY)
            {
                printf("Directory: %s\n", sPath);
                ListDirectoryContents(sPath); //Recursion, I love it!
            }
            else{
                printf("File: %s\n", sPath);
            }
        }
    }
    while(FindNextFile(hFind, &fdFile)); //Find the next file.

    FindClose(hFind); //Always, Always, clean things up!

    return true;
}

ListDirectoryContents("C:\\Windows\\"); */


int getDirectoryListWin(char *dir_string, struct FileSettings *f) {
    
    // Clear out all stored directories
    for (int i = 0; i < f->file_dir_max_length; i++) {
        if (f->file_dirs[i] != NULL) {
            f->file_dirs[i] = cAllocatorFree(f->file_dirs[i]);
        }
    }
    
	WIN32_FIND_DATA fdFile;
    HANDLE hFind = NULL;

    char sPath[2048];

    //Specify a file mask. *.* = We want everything!
    sprintf(sPath, "%s*.json", dir_string);

    if((hFind = FindFirstFile(sPath, &fdFile)) == INVALID_HANDLE_VALUE) {
        printf("Path not found: [%s]\n", sPath);
        return 0;
    }
	
	int pos = 0;
	
    do {
        //Find first file will always return "."
        //    and ".." as the first two directories.
        //if(strcmp(fdFile.cFileName, ".") != 0 && strcmp(fdFile.cFileName, "..") != 0) {
			char *dir_name_chars = cAllocatorAlloc(sizeof(char)*f->file_name_max_length, "dir name chars");
            sprintf(dir_name_chars, "%s", fdFile.cFileName);
            f->file_dirs[pos] = dir_name_chars;
            pos++;
       // }
    }
    while(FindNextFile(hFind, &fdFile)); //Find the next file.

    FindClose(hFind); //Always, Always, clean things up!

    return 1;
}

struct PeekDirResult *peekDirWin(char *dir_name, struct FileSettings *f) {
    /* return codes
     
     0 null and no .json ending.
     1 null but has .json ending.
     2 not null.
     */
    
    //int return_code = 0;
    struct PeekDirResult *result = cAllocatorAlloc(sizeof(struct PeekDirResult), "FileSettings result");
    result->status = 0;
    result->path = NULL;
    
    // append the dir names to the path.
    char *path = cAllocatorAlloc(sizeof(char)*f->file_name_max_length, "dir name chars");
    for(int i = 0; i < f->file_path_pos+1; i++) {
        if(f->file_path_list[i] != NULL) {
            if(i < 1) {
                sprintf(path, "%s%s", path, f->file_path_list[i]);
            } else {
                // Also append delimiter.
                sprintf(path, "%s%c%s", path, '\\', f->file_path_list[i]);
            }
        }
    }
    
    // append suggested dir name
    if(f->file_path_pos < 1) {
        sprintf(path, "%s%s", path, dir_name);
    } else {
        // Also append slash.
        sprintf(path, "%s\%s", path, dir_name);
    }
    
    printf("peek path:%s\n", path);
    int status = 0;
    
	WIN32_FIND_DATA fdFile;
    HANDLE hFind = NULL;
    //char sPath[2048];
    //Specify a file mask. *.* = We want everything!
    //sprintf(sPath, "%s", path);
    if((hFind = FindFirstFile(path, &fdFile)) == INVALID_HANDLE_VALUE) {
        status = 1;
        FindClose(hFind);
    }
    
    if(status == 0 && !f->file_editor_save && f->file_enter_pressed) {
        //Directories returnes 0, check if name ends in .json, in that case try to load!
        int len = (int)strlen(path);
        if(len > 4) {
            if(path[len-1] == 'n'
               && path[len-2] == 'o'
               && path[len-3] == 's'
               && path[len-4] == 'j'
               && path[len-5] == '.') {
                // this is a eligble file if entered with 'enter key'
                
                // save path to load the file.
                char *s_path = cAllocatorAlloc(sizeof(char)*f->file_path_max_length, "file path chars dir_win");
                sprintf(s_path, "%s", path);
				if(path != NULL) {
					path = cAllocatorFree(path);
				}
                result->path = s_path;
                
                f->file_enter_pressed = false;
                
                result->status = 1;
            } else {
                printf("peek filename does not contain .json\n");
                result->status = 0;
            }
        } else {
            printf("filename is not 5 chars or longer\n");
            result->status = 0;
        }
    } else if(status == 1 && !f->file_enter_pressed) {
        // dirs exist
        result->status = 2;
    }
    
    if(path != NULL) {
        cAllocatorFree(path);
    }
    
    return result;
}
