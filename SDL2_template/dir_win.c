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
    sprintf(sPath, "%s*.snibb", dir_string);

    if((hFind = FindFirstFile(sPath, &fdFile)) == INVALID_HANDLE_VALUE) {
        //printf("Path not found: [%s]\n", sPath);
        return 0;
    }
	
	int pos = 0;
	
    do {
        char *dir_name_chars = cAllocatorAlloc(sizeof(char)*f->file_name_max_length, "dir name chars");
        sprintf(dir_name_chars, "%s", fdFile.cFileName);
        f->file_dirs[pos] = dir_name_chars;
        pos++;
    }
    while(FindNextFile(hFind, &fdFile)); //Find the next file.

    FindClose(hFind); //Always, Always, clean things up!

    return 1;
}
