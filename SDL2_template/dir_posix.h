//
//  dir_posix.h
//  snibbetracker
//
//  Created by Harry Lundstrom on 09/03/15.
//  Copyright (c) 2015 D. All rights reserved.
//

#ifndef __snibbetracker__dir_posix__
#define __snibbetracker__dir_posix__

#include <stdio.h>
#include "file_settings.h"

int getDirectoryListPosix(char *dir_string, struct FileSettings *f);
struct PeekDirResult *peekDirPosix(char *dir_name, struct FileSettings *f);

#endif /* defined(__snibbetracker__dir_posix__) */
