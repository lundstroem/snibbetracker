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

#include <stdbool.h>

#ifndef snibbetracker_file_settings_h
#define snibbetracker_file_settings_h

struct FileSettings {

    bool file_editor_save;
    int file_cursor_y;
    char **file_dirs;
    int file_dir_max_length;
    int file_name_max_length;
    char **file_path_list;
    char *file_path;
    int file_path_max_length;
    int file_path_pos;
    int file_cursor_y_saved[256];
    bool reload_dirs;
    char *file_name;
    int file_name_limit;
    bool file_enter_pressed;
    bool file_moved_in_list;
};

struct PeekDirResult {
    int status;
    char *path;
};

#endif
