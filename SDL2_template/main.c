
#include <SDL2/SDL.h>
//#include <SDL2_image/SDL_image.h>
#include <stdbool.h>
#include "Game.h"
#include "CInput.h"
#include "CTouch.h"
#include "CSynth.h"
#include "CAllocator.h"
#include "CTimer.h"
#include "chars_gfx.h"
#include "cJSON.h"
#include <stdio.h>
#include "file_settings.h"



#ifdef _WIN64
//define something for Windows (64-bit)
    #define platform_windows
#elif _WIN32
//define something for Windows (32-bit)
    #define platform_windows
#elif __APPLE__
    #include "TargetConditionals.h"
    #if TARGET_OS_IPHONE && TARGET_IPHONE_SIMULATOR
    // define something for simulator
    #elif TARGET_OS_IPHONE
    // define something for iphone
    #else
    #define TARGET_OS_OSX 1
    // define something for OSX
    #define platform_osx
    #include "dir_posix.h"
#endif
#elif __linux
// linux
#elif __unix // all unices not caught above
// Unix
#elif __posix
// POSIX
#endif

bool load_gfx = false;

int screen_width = 1280;
int screen_height = 720;
int fps_print_interval = 0;
int old_time = 0;

#define MAX_TOUCHES 8
#define sheet_width 1024
#define sheet_height 1024
#define fullscreen 0

#define cengine_color_dull_red 0xFF771111
#define cengine_color_red 0xFF992222
#define cengine_color_green 0xFF229922
#define cengine_color_blue 0xFF0000CC
#define cengine_color_black 0xFF222222
#define cengine_color_white 0xFFCCCCCC

#define cengine_color_bg1 0xFF332222
#define cengine_color_bg2 0xFF223322
#define cengine_color_bg3 0xFF222233
#define cengine_color_bg4 0xFF332233
#define cengine_color_bg5 0xFF333322
#define cengine_color_bg6 0xFF223333

char *title = "snibbetracker BETA";

struct CInput *input = NULL;

unsigned int *raster = NULL;
unsigned int **raster2d = NULL;
unsigned int **sheet = NULL;
unsigned int *raw_sheet = NULL;
int width = 256*4;
int height = 144*4;
int s_width = 256*2;
int s_height = 144*2;

bool playing = false;
bool editing = false;
bool modifier = false;

int octave = 2;

int visual_track_width = 30;
int visual_track_height = 16;
int visual_cursor_x = 0;
int visual_cursor_y = 0;

bool pattern_editor = false;
int pattern_cursor_x = 0;
int pattern_cursor_y = 0;

bool instrument_editor = false;
int selected_instrument_id = 0;
int selected_instrument_node_index = 1;

bool file_editor = false;


bool pressed_left = false;
bool pressed_right = false;
bool pressed_up = false;
bool pressed_down = false;

struct CSynthContext *synth = NULL;

struct CTimer *infoTimer = NULL;
char *infoString = NULL;

bool show_tips = true;

struct FileSettings *file_settings = NULL;

static void handle_key_down_file(SDL_Keysym* keysym);
static void exitDir(void);
static void enterDir(void);
static void exitFileEditor(void);
static void initDefaultDirIfNull(void);
static int getDirectoryList(char *dir_string);
static void listDirectory(void);
static void renderFiles(void);
static void addFilenameChar(char c);
static void removeFilenameChar(void);
static void loadProjectFile(char *path);
static void saveProjectFile(void);
static void setInfoTimer(char *string);
static void setInfoTimerWithInt(char *string, int data);
static void initFileSettings(void);

/*
 

 - persistent default dir, need to research platform differences?

 - change colors to make more sense.
 - make flags to diff posix/win and write template code for win.
 
 //done
 - preserve path after save/load to make it default next time when entering.
 - test file name limits so not to crash the program.
 - adjust placement of labels to the infoLabel.
 - use escape overall to move back, remove esc as way to close program.
 */

static void initFileSettings(void) {
    struct FileSettings *f = cAllocatorAlloc(sizeof(struct FileSettings), "FileSettings");
    f->file_editor_save = false;
    f->file_cursor_y = 0;
    f->file_dirs = NULL;
    f->file_dir_max_length = 1024;
    f->file_name_max_length = 256;
    f->file_path_list = NULL;
    f->file_path = NULL;
    f->file_path_max_length = 256;
    f->file_path_pos = 0;
    //f->file_cursor_y_saved[256];
    f->reload_dirs = true;
    f->file_name = NULL;
    f->file_name_limit = 20;
    f->file_enter_pressed = false;
    
    file_settings = f;
    
    int r = 0;
    
    for(r = 0; r < file_settings->file_path_max_length; r++) {
        file_settings->file_cursor_y_saved[r] = 0;
    }
    
    file_settings->file_path_list = (char**) cAllocatorAlloc((file_settings->file_path_max_length * sizeof(char*)), "file path array");
    for(r = 0; r < file_settings->file_path_max_length; r++) {
        file_settings->file_path_list[r] = NULL;
    }
    
    file_settings->file_dirs = (char**) cAllocatorAlloc((file_settings->file_dir_max_length * sizeof(char*)), "file dir array");
    for(r = 0; r < file_settings->file_dir_max_length; r++) {
        file_settings->file_dirs[r] = NULL;
    }
}

static void handle_key_down_file(SDL_Keysym* keysym) {
   
    char c = 0;
    switch( keysym->sym ) {
        case SDLK_a: c = 'a'; addFilenameChar(c); break;
        case SDLK_b: c = 'b'; addFilenameChar(c); break;
        case SDLK_c: c = 'c'; addFilenameChar(c); break;
        case SDLK_d: c = 'd'; addFilenameChar(c); break;
        case SDLK_e: c = 'e'; addFilenameChar(c); break;
        case SDLK_f: c = 'f'; addFilenameChar(c); break;
        case SDLK_g: c = 'g'; addFilenameChar(c); break;
        case SDLK_h: c = 'h'; addFilenameChar(c); break;
        case SDLK_i: c = 'i'; addFilenameChar(c); break;
        case SDLK_j: c = 'j'; addFilenameChar(c); break;
        case SDLK_k: c = 'k'; addFilenameChar(c); break;
        case SDLK_l: c = 'l'; addFilenameChar(c); break;
        case SDLK_m: c = 'm'; addFilenameChar(c); break;
        case SDLK_n: c = 'n'; addFilenameChar(c); break;
        case SDLK_o: c = 'o'; addFilenameChar(c); break;
        case SDLK_p: c = 'p'; addFilenameChar(c); break;
        case SDLK_q: c = 'q'; addFilenameChar(c); break;
        case SDLK_r: c = 'r'; addFilenameChar(c); break;
        case SDLK_s: c = 's'; addFilenameChar(c); break;
        case SDLK_t: c = 't'; addFilenameChar(c); break;
        case SDLK_u: c = 'u'; addFilenameChar(c); break;
        case SDLK_v: c = 'v'; addFilenameChar(c); break;
        case SDLK_w: c = 'w'; addFilenameChar(c); break;
        case SDLK_x: c = 'x'; addFilenameChar(c); break;
        case SDLK_y: c = 'y'; addFilenameChar(c); break;
        case SDLK_z: c = 'z'; addFilenameChar(c); break;
        case SDLK_0: c = '0'; addFilenameChar(c); break;
        case SDLK_1: c = '1'; addFilenameChar(c); break;
        case SDLK_2: c = '2'; addFilenameChar(c); break;
        case SDLK_3: c = '3'; addFilenameChar(c); break;
        case SDLK_4: c = '4'; addFilenameChar(c); break;
        case SDLK_5: c = '5'; addFilenameChar(c); break;
        case SDLK_6: c = '6'; addFilenameChar(c); break;
        case SDLK_7: c = '7'; addFilenameChar(c); break;
        case SDLK_8: c = '8'; addFilenameChar(c); break;
        case SDLK_9: c = '9'; addFilenameChar(c); break;
        case SDLK_PLUS:
            break;
        case SDLK_MINUS:
            break;
        case SDLK_TAB:
            break;
        case SDLK_LGUI:
            break;
        case SDLK_ESCAPE:
                exitFileEditor();
            break;
        case SDLK_RETURN:
            // activate another node in the path.
            if(file_settings->file_editor_save) {
                saveProjectFile();
            } else {
                file_settings->file_enter_pressed = true;
                enterDir();
            }
            break;
        case SDLK_LEFT:
            exitDir();
            break;
        case SDLK_RIGHT:
            enterDir();
            break;
        case SDLK_UP:
            file_settings->file_cursor_y--;
            if(file_settings->file_cursor_y < 0) {
                file_settings->file_cursor_y = 0;
            }
            break;
        case SDLK_DOWN:
            if(file_settings->file_cursor_y < file_settings->file_dir_max_length-1) {
                if(file_settings->file_dirs[file_settings->file_cursor_y+1] != NULL) {
                    file_settings->file_cursor_y++;
                }
            }
            break;
        case SDLK_BACKSPACE:
            if(file_settings->file_editor_save) {
                removeFilenameChar();
            }
            break;
        case SDLK_SPACE:
            break;
        default:
            break;
    }
}

