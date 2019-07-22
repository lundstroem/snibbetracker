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

#import <Foundation/Foundation.h>
#import <stdlib.h>
#import "osx_settings.h"

void copy_demo_songs(const char *path) {
    NSString *demo_songs_key = @"demo_songs_added";
    NSString *demo_songs_added = [[NSUserDefaults standardUserDefaults] stringForKey:demo_songs_key];
    if (demo_songs_added == nil) {
        load_and_save_demo_from_bundle(path, "kissemisse");
        load_and_save_demo_from_bundle(path, "fiskbolja");
        load_and_save_demo_from_bundle(path, "catslayer");
        load_and_save_demo_from_bundle(path, "laptopmidi");
        load_and_save_demo_from_bundle(path, "korvhastig");
        load_and_save_demo_from_bundle(path, "websnacks");
        load_and_save_demo_from_bundle(path, "horizon");
        load_and_save_demo_from_bundle(path, "dunsa2");
        load_and_save_demo_from_bundle(path, "wrestchest");
        load_and_save_demo_from_bundle(path, "projectcart");
        [[NSUserDefaults standardUserDefaults] setObject:@"true" forKey:demo_songs_key];
    }
}

void load_and_save_demo_from_bundle(const char *path, const char *name) {
    NSString *ns_name = [NSString stringWithUTF8String:name];
    NSString *bundle_path = [[NSBundle mainBundle] pathForResource:ns_name ofType:@"snibb"];
    if(bundle_path != nil) {
        if(bundle_path != nil) {
            NSData *data = [NSData dataWithContentsOfFile:bundle_path];
            if(data != nil) {
                NSString *file_name = [NSString stringWithUTF8String:name];
                NSString *file_path = [NSString stringWithUTF8String:path];
                file_path = [file_path stringByAppendingString:file_name];
                file_path = [file_path stringByAppendingString:@".snibb"];
                NSFileHandle *output = [NSFileHandle fileHandleForWritingAtPath:file_path];
                if(output == nil) {
                    [[NSFileManager defaultManager] createFileAtPath:file_path contents:data attributes:nil];
                    output = [NSFileHandle fileHandleForWritingAtPath:file_path];
                } else {
                    NSLog(@"error: could not create file handle for path:%@", file_path);
                }
            }
        }
    }
}

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
        //printf("osx config:%s", my_str);
        return my_str;

    }
    return NULL;
}
