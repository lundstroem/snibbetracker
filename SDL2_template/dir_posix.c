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
            char *dir_name_chars = cAllocatorAlloc(sizeof(char)*f->file_name_max_length, "dir name chars 2");
            sprintf(dir_name_chars, "%s", dir->d_name);
            f->file_dirs[pos] = dir_name_chars;
            pos++;
        }
        closedir(d);
    } else {
        return 0;
        printf("dir is null.\n");
    }
    return 1;
}

struct PeekDirResult *peekDirPosix(char *dir_name, struct FileSettings *f) {
    /* return codes
     
     0 null and no .json ending.
     1 null but has .json ending.
     2 not null.
     */
    
    //int return_code = 0;
    struct PeekDirResult *result = cAllocatorAlloc(sizeof(struct PeekDirResult), "FileSettings");;
    result->status = 0;
    result->path = NULL;
    
    // append the dir names to the path.
    char *path = cAllocatorAlloc(sizeof(char)*f->file_name_max_length, "dir name chars 1");
    for(int i = 0; i < f->file_path_pos+1; i++) {
        if(f->file_path_list[i] != NULL) {
            if(i < 2) {
                sprintf(path, "%s%s", path, f->file_path_list[i]);
            } else {
                // Also append delimiter.
                sprintf(path, "%s%c%s", path, '/', f->file_path_list[i]);
            }
        }
    }
    
    // append suggested dir name
    if(f->file_path_pos < 1) {
        sprintf(path, "%s%s", path, dir_name);
    } else {
        // Also append slash.
        sprintf(path, "%s/%s", path, dir_name);
    }
    
    printf("peek path:%s\n", path);
    int status = 0;
    DIR *d = opendir(path);
    if(d) {
        status = 1;
        closedir(d);
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
                char *s_path = cAllocatorAlloc(sizeof(char)*f->file_path_max_length, "file path chars dir_posix");
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
