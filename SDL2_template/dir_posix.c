//
//  dir_posix.c
//  snibbetracker
//
//  Created by Harry Lundstrom on 09/03/15.
//  Copyright (c) 2015 D. All rights reserved.
//

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
            if(length > 4) {
                if(dir->d_name[length-2] == 'a') {
                    //wav file detected.
                    if(dir->d_name[length-1] == 'v' && dir->d_name[length-2] == 'a' && dir->d_name[length-3] == 'w' && dir->d_name[length-4] == '.') {
                        add_file = true;
                        printf("wav: %c%c%c%c\n", dir->d_name[length-4], dir->d_name[length-3], dir->d_name[length-2], dir->d_name[length-1]);
                    }
                } else if(length > 6) {
                    // check for snibb
                    if(dir->d_name[length-1] == 'b' && dir->d_name[length-2] == 'b' && dir->d_name[length-3] == 'i' && dir->d_name[length-4] == 'n' && dir->d_name[length-5] == 's' && dir->d_name[length-6] == '.') {
                        add_file = true;
                    }
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
