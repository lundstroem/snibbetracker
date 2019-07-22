/*
 
 MIT License
 
 Copyright (c) 2019 Harry Lundström
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 
 */

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