static void exitDir(void) {
    if(file_settings->file_path_pos > 0) {
        if(file_settings->file_path_list[file_settings->file_path_pos] != NULL) {
            file_settings->file_path_list[file_settings->file_path_pos] = cAllocatorFree(file_settings->file_path_list[file_settings->file_path_pos]);
        }
        file_settings->file_path_pos--;
        file_settings->file_cursor_y = file_settings->file_cursor_y_saved[file_settings->file_path_pos];
        file_settings->reload_dirs = true;
    } else {
        printf("cannot go further back.\n");
    }
}

static void enterDir(void) {
    
    struct PeekDirResult *result = NULL;
    if(file_settings->file_path_pos < file_settings->file_path_max_length) {
        initDefaultDirIfNull();
        
        #if defined(platform_osx)
            result = peekDirPosix(file_settings->file_dirs[file_settings->file_cursor_y], file_settings);
            if(result->status == 1) {
                if(result->path != NULL){
                    loadProjectFile(result->path);
                }
            }
        #elif defined(platform_windows)
            // TODO
        #endif
        
        printf("== peek status:%d\n", result->status);
        if(result != NULL && result->status == 2) {
            // advance directory
            file_settings->file_cursor_y_saved[file_settings->file_path_pos] = file_settings->file_cursor_y;
            file_settings->file_path_pos++;
            if(file_settings->file_path_list[file_settings->file_path_pos] != NULL) {
                file_settings->file_path_list[file_settings->file_path_pos] = cAllocatorFree(file_settings->file_path_list[file_settings->file_path_pos]);
            }
            char *path = cAllocatorAlloc(sizeof(char)*file_settings->file_name_max_length, "file path name chars");
            sprintf(path, "%s", file_settings->file_dirs[file_settings->file_cursor_y]);
            file_settings->file_path_list[file_settings->file_path_pos] = path;
            file_settings->file_cursor_y = 0;
            printf("adding another to path:%s path_pos:%d\n", path, file_settings->file_path_pos);
            file_settings->reload_dirs = true;
        } else {
            file_settings->file_enter_pressed = false;
        }
    } else {
        printf("file path max nodes reached, cannot go deeper\n");
    }
    
    if(result != NULL && result->path != NULL) {
        cAllocatorFree(result->path);
    }
    
    if(result != NULL) {
        cAllocatorFree(result);
    }
}

static void exitFileEditor(void) {
    for (int i = 0; i < file_settings->file_dir_max_length; i++) {
        if (file_settings->file_dirs[i] != NULL) {
            file_settings->file_dirs[i] = cAllocatorFree(file_settings->file_dirs[i]);
        }
    }
    
    file_settings->file_path = cAllocatorFree(file_settings->file_path);
    file_settings->file_name = cAllocatorFree(file_settings->file_name);
    file_editor = false;
    file_settings->file_editor_save = false;
    file_settings->reload_dirs = true;
}


static char *getDefaultDir(void) {
    //TODO windows
    
    // posix
    return "/";
}

static void initDefaultDirIfNull(void) {
    if(file_settings->file_path_list[0] == NULL) {
        printf("setting up file path defaults.\n");
        char *path = cAllocatorAlloc(sizeof(char)*file_settings->file_name_max_length, "file path name chars");
        sprintf(path, "%s", getDefaultDir());
        file_settings->file_path_list[0] = path;
        file_settings->file_path_pos = 0;
    }
}

static char getFilePathDelimiter(void) {
    return '/';
}


static int getDirectoryList(char *dir_string) {
    #if defined(platform_osx)
        return getDirectoryListPosix(dir_string, file_settings);
    #elif defined(platform_windows)
        return 0;
    #endif
}

static void listDirectory(void) {
    
    // Set default values
    if(file_settings->reload_dirs) {
        
        initDefaultDirIfNull();
        
        // append the dir names to the path.
        char *path = cAllocatorAlloc(sizeof(char)*file_settings->file_name_max_length, "dir name chars");
        for(int i = 0; i < file_settings->file_path_pos+1; i++) {
            if(file_settings->file_path_list[i] != NULL) {
                if(i < 2) {
                    sprintf(path, "%s%s", path, file_settings->file_path_list[i]);
                } else {
                    // Also append slash.
                    sprintf(path, "%s/%s", path, file_settings->file_path_list[i]);
                }
            }
        }
        
        // Set the new path
        if(file_settings->file_path != NULL) {
            file_settings->file_path = cAllocatorFree(file_settings->file_path);
        }
        file_settings->file_path = path;
        
        printf("path:%s path_pos:%d\n", path, file_settings->file_path_pos);
        
        getDirectoryList(file_settings->file_path);
        file_settings->reload_dirs = false;
    }
    
    renderFiles();
}

static void renderFiles(void) {
    
    int offset_y = 0;
    for (int i = 0; i < file_settings->file_dir_max_length; i++) {
        if (file_settings->file_dirs[i] != NULL) {
            if(i == file_settings->file_cursor_y) {
                cEngineRenderLabelWithParams(raster2d, file_settings->file_dirs[i], 0, offset_y-file_settings->file_cursor_y+10, cengine_color_green, cengine_color_black);
            } else {
                cEngineRenderLabelWithParams(raster2d, file_settings->file_dirs[i], 0, offset_y-file_settings->file_cursor_y+10, cengine_color_white, cengine_color_black);
            }
        }
        offset_y++;
    }
    
    int offset_x = 0;
    
    if(file_settings->file_path != NULL) {
        cEngineRenderLabelWithParams(raster2d, "path:                                                                                            ", offset_x, 23, cengine_color_red, cengine_color_black);
        cEngineRenderLabelWithParams(raster2d, file_settings->file_path, offset_x+5, 23, cengine_color_red, cengine_color_black);
    }
    
    if(file_settings->file_editor_save) {
        cEngineRenderLabelWithParams(raster2d, "save to file:                                                                                            ", offset_x, 22, cengine_color_red, cengine_color_black);
        if(file_settings->file_name != NULL) {
            cEngineRenderLabelWithParams(raster2d, file_settings->file_name, offset_x+13, 22, cengine_color_red, cengine_color_black);
        }
    }
}

static void addFilenameChar(char c) {
    
    if(file_settings->file_name == NULL) {
        file_settings->file_name = cAllocatorAlloc(sizeof(char)*file_settings->file_name_max_length, "file name chars");
    }
        
    int len = (int)strlen(file_settings->file_name);
    if(len < file_settings->file_name_limit) {
        sprintf(file_settings->file_name, "%s%c", file_settings->file_name, c);
    }
}

static void removeFilenameChar(void) {
    
    if(file_settings->file_name == NULL) {
        file_settings->file_name = cAllocatorAlloc(sizeof(char)*file_settings->file_name_max_length, "file name chars");
    }
    
    int len = (int)strlen(file_settings->file_name);
    if(len > 0) {
        file_settings->file_name[len-1] = '\0';
    }
}

static void loadProjectFile(char *path) {
    
    if(path != NULL) {
        FILE *fp = NULL;
        fp = fopen(path, "rb");
        if(fp != NULL) {
            fseek(fp, 0L, SEEK_END);
            long sz = ftell(fp);
            printf("file size:%ld\n", sz);
            char *b = malloc(sizeof(char)*sz);
            fseek(fp, 0, SEEK_SET);
            fread(b, sz, 1, fp);
         
            if(b != NULL) {
                printf("json_str:%s\n", b);
                int status = cSynthLoadProject(synth, b);
                free(b);
                if(status == 0) {
                    setInfoTimer("error: could not load project");
                } else {
                    setInfoTimer(path);
                    exitFileEditor();
                }
            } else {
                printf("buffer is null\n");
            }
            fclose(fp);
        } else {
            printf("file pointer is null\n");
        }
    } else {
        printf("cannot load, path is null\n");
    }
}

