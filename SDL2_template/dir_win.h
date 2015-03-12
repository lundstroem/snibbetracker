//
//  dir_posix.h
//  snibbetracker
//
//  Created by Harry Lundstrom on 09/03/15.
//  Copyright (c) 2015 D. All rights reserved.
//
#include <stdio.h>
#include "file_settings.h"

#ifndef __snibbetracker__dir_win__
#define __snibbetracker__dir_win__

int getDirectoryListWin(char *dir_string, struct FileSettings *f);
struct PeekDirResult *peekDirWin(char *dir_name, struct FileSettings *f);

#endif /* defined(__snibbetracker__dir_posix__) */
