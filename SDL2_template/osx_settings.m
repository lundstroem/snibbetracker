//
//  osx_settings.m
//  snibbetracker
//
//  Created by Harry Lundstrom on 07/05/15.
//  Copyright (c) 2015 D. All rights reserved.
//

#import <Foundation/Foundation.h>

char *get_settings_json(void) {
    
    NSString *filePath = [[NSBundle mainBundle] pathForResource:@"config" ofType:@"txt"];
    NSError* error = nil;
    NSString* content = [NSString stringWithContentsOfFile:filePath
                                                  encoding:NSUTF8StringEncoding
                                                     error:&error];
    if(error) { // If error object was instantiated, handle it.
        NSLog(@"ERROR while loading from file: %@", error);
    }
    
    if (content) {
        const char *chars = [content UTF8String];
        char *my_str = strdup(chars);
        printf("osx config:%s", my_str);
        return my_str;

    }
    
    return NULL;
}