static void saveProjectFile(void) {
    // TODO: set delimiter based on POSIX/WIN.
    // save the '.json' in a separate char array so it can easily be swapped for something else.
    
    if(file_settings->file_name != NULL && file_settings->file_path != NULL) {
        char *save_path = cAllocatorAlloc(sizeof(char)*file_settings->file_name_max_length, "save_path chars");
        sprintf(save_path, "%s%c%s.json", file_settings->file_path, getFilePathDelimiter(), file_settings->file_name);
        
        cJSON *root = cSynthSaveProject(synth);
        if(root != NULL) {
            FILE * fp;
            fp = fopen (save_path, "w+");
            fprintf(fp, "%s", cJSON_PrintUnformatted(root));
            fclose(fp);
            cJSON_Delete(root);
        }

        //sprintf(save_path, "saved file:%s", save_path);
        setInfoTimer(save_path);
        cAllocatorFree(save_path);
        exitFileEditor();
    } else {
        printf("cannot save, filename or path is null\n");
    }
}

static void setInfoTimer(char *string) {
    if(string != NULL) {
        int max_size = file_settings->file_name_max_length;
        int len = (int)strlen(string);
        if(len < max_size) {
            char *info = (char *)cAllocatorAlloc(max_size * sizeof(char), "info timer string");
            sprintf(info, "%s", string);
            if(infoString != NULL) {
                infoString = cAllocatorFree(infoString);
            }
            infoString = info;
            cTimerReset(infoTimer);
        } else {
            printf("setInfoTimerWithInt: string too large\n");
        }
    }
}

static void setInfoTimerWithInt(char *string, int data) {
    if(string != NULL) {
        int max_size = file_settings->file_name_max_length;
        int len = (int)strlen(string);
        if(len < max_size) {
            char *info = (char *)cAllocatorAlloc(max_size * sizeof(char), "info timer with int");
            sprintf(info, "%s:%d", string, data);
            if(infoString != NULL) {
                infoString = cAllocatorFree(infoString);
            }
            infoString = info;
            cTimerReset(infoTimer);
        } else {
            printf("setInfoTimerWithInt: string too large\n");
        }
    }
}

static void updateAndRenderInfo(double dt) {
    if(infoString != NULL) {
        if(!cTimerIsReady(infoTimer)) {
            cTimerAdvance(dt, infoTimer);
            cEngineRenderLabelWithParams(raster2d, infoString, 0, 23, cengine_color_white, cengine_color_black);
        }
    }
}

static void setup_data(void)
{
    int r = 0;
    
    initFileSettings();
    
    // contains an integer for every color/pixel on the screen.
    raster = (unsigned int *) cAllocatorAlloc((s_width*s_height) * sizeof(unsigned int), "main.c raster 1");
    for(r = 0; r < s_width*s_height; r++) {
        raster[r] = 0;
    }
    
    raster2d = cAllocatorAlloc(s_width * sizeof(unsigned int *), "main.c raster 2");
    if(raster2d == NULL) {
        fprintf(stderr, "out of memory\n");
    } else {
        for(int i = 0; i < s_width; i++) {
            raster2d[i] = cAllocatorAlloc(s_height * sizeof(unsigned int), "main.c raster 3");
            if(raster2d[i] == NULL)
            {
                fprintf(stderr, "out of memory\n");
            }
        }
    }
    
    input = cInputNew();
    
    input->touches = cAllocatorAlloc(MAX_TOUCHES * sizeof(struct CTouch*), "main.c touches");
    if(input->touches == NULL) {
        fprintf(stderr, "touchlist out of memory\n");
    }
    int i;
    for(i = 0; i < MAX_TOUCHES; i++) {
        if(input->touches != NULL) {
            input->touches[i] = cTouchNew();
            if(input->touches[i] == NULL) {
                fprintf(stderr, "touchlist out of memory\n");
            }
        }
    }
    
    input->ended_touches = cAllocatorAlloc(MAX_TOUCHES * sizeof(struct CTouch*), "main.c ended touches");
    if(input->ended_touches == NULL) {
        fprintf(stderr, "ended touchlist out of memory\n");
    }
    for(i = 0; i < MAX_TOUCHES; i++) {
        if(input->ended_touches != NULL) {
            input->ended_touches[i] = cTouchNew();
            if(input->ended_touches[i] == NULL) {
                fprintf(stderr, "ended touchlist out of memory\n");
            }
        }
    }
    
    infoTimer = cTimerNew(3000);
    cTimerReset(infoTimer);
    
}


void convertInput(int x, int y) {
    input->mouse_x = x/4;
    input->mouse_y = y/4;
}


int quit = 0;

static void quitGame(int code) {
    raster = cAllocatorFree(raster);
    for(int i = 0; i < s_width; i++) {
        raster2d[i] = cAllocatorFree(raster2d[i]);
    }
    raster2d = cAllocatorFree(raster2d);
    cSynthCleanup(synth);
    cEngineCleanup();
    input = cInputCleanup(input);
    infoTimer = cAllocatorFree(infoTimer);
    
    file_settings->file_dirs = cAllocatorFree(file_settings->file_dirs);

    for (int i = 0; i < file_settings->file_path_max_length; i++) {
        if (file_settings->file_path_list[i] != NULL) {
            file_settings->file_path_list[i] = cAllocatorFree(file_settings->file_path_list[i]);
        }
    }
    
    file_settings->file_path_list = cAllocatorFree(file_settings->file_path_list);
    infoString = cAllocatorFree(infoString);
    file_settings = cAllocatorFree(file_settings);
    
    if(!load_gfx) {
        cAllocatorFree(raw_sheet);
    }
    
    
    printf("quit game\n");
}


int sine_scroll = 0;
static void addTrackNodeWithOctave(int x, int y, bool editing, int tone);
static void checkVisualCursorBounds(void);
static void checkPatternCursorBounds(void);
static bool checkScreenBounds(int x, int y);
static char *getWaveTypeAsChar(int type);
static void changeParam(bool plus);
static void ADSRInvertYRender(double x, double y, int color);

static void addTrackNodeWithOctave(int x, int y, bool editing, int value) {
    int x_count = visual_cursor_x%5;
    
    if(instrument_editor || pattern_editor || !editing) {
        printf("not editing\n");
        // only allow preview of notes in editor
        cSynthAddTrackNode(synth, x, y, false, true, value+(octave*12));
    } else {
        
        if(!editing) {
            printf("not editing\n");
            cSynthAddTrackNode(synth, x, y, false, true, value+(octave*12));
        } else {
            
            if(x_count == 0) {
                cSynthAddTrackNode(synth, x, y, editing, true, value+(octave*12));
                if(editing) {
                    if(!playing) {
                        visual_cursor_y++;
                    }
                    checkVisualCursorBounds();
                }
            }
            
            if(x_count == 1 && editing) {
                cSynthAddTrackNodeParams(synth, x, y, value, -1, -1, -1);
                printf("change instrument value:%d\n", value);
                synth->current_instrument = value;
                
                if(!playing) {
                    visual_cursor_y++;
                    checkVisualCursorBounds();
                }
            }
            
            if(x_count == 2 && editing) {
                // change effect
                cSynthAddTrackNodeParams(synth, x, y, -1, value, -1, -1);
                printf("change effect value:%d\n", value);
                
                if(!playing) {
                    visual_cursor_y++;
                    checkVisualCursorBounds();
                }
            }
            
            if(x_count == 3 && editing) {
                // change param2
                cSynthAddTrackNodeParams(synth, x, y, -1, -1, value, -1);
                printf("change param1 value:%d\n", value);
                
                if(!playing) {
                    visual_cursor_y++;
                    checkVisualCursorBounds();
                }
            }
            
            if(x_count == 4 && editing) {
                // change param1
                cSynthAddTrackNodeParams(synth, x, y, -1, -1, -1, value);
                printf("change param2 value:%d\n", value);
                
                if(!playing) {
                    visual_cursor_y++;
                    checkVisualCursorBounds();
                }
            }
        }
    }
}

static void checkVisualCursorBounds(void) {
    
    if(visual_cursor_x == visual_track_width) {
        visual_cursor_x = 0;
    }
    
    if(visual_cursor_x == -1) {
        visual_cursor_x = visual_track_width-1;
    }
    
    if(visual_cursor_y == visual_track_height) {
        
        if(synth->current_track < synth->active_patterns-1) {
            synth->current_track++;
        } else {
            //rewind
            synth->current_track = 0;
        }
        
        visual_cursor_y = 0;
    }
    
    if(visual_cursor_y == -1) {
        
        if(synth->current_track > 0) {
            synth->current_track--;
        } else {
            //move to last pattern
            synth->current_track = synth->active_patterns-1;
        }
        
        visual_cursor_y = visual_track_height-1;
    }
    
    //printf("current track:%d", synth->current_track);
}

