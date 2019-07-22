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

#include "dir_posix.h"
#include "CAllocator.h"
#include <dirent.h>
#include <string.h>

int getDirectoryListPosix(char *dir_string, struct FileSettings *f) {
    
    // Clear out all stored directories
    for (int i = 0; i < f->file_dir_max_length; i++) {
        if (f->file_dirs[i] != NULL) {
            f->file_dirs[i] = cAllocatorFree(f->file_dirs[i]);
        }
    }
    
    int pos = 0;
    struct dirent *dir;
    DIR *d = opendir(dir_string);
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            int length = (int)strlen(dir->d_name);
            bool add_file = false;
            if(length > 6) {
                // check for snibb
                if(dir->d_name[length-1] == 'b' && dir->d_name[length-2] == 'b' && dir->d_name[length-3] == 'i' && dir->d_name[length-4] == 'n' && dir->d_name[length-5] == 's' && dir->d_name[length-6] == '.') {
                    add_file = true;
                }
            }
            if (add_file) {
                char *dir_name_chars = cAllocatorAlloc(sizeof(char)*f->file_name_max_length, "dir name chars 2");
                sprintf(dir_name_chars, "%s", dir->d_name);
                f->file_dirs[pos] = dir_name_chars;
                pos++;
            }
        }
        closedir(d);
    } else {
        return 0;
        printf("dir is null.\n");
    }
    return 1;
}
