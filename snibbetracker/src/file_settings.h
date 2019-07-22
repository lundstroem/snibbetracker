//
//  file_settings.h
//  snibbetracker
//
//  Created by Harry Lundstrom on 09/03/15.
//  Copyright (c) 2015 D. All rights reserved.
//

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