static void checkPatternCursorBounds(void) {
    
    if(pattern_cursor_x == synth->patterns_and_voices_width) {
        pattern_cursor_x = 0;
    }
    
    if(pattern_cursor_x == -1) {
        pattern_cursor_x = synth->patterns_and_voices_width-1;
    }
    
    if(pattern_cursor_y == synth->patterns_and_voices_height) {
        pattern_cursor_y = 0;
    }
    
    if(pattern_cursor_y == -1) {
        pattern_cursor_y = synth->patterns_and_voices_height-1;
    }
    
    printf("pattern cursor y:%d\n", pattern_cursor_y);
}

static bool checkScreenBounds(int x, int y) {
    if(x > -1 && x < s_width && y > -1 && y < s_height) {
        return true;
    } else {
        return false;
    }
}

void handle_key_up( SDL_Keysym* keysym )
{
    switch( keysym->sym ) {
        case SDLK_LGUI:
            modifier = false;
            //printf("modifier off");
            break;
        case SDLK_LEFT:
            pressed_left = false;
            break;
        case SDLK_RIGHT:
            pressed_right = false;
            break;
        case SDLK_UP:
            pressed_up = false;
            break;
        case SDLK_DOWN:
            pressed_down = false;
            break;
    }
}

void handleNoteKeys( SDL_Keysym* keysym ) {
    
    switch( keysym->sym ) {
        case SDLK_z:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 12);
            break;
        case SDLK_s:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 13);
            break;
        case SDLK_x:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 14);
            break;
        case SDLK_d:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 15);
            break;
        case SDLK_c:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 16);
            break;
        case SDLK_v:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 17);
            break;
        case SDLK_g:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 18);
            break;
        case SDLK_b:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 19);
            break;
        case SDLK_h:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 20);
            break;
        case SDLK_n:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 21);
            break;
        case SDLK_j:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 22);
            break;
        case SDLK_m:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 23);
            break;
        case SDLK_COMMA:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 24);
            break;
        case SDLK_l:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 25);
            break;
        case SDLK_PERIOD:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 26);
            break;
            
            //upper keyboard
        case SDLK_q:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 24);
            break;
        case SDLK_2:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 25);
            break;
        case SDLK_w:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 26);
            break;
        case SDLK_3:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 27);
            break;
        case SDLK_e:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 28);
            break;
        case SDLK_r:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 29);
            break;
        case SDLK_5:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 30);
            break;
        case SDLK_t:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 31);
            break;
        case SDLK_6:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 32);
            break;
        case SDLK_y:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 33);
            break;
        case SDLK_7:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 34);
            break;
        case SDLK_u:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 35);
            break;
        case SDLK_i:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 36);
            break;
        case SDLK_9:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 37);
            break;
        case SDLK_o:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 38);
            break;
        case SDLK_0:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 39);
            break;
        case SDLK_p:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 40);
            break;
            
        default:
            break;
    }
}




void handlePatternKeys( SDL_Keysym* keysym ) {
    
    if(pattern_cursor_y > 0 && pattern_cursor_y < 17) {
        // pattern nr.
        int pattern = 0;

        switch( keysym->sym ) {
            case SDLK_0:
                pattern = 0;
                break;
            case SDLK_1:
                pattern = 1;
                break;
            case SDLK_2:
                pattern = 2;
                break;
            case SDLK_3:
                pattern = 3;
                break;
            case SDLK_4:
                pattern = 4;
                break;
            case SDLK_5:
                pattern = 5;
                break;
            case SDLK_6:
                pattern = 6;
                break;
            case SDLK_7:
                pattern = 7;
                break;
            case SDLK_8:
                pattern = 8;
                break;
            case SDLK_9:
                pattern = 9;
                break;
            default:
                return;
                break;
        }
        
        if(pattern > 9) {
            pattern = 9;
        } else if(pattern < 0){
            pattern = 0;
        }
        
        synth->patterns_and_voices[pattern_cursor_x][pattern_cursor_y] = pattern;
    }
}



void handleInstrumentKeys( SDL_Keysym* keysym ) {
    
    switch( keysym->sym ) {
        case SDLK_0:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 0);
            break;
        case SDLK_1:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 1);
            break;
        case SDLK_2:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 2);
            break;
        case SDLK_3:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 3);
            break;
        case SDLK_4:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 4);
            break;
        case SDLK_5:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 5);
            break;
        case SDLK_6:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 6);
            break;
        case SDLK_7:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 7);
            break;
        case SDLK_8:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 8);
            break;
        case SDLK_9:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 9);
            break;
        default:
            break;
    }
}

void handleEffectKeys( SDL_Keysym* keysym ) {
    
    switch( keysym->sym ) {
        case SDLK_a:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 10);
            break;
        case SDLK_b:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 11);
            break;
        case SDLK_c:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 12);
            break;
        case SDLK_d:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 13);
            break;
        case SDLK_e:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 14);
            break;
        case SDLK_f:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 15);
            break;
        case SDLK_0:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 0);
            break;
        case SDLK_1:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 1);
            break;
        case SDLK_2:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 2);
            break;
        case SDLK_3:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 3);
            break;
        case SDLK_4:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 4);
            break;
        case SDLK_5:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 5);
            break;
        case SDLK_6:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 6);
            break;
        case SDLK_7:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 7);
            break;
        case SDLK_8:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 8);
            break;
        case SDLK_9:
            addTrackNodeWithOctave(synth->track_cursor_x, synth->track_cursor_y, editing, 9);
            break;
        default:
            break;
    }
}


