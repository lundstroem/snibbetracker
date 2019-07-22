//
//  osx_settings.h
//  snibbetracker
//
//  Created by Harry Lundstrom on 07/05/15.
//  Copyright (c) 2015 D. All rights reserved.
//

#ifndef snibbetracker_osx_settings_h
#define snibbetracker_osx_settings_h
void copy_demo_songs(const char *path);
void load_and_save_demo_from_bundle(const char *path, const char *name);
char *get_settings_json(void);
#endif