void handle_key_down( SDL_Keysym* keysym )
{
    
    
    if(file_editor) {
        handle_key_down_file(keysym);
        return;
    } else {
    
        switch( keysym->sym ) {
            case SDLK_PLUS:
                if(instrument_editor) {
                    
                } else if(pattern_editor) {
                    changeParam(true);
                }
                break;
            case SDLK_MINUS:
                
                if(instrument_editor) {
                    
                } else if(pattern_editor) {
                    changeParam(false);
                }
                break;
            case SDLK_o:
                if(modifier) {
                    file_editor = true;
                    return;
                }
                break;
            case SDLK_s:
                if(modifier) {
                    file_editor = true;
                    file_settings->file_editor_save = true;
                    return;
                }
                break;
            case SDLK_TAB:
                if(instrument_editor) {
                    struct CInstrument *ins = synth->instruments[selected_instrument_id];
                    selected_instrument_node_index++;
                    if(selected_instrument_node_index >= ins->adsr_nodes) {
                        selected_instrument_node_index = 1;
                    }
                    
                    printf("selected_instrument_node_index:%i", selected_instrument_node_index);
                } else {
                    if(pattern_editor) {
                        pattern_editor = false;
                    } else {
                        pattern_editor = true;
                    }
                }
                break;
            case SDLK_LGUI:
                modifier = true;
                //printf("modifier on");
                break;
            case SDLK_ESCAPE:
                //quit = true;
                break;
            case SDLK_RETURN:
                if(playing == 0) {
                    playing = true;
                    if(pattern_editor) {
                        printf("playing from pattern when editing:%d\n", pattern_cursor_y-1);
                        cSynthResetTrackProgress(synth, pattern_cursor_y-1);
                    } else {
                        cSynthResetTrackProgress(synth, synth->current_track);
                        printf("playing from pattern:%d\n", synth->current_track);
                    }
                } else {
                    // note off to all voices when stopping playing.
                    for(int v_i = 0; v_i < synth->max_voices; v_i++) {
                        synth->voices[v_i]->note_on = false;
                    }
                    playing = 0;
                }
                break;
                
            case SDLK_LEFT:
                pressed_left = true;
                if(instrument_editor) {
                    
                } else {
                    if(modifier) {
                        octave--;
                        if(octave < 0) {
                            octave = 0;
                        }
                        setInfoTimerWithInt("octave", octave);
                    } else if(pattern_editor) {
                        pattern_cursor_x--;
                        checkPatternCursorBounds();
                    } else {
                        visual_cursor_x--;
                        checkVisualCursorBounds();
                    }
                }
                break;
            case SDLK_RIGHT:
                pressed_right = true;
                if(instrument_editor) {
                    
                } else {
                    if(modifier) {
                        octave++;
                        if(octave > 7) {
                            octave = 7;
                        }
                        setInfoTimerWithInt("octave", octave);
                    } else if(pattern_editor) {
                        pattern_cursor_x++;
                        checkPatternCursorBounds();
                    } else {
                        visual_cursor_x++;
                        checkVisualCursorBounds();
                    }
                }
                break;
            case SDLK_UP:
                pressed_up = true;
                if(instrument_editor) {
                    
                } else {
                    if(pattern_editor) {
                        pattern_cursor_y--;
                        checkPatternCursorBounds();
                    } else {
                        visual_cursor_y--;
                        checkVisualCursorBounds();
                    }
                }
                break;
            case SDLK_DOWN:
                pressed_down = true;
                if(instrument_editor) {
                    
                } else {
                    if(pattern_editor) {
                        pattern_cursor_y++;
                        checkPatternCursorBounds();
                    } else {
                        visual_cursor_y++;
                        checkVisualCursorBounds();
                    }
                }
                break;
            case SDLK_BACKSPACE:
                if(instrument_editor) {}
                else if(pattern_editor) {}
                else if(editing) {
                    int x_count = visual_cursor_x%5;
                    if(x_count == 0) {
                        cSynthRemoveTrackNode(synth, synth->track_cursor_x, synth->track_cursor_y);
                    } else if(x_count == 1) {
                        cSynthRemoveTrackNodeParams(synth, synth->track_cursor_x, synth->track_cursor_y, true, false, false, false);
                    } else if(x_count == 2) {
                        cSynthRemoveTrackNodeParams(synth, synth->track_cursor_x, synth->track_cursor_y, false, true, false, false);
                    } else if(x_count == 3) {
                        cSynthRemoveTrackNodeParams(synth, synth->track_cursor_x, synth->track_cursor_y, false, false, true, false);
                    } else if(x_count == 4) {
                        cSynthRemoveTrackNodeParams(synth, synth->track_cursor_x, synth->track_cursor_y, false, false, false, true);
                    }
                }
                break;
            case SDLK_SPACE:
                if(instrument_editor) {
                    instrument_editor = false;
                } else {
                    if(pattern_editor) {
                        if(pattern_cursor_y == 17 || pattern_cursor_y == 18) {
                            int ins_nr = pattern_cursor_x;
                            // instruments
                            if(pattern_cursor_y == 18) {
                                ins_nr += 6;
                            }
                            selected_instrument_id = ins_nr;
                            printf("selected instrument:%d\n", ins_nr);
                            if(instrument_editor) {
                                instrument_editor = false;
                                printf("instrument editor false\n");
                            } else {
                                instrument_editor = true;
                                printf("instrument editor true\n");
                            }
                        }
                    } else {
                        if(editing == true) {
                            editing = false;
                            setInfoTimer("editing off");
                        } else {
                            editing = true;
                            setInfoTimer("editing on");
                        }
                    }
                }
                break;
                
            default:
                break;
        }
    }
    
    int x_count = visual_cursor_x%5;
    
    if(pattern_editor) {
        handlePatternKeys(keysym);
        return;
    } else if(x_count == 1 && editing) {
        handleInstrumentKeys(keysym);
        return;
    } else if((x_count > 1 && editing) && (x_count < 5 && editing)) {
        handleEffectKeys(keysym);
        return;
    } else {
        handleNoteKeys(keysym);
    }
}




/*C-1 - D-2
zsxdcvgbhnjm,l.
 
C-2 - E-3
q2w3er5t6y7ui9o0p 
 
 
 
 */
 
static void checkSDLEvents(SDL_Event event) {
    
    while (SDL_PollEvent(&event)) {
        switch(event.type) {
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_KEYDOWN:
                handle_key_down( &event.key.keysym );
                break;
            case SDL_KEYUP:
                handle_key_up( &event.key.keysym );
                break;
            case SDL_MOUSEMOTION:
                //printf("Mouse moved by %d,%d to (%d,%d)\n",
                //           event.motion.xrel, event.motion.yrel,
                //           event.motion.x, event.motion.y);
                convertInput(event.motion.x, event.motion.y);
                if(input->mouse1 == 1) {
                    input->touches[0]->x = event.motion.x/4;
                    input->touches[0]->y = event.motion.y/4;
                }
                if(input->mouse2 == 1) {
                    input->touches[1]->x = event.motion.x/4;
                    input->touches[1]->y = event.motion.y/4;
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                //printf("Mouse button %d pressed at (%d,%d)\n",
                //           event.button.button, event.button.x, event.button.y);
                
                if(event.button.button == SDL_BUTTON_LEFT) {
                    input->mouse1 = true;
                    input->touches[0]->active = true;
                    input->touches[0]->x = event.motion.x/4;
                    input->touches[0]->y = event.motion.y/4;
                    printf("mouse 1 x:%i y:%i\n", input->touches[0]->x, input->touches[0]->y);
                    
                }
                
                if(event.button.button == SDL_BUTTON_RIGHT) {
                    input->mouse2 = true;
                    input->touches[1]->active = true;
                    input->touches[1]->x = event.motion.x;
                    input->touches[1]->y = event.motion.y;
                }
                
                break;
            case SDL_MOUSEBUTTONUP:
                //printf("Mouse button %d pressed at (%d,%d)\n",
                //           event.button.button, event.button.x, event.button.y);
                
                if(event.button.button == SDL_BUTTON_LEFT) {
                    input->mouse1 = 0;
                    input->touches[0]->active = false;
                    input->ended_touches[0]->active = true;
                    input->ended_touches[0]->x = event.motion.x;
                    input->ended_touches[0]->y = event.motion.y;
                }
                
                if(event.button.button == SDL_BUTTON_RIGHT) {
                    input->mouse2 = 0;
                    input->touches[1]->active = false;
                    input->ended_touches[1]->active = true;
                    input->ended_touches[1]->x = event.motion.x;
                    input->ended_touches[1]->y = event.motion.y;
                }
                
                break;
        }
    }
}

static int getDelta(void) {
    int currentTime = SDL_GetTicks();
    int delta = 0;
    //int fps = 0;
    
    if(old_time == 0) {
        old_time = currentTime;
        delta = 16;
    } else {
        delta = currentTime-old_time;
        old_time = currentTime;
        if(delta <= 0) {
            delta = 1;
        }
        //fps = 1000/delta;
    }
    
    if(fps_print_interval == 100) {
        fps_print_interval = 0;
    }
    
    fps_print_interval++;
    return delta;
}

/*** synth stuff */
//const double ChromaticRatio = 1.059463094359295264562;
const double Tao = 6.283185307179586476925;

Uint16 bufferSize = 64; // must be a power of two, decrease to allow for a lower syncCompensationFactor to allow for lower latency, increase to reduce risk of underrun
Uint32 samplesPerFrame; // = sampleRate/frameRate;
Uint32 msPerFrame; // = 1000/frameRate;
double practicallySilent = 0.001;

SDL_atomic_t audioCallbackLeftOff;
Sint32 audioMainLeftOff;
Sint8 audioMainAccumulator;

SDL_AudioDeviceID AudioDevice;
SDL_AudioSpec audioSpec;

SDL_Event event;
SDL_bool running = SDL_TRUE;



void logWavedata(float *floatStream, Uint32 floatStreamLength, Uint32 increment) {
    printf("\n\nwaveform data:\n\n");
    Uint32 i=0;
    for (i=0; i<floatStreamLength; i+=increment)
        printf("%4d:%2.16f\n", i, floatStream[i]);
    printf("\n\n");
}

int testSchedule = 0;
int testScheduleSwitch = 0;

//int count = -1;
//int f_limit = 100;
Sint16 last_sample = 0;

void audioCallback(void *unused, Uint8 *byteStream, int byteStreamLength) {
   
    memset(byteStream, 0, byteStreamLength);
    
    if(quit) {
        return;
    }

    Sint16 *s_byteStream = (Sint16 *)byteStream;
    int remain = byteStreamLength / 2;
    
    if(synth == NULL) {
        printf("audioCallback: synthContext is null, returning.");
        return;
    }
    
    for(int v_i = 0; v_i < synth->max_voices; v_i++) {
        struct CVoice *voice = synth->voices[v_i];
        struct CInstrument *ins = voice->instrument;
        if(voice != NULL && ins != NULL && voice->note_on && voice->instrument != NULL) {
            Uint32 i;
            double d_sampleRate = synth->sample_rate;
            double d_waveformLength = voice->waveform_length;
            
            
            cSynthVoiceApplyEffects(synth, voice);
            
            double delta_phi = (double) (cSynthGetFrequency((double)voice->tone_with_fx) / d_sampleRate * (double)d_waveformLength);
            
            
            for (i = 0; i < remain; i+=2) {
                if(voice->note_on) {
                    
                    double amp = 0;

                    if(voice->waveform == synth->noise_table) {
                        voice->phase_double+=voice->tone_with_fx*2;
                        voice->phase_int = (int)voice->phase_double;
                        if(voice->phase_double >= synth->noise_length) {
                            double diff = voice->phase_double - synth->noise_length;
                            voice->phase_double = diff;
                            voice->phase_int = (int)diff;
                        }
                    } else {
                        cSynthIncPhase(voice, delta_phi);
                    }
                    
                    if(voice->noteoff_slope) {
                        double init_amp = cSynthInstrumentVolumeByPos(ins, voice->adsr_cursor)*ins->volume_scalar;
                        amp = voice->noteoff_slope_value*init_amp;
                        double bpm = synth->bpm;
                        //TODO: needs to be sample rate independent. Currently dependent on 44100Hz.
                        voice->noteoff_slope_value -= bpm * 0.00005;
                        if(voice->noteoff_slope_value < 0) {
                            voice->noteoff_slope_value = 0;
                        }
                    } else {
                        amp = cSynthInstrumentVolumeByPos(ins, voice->adsr_cursor)*ins->volume_scalar;
                    }
                    
                    //TODO: needs to be sample rate independent. Currently dependent on 44100Hz.
                    voice->adsr_cursor += 0.00001;
                    
                    if(voice->lowpass_sweep_up || voice->lowpass_sweep_down) {
                       
                        if(voice->lowpass_sweep_up) {
                            cSynthVoiceApplyLowpassSweep(synth, voice, true);
                        }
                        
                        if(voice->lowpass_sweep_down) {
                            cSynthVoiceApplyLowpassSweep(synth, voice, false);
                        }
                        
                        if(voice->lowpass_next_sample) {
                            if(voice->waveform == synth->noise_table) {
                                if(voice->phase_int < synth->noise_length) {
                                    voice->lowpass_last_sample = voice->waveform[voice->phase_int]*amp;
                                }
                            } else {
                                if(voice->phase_int < synth->wave_length) {
                                    voice->lowpass_last_sample = voice->waveform[voice->phase_int]*amp;
                                }
                            }
                            
                            voice->lowpass_next_sample = false;
                        }
                        
                        s_byteStream[i] += voice->lowpass_last_sample;
                        s_byteStream[i+1] += voice->lowpass_last_sample;
                        
                    } else {
                        if(voice->waveform == synth->noise_table) {
                            if(voice->phase_int < synth->noise_length) {
                                s_byteStream[i] += voice->waveform[voice->phase_int]*amp;
                                s_byteStream[i+1] += voice->waveform[voice->phase_int]*amp;
                            }
                        } else if(voice->phase_int < synth->wave_length) {
                            s_byteStream[i] += voice->waveform[voice->phase_int]*amp;
                            s_byteStream[i+1] += voice->waveform[voice->phase_int]*amp;
                        }
                    }
                }
            }
        }
    }
    
    if(playing == 1) {
        cSynthAdvanceTrack(synth, remain);
    }
}

static void changeWaveform(int plus) {
    
    int current_waveform = synth->patterns_and_voices[pattern_cursor_x][pattern_cursor_y];
    if(plus) {
        current_waveform++;
    } else {
        current_waveform--;
    }
    
    if(current_waveform < 0) {
        current_waveform = 4;
    } else if(current_waveform > 4) {
        current_waveform = 0;
    }
    
    if(current_waveform == 0) {
        synth->voices[pattern_cursor_x]->waveform = synth->sine_wave_table;
    }
    if(current_waveform == 1) {
        synth->voices[pattern_cursor_x]->waveform = synth->sawtooth_wave_table;
    }
    if(current_waveform == 2) {
        synth->voices[pattern_cursor_x]->waveform = synth->square_wave_table;
    }
    if(current_waveform == 3) {
        synth->voices[pattern_cursor_x]->waveform = synth->triangle_wave_table;
    }
    if(current_waveform == 4) {
        synth->voices[pattern_cursor_x]->waveform = synth->noise_table;
    }
    synth->patterns_and_voices[pattern_cursor_x][pattern_cursor_y] = current_waveform;
}


static void changeParam(bool plus) {
    
    int x = pattern_cursor_x;
    int y = pattern_cursor_y;
    
    if(y == 0) {
        changeWaveform(plus);
    } else if(y == 17 || y == 18) {
        //int ins_nr = x;
        // instruments
        if(y == 18) {
            //ins_nr += 6;
        }
    } else if(y == 19 && x == 0) {
        //BPM
        int bpm = synth->bpm;
        if(plus) {
            bpm++;
            synth->bpm = bpm;
        } else {
            bpm--;
            if(bpm < 1) {
                bpm = 1;
            }
            synth->bpm = bpm;
        }
    
    } else if(y == 19 && x == 1) {
        // active patterns
        int active_patterns = synth->active_patterns;
        if(plus) {
            active_patterns++;
            if(active_patterns > 16) {
                active_patterns = 1;
            }
            synth->active_patterns = active_patterns;
        } else {
            active_patterns--;
            if(active_patterns < 1) {
                active_patterns = 16;
            }
            synth->active_patterns = active_patterns;
        }
        
    } else if(y == 19 && x == 2) {
        // active rows for all patterns
        int track_height = synth->track_height;
        if(plus) {
            track_height++;
            if(track_height > synth->track_max_height) {
                track_height = synth->track_max_height;
            }
            synth->track_height = track_height;
            visual_track_height = track_height;
        } else {
            track_height--;
            if(track_height < 16) {
                track_height = 16;
            }
            synth->track_height = track_height;
            visual_track_height = track_height;
        }
        
    } else if(y == 19 && x == 3) {
        if(plus) {
            synth->arpeggio_speed++;
        } else {
            synth->arpeggio_speed--;
            if(synth->arpeggio_speed < 1) {
                synth->arpeggio_speed = 1;
            }
        }
    } else if(y == 19 && x == 4) {
        if(plus) {
            synth->swing++;
        } else {
            synth->swing--;
            if(synth->swing < 0) {
                synth->swing = 0;
            }
        }
        
    }
    else if(y == 19) {
        //nothing
    } else {
        // pattern nr.
        int pattern = synth->patterns_and_voices[pattern_cursor_x][pattern_cursor_y];
        if(plus) {
            pattern++;
        } else {
            pattern--;
        }
        if(pattern > 9) {
            pattern = 0;
        } else if(pattern < 0){
            pattern = 9;
        }
        synth->patterns_and_voices[pattern_cursor_x][pattern_cursor_y] = pattern;
    }
}

static void drawLine(int x0, int y0, int x1, int y1) {
    double g_vec_x = x1 - x0;
    double g_vec_y = y1 - y0;
    double scale_factor = 1.0;
    double length = sqrt( g_vec_x*g_vec_x + g_vec_y*g_vec_y );
    g_vec_x = (g_vec_x/length) * scale_factor;
    g_vec_y = (g_vec_y/length) * scale_factor;
    double pos_x = x0;
    double pos_y = y0;
    int i = 0;
    while (i < length) {
        pos_x += g_vec_x;
        pos_y += g_vec_y;
        int i_pos_x = (int)pos_x;
        int i_pos_y = (int)pos_y;
        ADSRInvertYRender(i_pos_x, i_pos_y, 0xff00ff00);
        i++;
    }
}

static void renderInstrumentEditor(void) {

    struct CInstrument *ins = synth->instruments[selected_instrument_id];
    int max_nodes = ins->adsr_nodes;
    int inset_x = 10;
    int inset_y = 50;
    double speed = 0.01;
    if(modifier) {
        speed = 0.001;
    }
    
    if(selected_instrument_node_index > 0) {
        if (pressed_left) {
                double pos1 = ins->adsr[selected_instrument_node_index-1]->pos;
                double pos2 = ins->adsr[selected_instrument_node_index]->pos;
                if(pos2 > pos1) {
                    ins->adsr[selected_instrument_node_index]->pos -= speed;
                }
                if(pos2 < pos1) {
                    ins->adsr[selected_instrument_node_index]->pos = pos1;
                }
        } else if(pressed_right) {
            if(selected_instrument_node_index < max_nodes-1) {
                double pos1 = ins->adsr[selected_instrument_node_index]->pos;
                double pos2 = ins->adsr[selected_instrument_node_index+1]->pos;
                if(pos1 < pos2) {
                    ins->adsr[selected_instrument_node_index]->pos += speed;
                }
                if(ins->adsr[selected_instrument_node_index]->pos > pos2) {
                    ins->adsr[selected_instrument_node_index]->pos = pos2;
                }
            } else if(selected_instrument_node_index == max_nodes-1) {
                // last node
                ins->adsr[selected_instrument_node_index]->pos += speed;
            }
        }
        
        if(pressed_down) {
            ins->adsr[selected_instrument_node_index]->amp -= speed;
            if(ins->adsr[selected_instrument_node_index]->amp < 0) {
                ins->adsr[selected_instrument_node_index]->amp = 0;
            }
        } else if(pressed_up) {
            ins->adsr[selected_instrument_node_index]->amp += speed;
            if(ins->adsr[selected_instrument_node_index]->amp > 1) {
                ins->adsr[selected_instrument_node_index]->amp = 1;
            }
        }
    }

    double amp_factor = 100;
    double pos_factor = 400;
    
    for(int i = 0; i < 2000; i++) {
        double g_pos = (i*(pos_factor*0.001)) + inset_x;
        int top_line_y = amp_factor + inset_y;
        int bottom_line_y = 0 + inset_y;
        ADSRInvertYRender(g_pos, top_line_y, cengine_color_white);
        ADSRInvertYRender(g_pos, bottom_line_y, cengine_color_white);
    }
    
    for(int i = 0; i < max_nodes-1; i++) {
        struct CadsrNode *node = ins->adsr[i];
        struct CadsrNode *node2 = ins->adsr[i+1];
        double g_amp = (node->amp*amp_factor) + inset_y;
        double g_pos = (node->pos*pos_factor) + inset_x;
        double g_amp2 = (node2->amp*amp_factor) + inset_y;
        double g_pos2 = (node2->pos*pos_factor) + inset_x;
        drawLine((int)g_pos, (int)g_amp, (int)g_pos2, (int)g_amp2);
    }
    
    // render dots for nodes
    for(int i = 0; i < max_nodes; i++) {
        struct CadsrNode *node = ins->adsr[i];
        double g_amp = (node->amp*amp_factor) + inset_y;
        double g_pos = (node->pos*pos_factor) + inset_x;
        for(int x = -2; x < 2; x++) {
            for(int y = -2; y < 2; y++) {
                int color = cengine_color_red;
                if(i == selected_instrument_node_index) {
                    color = cengine_color_green;
                }
                ADSRInvertYRender(g_pos+x, g_amp+y, color);
            }
        }
    }
}

static void ADSRInvertYRender(double x, double y, int color) {
    int i_x = (int)x;
    int i_y = (int)y/-1+170;
    if(checkScreenBounds(i_x, i_y)) {
        raster2d[i_x][i_y] = color;
    }
}

static void renderPatternMapping(void) {
    
    int inset_x = 1;
    int inset_y = 1;
    for (int x = 0; x < synth->patterns_and_voices_width; x++) {
        for (int y = 0; y < synth->patterns_and_voices_height; y++) {
            
            int val = synth->patterns_and_voices[x][y];
            char cval[2];
            sprintf(cval, "%d", val);
            int bg_color = cengine_color_black;
            if(x == pattern_cursor_x && y == pattern_cursor_y) {
                bg_color = cengine_color_red;
            }
            if(y == 0) {
                cEngineRenderLabelWithParams(raster2d, getWaveTypeAsChar(val), x*10+inset_x, y+inset_y, cengine_color_white, bg_color);
            } else if(y == 17 || y == 18) {
                char cval[10];
                int ins_nr = x;
                if(y == 18) { ins_nr += 6; }
                sprintf(cval, "Ins %d", ins_nr);
                cEngineRenderLabelWithParams(raster2d, cval, x*10+inset_x, y+inset_y, cengine_color_white, bg_color);
            } else if(y == 19 && x == 0) {
                char cval[10];
                sprintf(cval, "BPM %d", synth->bpm);
                cEngineRenderLabelWithParams(raster2d, cval, x*10+inset_x, y+inset_y, cengine_color_white, bg_color);
            } else if(y == 19 && x == 1) {
                char cval[20];
                sprintf(cval, "Active %d", synth->active_patterns);
                cEngineRenderLabelWithParams(raster2d, cval, x*10+inset_x, y+inset_y, cengine_color_white, bg_color);
            } else if(y == 19 && x == 2) {
                char cval[20];
                sprintf(cval, "Rows %d", synth->track_height);
                cEngineRenderLabelWithParams(raster2d, cval, x*10+inset_x, y+inset_y, cengine_color_white, bg_color);
            } else if(y == 19 && x == 3) {
                char cval[20];
                sprintf(cval, "Arp %d", synth->arpeggio_speed);
                cEngineRenderLabelWithParams(raster2d, cval, x*10+inset_x, y+inset_y, cengine_color_white, bg_color);
            } else if(y == 19 && x == 4) {
                char cval[20];
                sprintf(cval, "Swing %d", synth->swing);
                cEngineRenderLabelWithParams(raster2d, cval, x*10+inset_x, y+inset_y, cengine_color_white, bg_color);
            }
            else if(y == 19) {
                //nothing
                cEngineRenderLabelWithParams(raster2d, "-", x*10+inset_x, y+inset_y, cengine_color_white, bg_color);
            }
            else {
                if(y <= synth->active_patterns) {
                    bg_color = cengine_color_green;
                    if(x == pattern_cursor_x && y == pattern_cursor_y) {
                        bg_color = cengine_color_red;
                    }
                }
                
                if(y-1 == synth->current_track && playing) {
                    bg_color = cengine_color_red;
                }
                
                cEngineRenderLabelWithParams(raster2d, cval, x*10+inset_x, y+inset_y, cengine_color_white, bg_color);
            }
        }
    }
}

static char *getWaveTypeAsChar(int type) {
    if(type == 0) {
        return "sine";
    }
    if(type == 1) {
        return "saw";
    }
    if(type == 2) {
        return "square";
    }
    if(type == 3) {
        return "tri";
    }
    if(type == 4) {
        return "noise";
    }
    return "error";
}

static void drawWaveTypes(void) {
    
    for (int x = 0; x < synth->patterns_and_voices_width; x++) {
        int val = synth->patterns_and_voices[x][0];
        cEngineRenderLabelWithParams(raster2d, getWaveTypeAsChar(val), 1+x*10, -visual_cursor_y+5, cengine_color_white, cengine_color_black);
    }
}

void renderTrack(void) {
    
    if(instrument_editor && !file_editor) {
        renderInstrumentEditor();
        return;
    } else if(pattern_editor && !file_editor) {
        renderPatternMapping();
        return;
    } else if(file_editor) {
        listDirectory();
        return;
    }
    
    drawWaveTypes();
    
    int x_count = 0;
    int offset_x = 0;
    int inset_x = 1;
    int inset_y = 6;
    
    
    int cursor_x = visual_cursor_x/5;
    cSynthUpdateTrackCursor(synth, cursor_x, visual_cursor_y);
    int track_progress_int = synth->track_progress_int;
    
    if(playing == false) {
        track_progress_int = visual_cursor_y;
    } else {
        visual_cursor_y = track_progress_int;
        checkVisualCursorBounds();
    }
    
    int node_x = -1;
    
    for (int y = 0; y < synth->track_height; y++) {
        offset_x = 0;
        for (int x = 0; x < visual_track_width; x++) {
            
            int bg_color = cengine_color_black;
            if(x >= 0 && x < 5 ) bg_color = cengine_color_bg1;
            if(x >= 5 && x < 10 ) bg_color = cengine_color_bg2;
            if(x >= 10 && x < 15 ) bg_color = cengine_color_bg3;
            if(x >= 15 && x < 20 ) bg_color = cengine_color_bg4;
            if(x >= 20 && x < 25 ) bg_color = cengine_color_bg5;
            if(x >= 25 && x < 30 ) bg_color = cengine_color_bg6;
            
            if(synth->track_progress_int == y && playing == 1) {
                bg_color = cengine_color_green;
            }
            
            if(visual_cursor_x == x && visual_cursor_y == y) {
                if(editing == 1) {
                    bg_color = cengine_color_red;
                } else{
                    bg_color = cengine_color_dull_red;
                }
            }
            
            int pos_y = inset_y+y-track_progress_int;
            if(bg_color == cengine_color_green) {
                // TODO: For some reason, it flips to 7 for a few frames randomly. Needs more investigation. Can it have something with threading to do?
                if (pos_y == 7) {
                    pos_y = 6;
                }
            }
            
            if(x == 0 || x == 5 || x == 10 || x == 15 || x == 20 || x == 25) {
                node_x = floor(x/5);
                int pattern = synth->patterns_and_voices[node_x][synth->current_track+1];
                
                struct CTrackNode *t = synth->track[pattern][node_x][y];
                if(t != NULL && t->tone_active) {
                    int tone = t->tone;
                    char *ctone = cSynthToneToChar(tone);
                    cEngineRenderLabelWithParams(raster2d, ctone, inset_x+x+offset_x, pos_y, cengine_color_white, bg_color);
                } else {
                    cEngineRenderLabelWithParams(raster2d, " - ", inset_x+x+offset_x, pos_y, cengine_color_white, bg_color);
                }
                offset_x += 3;
            } else {
                int pattern = synth->patterns_and_voices[node_x][synth->current_track+1];
                
                struct CTrackNode *t = synth->track[pattern][node_x][y];
                if(t != NULL) {
                    if(x_count == 1) {
                        if(t->instrument != NULL) {
                            char cval[20];
                            sprintf(cval, "%d", t->instrument_nr);
                            cEngineRenderLabelWithParams(raster2d, cval, inset_x+x+offset_x, pos_y, cengine_color_white, bg_color);
                        } else {
                            cEngineRenderLabelWithParams(raster2d, "-", inset_x+x+offset_x, pos_y, cengine_color_white, bg_color);
                        }
                    } else if(x_count == 2) {
                        //effect type
                        char cval[20];
                        sprintf(cval, "%c", t->effect);
                        cEngineRenderLabelWithParams(raster2d, cval, inset_x+x+offset_x, pos_y, cengine_color_white, bg_color);
                    } else if(x_count == 3) {
                        //effect type
                        char cval[20];
                        sprintf(cval, "%c", t->effect_param1);
                        cEngineRenderLabelWithParams(raster2d, cval, inset_x+x+offset_x, pos_y, cengine_color_white, bg_color);
                    } else if(x_count == 4) {
                        //effect type
                        char cval[20];
                        sprintf(cval, "%c", t->effect_param2);
                        cEngineRenderLabelWithParams(raster2d, cval, inset_x+x+offset_x, pos_y, cengine_color_white, bg_color);
                    }
                } else {
                    cEngineRenderLabelWithParams(raster2d, "-", inset_x+x+offset_x, pos_y, cengine_color_white, bg_color);
                }
                
                if(x_count == 1 || x_count == 4) {
                    offset_x++;
                }
            }
            
            x_count++;
            if(x_count == 5) {
                x_count = 0;
            }
        }
    }
}
SDL_Window *window = NULL;
SDL_Texture *texture;
SDL_Renderer *renderer;
SDL_GLContext context;

void setupSDL(void) {
    
    SDL_Init(SDL_INIT_VIDEO);
    
    window = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL);
    context = SDL_GL_CreateContext(window);
    if(context == NULL) {
        printf("\nFailed to create context: %s\n", SDL_GetError());
    }
    SDL_GL_MakeCurrent(window, context);
    
    if (window != NULL) {
        
        renderer = SDL_CreateRenderer(window, -1, 0);
        if (renderer != NULL) {
        
            texture = SDL_CreateTexture(renderer,
                                        SDL_PIXELFORMAT_ARGB8888,
                                        SDL_TEXTUREACCESS_STREAMING,
                                        s_width, s_height);
            
            SDL_GL_SetSwapInterval(1);
            
            char title_string[40];
            sprintf(title_string, "%s (build:%d)", title, synth->build_number);
            SDL_SetWindowTitle(window, title_string);
            
            visual_track_height = synth->track_height;
        } else {
            printf("renderer is null");
        }
    } else {
        printf("window is null");
    }
}

void destroySDL(void) {
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}

int setupSDLAudio(void) {
    SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER);
    SDL_AudioSpec want;
    SDL_zero(want);// btw, I have no idea what this is...
    SDL_zero(audioSpec);// btw, I have no idea what this is...
    
    want.freq = synth->sample_rate;
    want.format = AUDIO_S16LSB;
    want.channels = 2;
    want.samples = bufferSize;
    want.callback = audioCallback;
    
    AudioDevice = SDL_OpenAudioDevice(NULL, 0, &want, &audioSpec, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
    
    printf("   audioSpec\n");
    printf("----------------\n");
    printf("sample rate:%d\n", want.freq);
    printf("channels:%d\n", audioSpec.channels);
    printf("samples:%d\n", audioSpec.samples);
    printf("buffer size:%d\n", audioSpec.size);
    printf("----------------\n");
    
    
    if (AudioDevice == 0) {
        printf("\nFailed to open audio: %s\n", SDL_GetError());
        return 1;
    }
    
    
    if (audioSpec.format != want.format) {
        printf("\nCouldn't get Float32 audio format.\n");
        return 2;
    }
    
    
    //int sampleRate = synth->sample_rate;
    int frameRate = synth->frame_rate;
    
    int sampleRate = audioSpec.freq;
    bufferSize = audioSpec.samples;
    samplesPerFrame = sampleRate/frameRate;
    msPerFrame = 1000/frameRate;
    audioMainLeftOff = samplesPerFrame*8;
    SDL_AtomicSet(&audioCallbackLeftOff, 0);
    
    SDL_PauseAudioDevice(AudioDevice, 0);// unpause audio.
    
    return 0;
}

void setupCSynth(void) {
    setup_data();
    struct CEngineContext *c = cEngineContextNew();
    c->width = s_width;
    c->height = s_height;
    c->sprite_size = 16;
    c->sheet_size = 1024;
    c->max_touches = 8;
    c->level_width = 64;
    c->level_height = 64;
    c->max_buttons = 10;
    c->show_fps = false;
    c->ground_render_enabled = false;
    
    cEngineInit(c);
    
    // load char gfx from png and store in source. Remove this step before release to get rid of depencency.
    SDL_Surface * image = NULL;
    if(load_gfx) {
        /*
        char * filename = "groundtiles.png";
        SDL_Surface * image = IMG_Load(filename);
        raw_sheet = image->pixels;
        printf("\n unsigned int chars_gfx[16384] = {");
        // print label gfx to store in code instead.
        for (int i = 0; i< 16384; i++) {
            printf("%d,", raw_sheet[i]);
        }
        printf("};\n");
         */
    } else {
        // contains an integer for every color/pixel on the screen.
        raw_sheet = (unsigned int *) cAllocatorAlloc((sheet_width*sheet_height) * sizeof(unsigned int), "main.c raw_sheet 1");
        for(int r = 0; r < sheet_width*sheet_height; r++) {
            raw_sheet[r] = 0;
        }
    }
    
    // load gfx from source.
    for (int i = 0; i < c->sheet_size*16; i++) {
        raw_sheet[i] = chars_gfx[i];
    }
    cEngineWritePixelData(raw_sheet);
    synth = cSynthContextNew();
    cSynthInit(synth);
    visual_track_height = synth->track_height;
    
    if(load_gfx) {
        if(image != NULL) {
            SDL_FreeSurface(image);
        }
    }
}

void cleanupSynth(void) {
    printf("allocs before cleanup:\n");
    cAllocatorPrintAllocationCount();
    
    //cSynthSaveProject(synth);
    
    quitGame(0);
    
    printf("allocs after cleanup:\n");
    cAllocatorPrintAllocations();
    cAllocatorPrintAllocationCount();
    
    // If allocation tracking is on, clean up the last stuff.
    cAllocatorCleanup();
}

int error_freq = 0;
void mainLoop(void) {
    checkSDLEvents(event);
    
    for (int r_x = 0; r_x < s_width; r_x++) {
        for (int r_y = 0; r_y < s_height; r_y++) {
            raster[r_x+r_y*s_width] = raster2d[r_x][r_y];
        }
    }
    
    double dt = (double)getDelta();
    cEngineGameloop(dt, raster2d, input);
    
    
    //renderStuff
    
    for(int x = 0; x < s_width; x++) {
        for(int y = 0; y < s_height; y++) {
            raster2d[x][y] = 0;
        }
    }
    
    renderTrack();
    
    updateAndRenderInfo(dt);
    
    SDL_UpdateTexture(texture, NULL, raster, s_width * sizeof (Uint32));
    SDL_RenderClear(renderer);
    
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
    
    SDL_Delay(16);
    
}





int main(int argc, char ** argv)
{
    
    setupCSynth();
    setupSDL();
    setupSDLAudio();
            
    if (texture != NULL) {
        while (!quit) {
            mainLoop();
        }
    }

    cleanupSynth();
            
    //SDL_GL_DeleteContext(mainGLContext);
    destroySDL();
    
    SDL_CloseAudioDevice(AudioDevice);
    SDL_Quit();

    
    return 0;
}



