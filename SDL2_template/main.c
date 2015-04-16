

//#include <SDL2_image/SDL_image.h>
#include <stdbool.h>
#include "CEngine.h"
#include "CInput.h"
#include "CTouch.h"
#include "CSynth.h"
#include "CAllocator.h"
#include "CTimer.h"
#include "chars_gfx.h"
#include "cJSON.h"
#include <stdio.h>
#include <stdlib.h>
#include "file_settings.h"
#include <SDL2/SDL.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#ifdef _WIN64
//define something for Windows (64-bit)
    #define platform_windows
#elif _WIN32
//define something for Windows (32-bit)
    #define platform_windows
	#include "dir_win.h"
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
bool log_file_enabled = true;
bool release_build = true;
bool run_with_sdl = true;
bool redraw_screen = true;
int screen_width = 1280;
int screen_height = 720;
int current_pattern = 0;
int current_track = 0;
int quit = 0;
char *title = "snibbetracker test";
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
bool exporting = false;
bool editing = false;
bool modifier = false;
bool shift_down = false;
bool selection_enabled = false;
bool selection_set = false;
bool follow = false;
bool visualiser = false;
bool export_project = false;
int octave = 2;
int visual_pattern_offset = 0;
int visual_track_width = 30;
int visual_track_height = 16;
int visual_cursor_x = 0;
int visual_cursor_y = 0;
int selection_x = 0;
int selection_y = 0;
int last_selection_x = 0;
int last_selection_y = 0;
int last_selection_w = 0;
int last_selection_h = 0;
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
struct CTimer *info_timer = NULL;
char *info_string = NULL;
bool show_tips = true;
struct FileSettings *file_settings = NULL;
int sine_scroll = 0;


#define MAX_TOUCHES 8
#define sheet_width 1024
#define sheet_height 1024
#define fullscreen 0



/*
 TODO: 
 - formatting.
 - delete old filehandling crap
 - add colors to settings file.
 - move logic out of input handler

 
static int color_font = 0xFFCCCCCC;
static int color_font_bg = 0xFF222222;

static int color_cursor_font = 0xFFCCCCCC;
static int color_cursor_bg = 0xFF771111;

static int color_cursor_active_font = 0xFFCCCCCC;
static int color_cursor_active_bg = 0xFF992222;

static int color_playing_row_font = 0xFFCCCCCC;
static int color_playing_row_bg = 0xFF229922;

//static int color_pattern_row_font = 0xFFCCCCCC;
//static int color_pattern_row_bg = 0xFF771111;

static int color_active_pattern_row_font = 0xFFCCCCCC;
static int color_active_pattern_row_bg = 0xFF229922;

static int color_playing_pattern_row_font = 0xFFCCCCCC;
static int color_playing_pattern_row_bg = 0xFF229922;

static int color_instrument_line = 0xFFCCCCCC;
static int color_adsr_line = 0xFF229922;
static int color_adsr_inactive_node = 0xFF771111;
static int color_adsr_active_node = 0xFF229922;
static int color_voice_1_bg = 0xFF332222;
static int color_voice_2_bg = 0xFF223322;
static int color_voice_3_bg = 0xFF222233;
static int color_voice_4_bg = 0xFF332233;
static int color_voice_5_bg = 0xFF333322;
static int color_voice_6_bg = 0xFF223333;
static int color_bg = 0xFF000000;

 */
 
#define cengine_color_dull_red 0xFF771111
#define cengine_color_red 0xFFFF0000
#define cengine_color_green 0xFF00FF00
#define cengine_color_cyan 0xFF00FFFF
#define cengine_color_blue 0xFF0000FF
#define cengine_color_black 0xFF000000
#define cengine_color_white 0xFFCCCCCC
#define cengine_color_magenta 0xFFFF00FF
#define cengine_color_dull_green 0xFF117711

#define cengine_color_bg 0xFF111111
#define cengine_color_bg1 0xFF332222
#define cengine_color_bg2 0xFF223322
#define cengine_color_bg3 0xFF222233
#define cengine_color_bg4 0xFF332233
#define cengine_color_bg5 0xFF333322
#define cengine_color_bg6 0xFF223333


static void init_file_settings(void);
static void handle_key_down_file(SDL_Keysym* keysym);
static void exit_file_editor(void);
static void render_files(void);
static void add_filename_char(char c);
static void remove_filename_char(void);
static char *load_file(char *path);
static void load_project_file(char *path);
static void save_project_file(void);
static void set_info_timer(char *string);
static void set_info_timer_with_int(char *string, int data);
static void update_and_render_info(double dt);
static void setup_data(void);
static void convert_input(int x, int y);
static void cleanup_data(void);
static void copy_notes(int track, int cursor_x, int cursor_y, int selection_x, int selection_y, bool cut, bool store);
static void paste_notes(int track, int cursor_x, int cursor_y);
static void convert_input(int x, int y);
static void add_track_node_with_octave(int x, int y, bool editing, int value);
static void set_visual_cursor(int diff_x, int diff_y, bool user);
static void check_pattern_cursor_bounds(void);
static bool check_screen_bounds(int x, int y);
static void toggle_playback(void);
static void toggle_editing(void);
static void move_notes_up(void);
static void move_notes_down(void);
void handle_key_up(SDL_Keysym* keysym);
void handle_key_down(SDL_Keysym* keysym);
static void handle_note_keys(SDL_Keysym* keysym);
static void handle_pattern_keys(SDL_Keysym* keysym);
static void handle_instrument_keys(SDL_Keysym* keysym);
static void handle_effect_keys(SDL_Keysym* keysym);
static void check_sdl_events(SDL_Event event);
static int get_delta(void);
static void log_wave_data(float *floatStream, Uint32 floatStreamLength, Uint32 increment);
static void render_audio(Sint16 *s_byteStream, int begin, int end, int length);
void audio_callback(void *unused, Uint8 *byteStream, int byteStreamLength);
static void change_waveform(int plus);
static void change_param(bool plus);
static void draw_line(int x0, int y0, int x1, int y1);
static void render_instrument_editor(double dt);
static void adsr_invert_y_render(double x, double y, int color);
static void render_pattern_mapping(void);
static char *get_wave_type_as_char(int type);
static void draw_wave_types(void);
static void render_visuals(void);
static void render_track(double dt);
static void setup_sdl(void);
static void setup_synth(void);
static void setup_texture(void);
static void destroy_sdl(void);
static int setup_sdl_audio(void);
static void setup_cengine(void);
static void cleanup_synth(void);
static void main_loop(void);
static void debug_log(char *str);
static int get_buffer_size_from_index(int i);
static void load_config();
static void st_log(char *message);
static void st_pause(void);
static void add_track_node_with_octave(int x, int y, bool editing, int tone);
static bool check_screen_bounds(int x, int y);
static char *get_wave_type_as_char(int type);
static void change_param(bool plus);
static void export_wav(char *filename);
static void write_little_endian(unsigned int word, int num_bytes, FILE *wav_file);
static void write_wav(char *filename, unsigned long num_samples, short int *data, int s_rate);



static void init_file_settings(void) {
    
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
    f->reload_dirs = true;
    f->file_name = NULL;
    f->file_name_limit = 20;
    f->file_enter_pressed = false;
    
    file_settings = f;
    
    int r = 0;
    
    for(r = 0; r < file_settings->file_path_max_length; r++) {
        file_settings->file_cursor_y_saved[r] = 0;
    }
    
    file_settings->file_path_list = cAllocatorAlloc((file_settings->file_path_max_length * sizeof(char*)), "file path array");
    for(r = 0; r < file_settings->file_path_max_length; r++) {
        file_settings->file_path_list[r] = NULL;
    }
    
    file_settings->file_dirs = cAllocatorAlloc((file_settings->file_dir_max_length * sizeof(char*)), "file dir array");
    for(r = 0; r < file_settings->file_dir_max_length; r++) {
        file_settings->file_dirs[r] = NULL;
    }
}

static void handle_key_down_file(SDL_Keysym* keysym) {
   
    char c = 0;
    switch( keysym->sym ) {
        case SDLK_a: c = 'a'; add_filename_char(c); break;
        case SDLK_b: c = 'b'; add_filename_char(c); break;
        case SDLK_c: c = 'c'; add_filename_char(c); break;
        case SDLK_d: c = 'd'; add_filename_char(c); break;
        case SDLK_e: c = 'e'; add_filename_char(c); break;
        case SDLK_f: c = 'f'; add_filename_char(c); break;
        case SDLK_g: c = 'g'; add_filename_char(c); break;
        case SDLK_h: c = 'h'; add_filename_char(c); break;
        case SDLK_i: c = 'i'; add_filename_char(c); break;
        case SDLK_j: c = 'j'; add_filename_char(c); break;
        case SDLK_k: c = 'k'; add_filename_char(c); break;
        case SDLK_l: c = 'l'; add_filename_char(c); break;
        case SDLK_m: c = 'm'; add_filename_char(c); break;
        case SDLK_n: c = 'n'; add_filename_char(c); break;
        case SDLK_o: c = 'o'; add_filename_char(c); break;
        case SDLK_p: c = 'p'; add_filename_char(c); break;
        case SDLK_q: c = 'q'; add_filename_char(c); break;
        case SDLK_r: c = 'r'; add_filename_char(c); break;
        case SDLK_s: c = 's'; add_filename_char(c); break;
        case SDLK_t: c = 't'; add_filename_char(c); break;
        case SDLK_u: c = 'u'; add_filename_char(c); break;
        case SDLK_v: c = 'v'; add_filename_char(c); break;
        case SDLK_w: c = 'w'; add_filename_char(c); break;
        case SDLK_x: c = 'x'; add_filename_char(c); break;
        case SDLK_y: c = 'y'; add_filename_char(c); break;
        case SDLK_z: c = 'z'; add_filename_char(c); break;
        case SDLK_0: c = '0'; add_filename_char(c); break;
        case SDLK_1: c = '1'; add_filename_char(c); break;
        case SDLK_2: c = '2'; add_filename_char(c); break;
        case SDLK_3: c = '3'; add_filename_char(c); break;
        case SDLK_4: c = '4'; add_filename_char(c); break;
        case SDLK_5: c = '5'; add_filename_char(c); break;
        case SDLK_6: c = '6'; add_filename_char(c); break;
        case SDLK_7: c = '7'; add_filename_char(c); break;
        case SDLK_8: c = '8'; add_filename_char(c); break;
        case SDLK_9: c = '9'; add_filename_char(c); break;
        case SDLK_PLUS:
            break;
        case SDLK_MINUS:
            break;
        case SDLK_TAB:
            break;
        case SDLK_LGUI:
            break;
        case SDLK_LCTRL:
            break;
        case SDLK_ESCAPE:
            exit_file_editor();
            break;
        case SDLK_RETURN:
            if(export_project) {
                // TODO export wav.
                if(file_settings->file_name != NULL) {
                    char *file_name = cAllocatorAlloc(sizeof(char)*file_settings->file_name_max_length, "load_path chars");
                    sprintf(file_name, "%s.wav", file_settings->file_name);
                    export_wav(file_name);
                    cAllocatorFree(file_name);
                    export_project = false;
                    exit_file_editor();
                }
            } else {
                if(file_settings->file_editor_save) {
                    save_project_file();
                } else {
                    if(file_settings->file_name != NULL) {
                        char *load_path = cAllocatorAlloc(sizeof(char)*file_settings->file_name_max_length, "load_path chars");
                        sprintf(load_path, "%s.json", file_settings->file_name);
                        load_project_file(load_path);
                        cAllocatorFree(load_path);
                            
                        // set visual track height
                        visual_track_height = synth->track_height;
                
                    }
                }
            }
            break;
        case SDLK_LEFT:
            break;
        case SDLK_RIGHT:
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
                remove_filename_char();
            break;
        case SDLK_SPACE:
            break;
        default:
            break;
    }
}

static void exit_file_editor(void) {
    
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


static void render_files(void) {
    
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

    if(export_project) {
        cEngineRenderLabelWithParams(raster2d, "  export wav as:                                                                                            ", offset_x, 22, cengine_color_red, cengine_color_black);
    } else {
        cEngineRenderLabelWithParams(raster2d, "save project as:                                                                                            ", offset_x, 22, cengine_color_red, cengine_color_black);
    }
    if(file_settings->file_name != NULL) {
        cEngineRenderLabelWithParams(raster2d, file_settings->file_name, offset_x+16, 22, cengine_color_red, cengine_color_black);
    }
}

static void add_filename_char(char c) {
    
    if(file_settings->file_name == NULL) {
        file_settings->file_name = cAllocatorAlloc(sizeof(char)*file_settings->file_name_max_length, "file name chars");
        file_settings->file_name[0] = '\0';
    }
        
    int len = (int)strlen(file_settings->file_name);
    if(len < file_settings->file_name_limit) {
        char *temp_chars = cAllocatorAlloc(sizeof(char)*file_settings->file_name_max_length, "file name chars");
        sprintf(temp_chars, "%s%c", file_settings->file_name, c);
        file_settings->file_name = cAllocatorFree(file_settings->file_name);
        file_settings->file_name = temp_chars;
    }
}

static void remove_filename_char(void) {
    
    if(file_settings->file_name == NULL) {
        file_settings->file_name = cAllocatorAlloc(sizeof(char)*file_settings->file_name_max_length, "file name chars");
        file_settings->file_name[0] = '\0';
    }
    
    int len = (int)strlen(file_settings->file_name);
    if(len > 0) {
        file_settings->file_name[len] = ' ';
        file_settings->file_name[len-1] = '\0';
        char *temp_chars = cAllocatorAlloc(sizeof(char)*file_settings->file_name_max_length, "file name chars");
        sprintf(temp_chars, "%s", file_settings->file_name);
        file_settings->file_name = cAllocatorFree(file_settings->file_name);
        file_settings->file_name = temp_chars;
    }
}

static char *load_file(char *path) {
    
    if(path != NULL) {
        FILE *fp = NULL;
        fp = fopen(path, "rb");
        if(fp != NULL) {
            fseek(fp, 0L, SEEK_END);
            long sz = ftell(fp);
            printf("file size:%ld\n", sz);
            char *b = cAllocatorAlloc(sizeof(char)*sz, "load file chars");
            fseek(fp, 0, SEEK_SET);
            fread(b, sz, 1, fp);
            
            if(b != NULL) {
                return b;
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
    return NULL;
}

static void load_project_file(char *path) {
    
    char *b = load_file(path);
    if(b != NULL) {
        cSynthReset(synth);
        int status = cSynthLoadProject(synth, b);
        cAllocatorFree(b);
        if(status == 0) {
            set_info_timer("error: could not load project");
        } else {
            set_info_timer(path);
            exit_file_editor();
        }
    } else {
        printf("could not load file.\n");
		set_info_timer("could not load file.");
    }
}

static void save_project_file(void) {

    if(file_settings->file_name != NULL) {
        char *save_path = cAllocatorAlloc(sizeof(char)*file_settings->file_name_max_length, "save_path chars");
        sprintf(save_path, "%s.json", file_settings->file_name);
        cJSON *root = cSynthSaveProject(synth);
        if(root != NULL) {
            FILE * fp;
            fp = fopen (save_path, "w+");
            char *json_print = cJSON_PrintUnformatted(root);
            fprintf(fp, "%s", json_print);
            fclose(fp);
            cJSON_Delete(root);
            free(json_print);
        }
        set_info_timer(save_path);
        cAllocatorFree(save_path);
        exit_file_editor();
    } else {
        printf("cannot save, filename or path is null\n");
    }
}

static void set_info_timer(char *string) {
    
    if(string != NULL) {
        int max_size = file_settings->file_name_max_length;
        int len = (int)strlen(string);
        if(len < max_size) {
            char *info = cAllocatorAlloc(max_size * sizeof(char), "info timer string");
            sprintf(info, "%s", string);
            if(info_string != NULL) {
                info_string = cAllocatorFree(info_string);
            }
            info_string = info;
            cTimerReset(info_timer);
        } else {
            printf("setInfoTimerWithInt: string too large\n");
        }
    }
}

static void set_info_timer_with_int(char *string, int data) {
    
    if(string != NULL) {
        int max_size = file_settings->file_name_max_length;
        int len = (int)strlen(string);
        if(len < max_size) {
            char *info = cAllocatorAlloc(max_size * sizeof(char), "info timer with int");
            sprintf(info, "%s:%d", string, data);
            if(info_string != NULL) {
                info_string = cAllocatorFree(info_string);
            }
            info_string = info;
            cTimerReset(info_timer);
        } else {
            printf("setInfoTimerWithInt: string too large\n");
        }
    }
}

static void update_and_render_info(double dt) {
    
    if(info_string != NULL) {
        if(!cTimerIsReady(info_timer)) {
            cTimerAdvance(dt, info_timer);
            if(cTimerIsReady(info_timer)) {
                redraw_screen = true;
            }
            cEngineRenderLabelWithParams(raster2d, info_string, 0, 23, cengine_color_white, cengine_color_black);
        }
    }
}

static void setup_data(void) {
    
    int i = 0;
    int r_x = 0;
    int r_y = 0;
    
    init_file_settings();
    
    raster = cAllocatorAlloc((s_width*s_height) * sizeof(unsigned int), "main.c raster 1");
    for(i = 0; i < s_width*s_height; i++) {
        raster[i] = 0;
    }
    
    raster2d = cAllocatorAlloc(s_width * sizeof(unsigned int *), "main.c raster 2");
    if(raster2d == NULL) {
        fprintf(stderr, "out of memory\n");
    } else {
        for(i = 0; i < s_width; i++) {
            raster2d[i] = cAllocatorAlloc(s_height * sizeof(unsigned int), "main.c raster 3");
            if(raster2d[i] == NULL)
            {
                fprintf(stderr, "out of memory\n");
            }
        }
    }
    
    for(r_x = 0; r_x < s_width; r_x++) {
        for(r_y = 0; r_y < s_height; r_y++) {
            if(raster2d != NULL && raster2d[r_x] != NULL) {
                raster2d[r_x][r_y] = 0;
            }
        }
    }
    
    input = cInputNew();
    
    input->touches = cAllocatorAlloc(MAX_TOUCHES * sizeof(struct CTouch*), "main.c touches");
    if(input->touches == NULL) {
        fprintf(stderr, "touchlist out of memory\n");
    }
    
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
    
    info_timer = cTimerNew(3000);
    cTimerReset(info_timer);
}


static void convert_input(int x, int y) {
    
    input->mouse_x = x/4;
    input->mouse_y = y/4;
}

static void cleanup_data(void) {
    int i = 0;
    
    raster = cAllocatorFree(raster);
    for(i = 0; i < s_width; i++) {
        raster2d[i] = cAllocatorFree(raster2d[i]);
    }
    raster2d = cAllocatorFree(raster2d);
    cSynthCleanup(synth);
    cEngineCleanup();
    input = cInputCleanup(input);
    info_timer = cAllocatorFree(info_timer);
    
    info_string = cAllocatorFree(info_string);
    
    // File settings --
    for (i = 0; i < file_settings->file_path_max_length; i++) {
        if (file_settings->file_path_list[i] != NULL) {
            file_settings->file_path_list[i] = cAllocatorFree(file_settings->file_path_list[i]);
        }
    }
    file_settings->file_path_list = cAllocatorFree(file_settings->file_path_list);
    
    for (int i = 0; i < file_settings->file_dir_max_length; i++) {
        if (file_settings->file_dirs[i] != NULL) {
            file_settings->file_dirs[i] = cAllocatorFree(file_settings->file_dirs[i]);
        }
    }
    file_settings->file_dirs = cAllocatorFree(file_settings->file_dirs);
    file_settings->file_path = cAllocatorFree(file_settings->file_path);
    file_settings->file_name = cAllocatorFree(file_settings->file_name);
    file_settings = cAllocatorFree(file_settings);
}

static void copy_notes(int track, int cursor_x, int cursor_y, int selection_x, int selection_y, bool cut, bool store) {
    cSynthCopyNotesFromSelection(synth, track, cursor_x, cursor_y, selection_x, selection_y, cut, store);
    last_selection_x = cursor_x;
    last_selection_y = cursor_y;
    last_selection_w = selection_x;
    last_selection_h = selection_y;
    
    /* TODO
    - show green bg color when playing over selection.
    - backspace to cut selection. (no notes should be saved to temp storage)
    - copypaste should not be enabled without editing true?
    - copy paste in pattern view
     */
}

static void paste_notes(int track, int cursor_x, int cursor_y) {
    cSynthPasteNotesToPos(synth, track, cursor_x, cursor_y);
    selection_x = cursor_x + abs(last_selection_x - last_selection_w);
    selection_y = cursor_y + abs(last_selection_y - last_selection_h);
    selection_enabled = true;
}

static void add_track_node_with_octave(int x, int y, bool editing, int value) {
    
    int x_count = visual_cursor_x%5;
    
    if(instrument_editor || pattern_editor || !editing) {
        // only allow preview of notes in editor
        cSynthAddTrackNode(synth, current_track, x, y, false, true, value+(octave*12));
    } else {
        
        if(!editing) {
            cSynthAddTrackNode(synth, current_track, x, y, false, true, value+(octave*12));
        } else {
            
            if(x_count == 0) {
                cSynthAddTrackNode(synth, current_track, x, y, editing, true, value+(octave*12));
                if(editing) {
                    if(playing && follow) {}
                    else {
                        set_visual_cursor(0, 1, true);
                    }
                }
            }
            
            if(x_count == 1 && editing) {
                cSynthAddTrackNodeParams(synth, current_track, x, y, value, -1, -1, -1);
                // change instrument
                synth->current_instrument = value;
                if(playing && follow) {}
                else {
                    set_visual_cursor(0, 1, true);
                }
            }
            
            if(x_count == 2 && editing) {
                // change effect
                cSynthAddTrackNodeParams(synth, current_track, x, y, -1, (char)value, -1, -1);
                if(playing && follow) {}
                else {
                    set_visual_cursor(0, 1, true);
                }
            }
            
            if(x_count == 3 && editing) {
                // change param2
                cSynthAddTrackNodeParams(synth, current_track, x, y, -1, -1, (char)value, -1);
                if(playing && follow) {}
                else {
                    set_visual_cursor(0, 1, true);
                }
            }
            
            if(x_count == 4 && editing) {
                // change param1
                cSynthAddTrackNodeParams(synth, current_track, x, y, -1, -1, -1, (char)value);
                if(playing && follow) {}
                else {
                    set_visual_cursor(0, 1, true);
                }
            }
        }
    }
}

// only move across active tracks
static void set_visual_cursor(int diff_x, int diff_y, bool user) {
    
    if(shift_down) {
        int node_x = (int)floor(visual_cursor_x/5);
        visual_cursor_x = (node_x + diff_x)*5;
    } else {
        visual_cursor_x += diff_x;
    }
    
    visual_cursor_y += diff_y;
    
    if(user) {
        if(visual_cursor_x > visual_track_width-1) {
            visual_cursor_x = 0;
        }
        
        if(visual_cursor_x < 0) {
            visual_cursor_x = visual_track_width-1;
            if(shift_down) {
                visual_cursor_x = visual_track_width-5;
            }
        }
        
        if(visual_cursor_y == visual_track_height) {
            current_track = cSynthGetNextActiveTrack(current_track, synth, true);
            visual_cursor_y = 0;
        }
        
        if(visual_cursor_y == -1) {
            current_track = cSynthGetNextActiveTrack(current_track, synth, false);
            visual_cursor_y = visual_track_height-1;
        }
        
    } else {
        
        if(playing && follow) {
            if(visual_cursor_x == visual_track_width) {
                visual_cursor_x = 0;
            }
            
            if(visual_cursor_x == -1) {
                visual_cursor_x = visual_track_width-1;
            }
            
            if(visual_cursor_y == visual_track_height) {
                synth->current_track = cSynthGetNextActiveTrack(synth->current_track, synth, true);
                visual_cursor_y = 0;
            }
            
            if(visual_cursor_y == -1) {
                synth->current_track = cSynthGetNextActiveTrack(synth->current_track, synth, false);
                visual_cursor_y = visual_track_height-1;
            }
            current_track = synth->current_track;
        }
    }
    
    if(!shift_down) {
        selection_x = visual_cursor_x;
        selection_y = visual_cursor_y;
        selection_enabled = false;
    } else {
        selection_enabled = true;
    }
}


// move across the whole 16 tracks. Keep this commented for now.
//static void setVisualCursor(int diff_x, int diff_y, bool user) {
//    
//    visual_cursor_x += diff_x;
//    visual_cursor_y += diff_y;
//    if(user) {
//        if(visual_cursor_x == visual_track_width) {
//            visual_cursor_x = 0;
//        }
//        
//        if(visual_cursor_x == -1) {
//            visual_cursor_x = visual_track_width-1;
//        }
//        
//        if(visual_cursor_y == visual_track_height) {
//            
//            if(current_track < 15) {
//                current_track++;
//            } else {
//                //rewind
//                current_track = 0;
//            }
//            
//            visual_cursor_y = 0;
//        }
//        
//        if(visual_cursor_y == -1) {
//            
//            if(current_track > 0) {
//                current_track--;
//            } else {
//                //move to last pattern
//                current_track = 15;
//            }
//            
//            visual_cursor_y = visual_track_height-1;
//        }
//        
//    } else {
//        
//        if(playing && follow) {
//            if(visual_cursor_x == visual_track_width) {
//                visual_cursor_x = 0;
//            }
//            
//            if(visual_cursor_x == -1) {
//                visual_cursor_x = visual_track_width-1;
//            }
//            
//            if(visual_cursor_y == visual_track_height) {
//                
//                if(synth->current_track < 15) {
//                    synth->current_track++;
//                } else {
//                    //rewind
//                    synth->current_track = 0;
//                }
//                
//                visual_cursor_y = 0;
//            }
//            
//            if(visual_cursor_y == -1) {
//                
//                if(synth->current_track > 0) {
//                    synth->current_track--;
//                } else {
//                    //move to last pattern
//                    synth->current_track = 15;
//                }
//                
//                visual_cursor_y = visual_track_height-1;
//            }
//            current_track = synth->current_track;
//        }
//    }
//}

static void check_pattern_cursor_bounds(void) {
    
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
}

static bool check_screen_bounds(int x, int y) {
    
    if(x > -1 && x < s_width && y > -1 && y < s_height) {
        return true;
    } else {
        return false;
    }
}

static void toggle_playback(void) {
    
    if(playing == false) {
        playing = true;
        if(pattern_editor) {
            if(pattern_cursor_y > 0 && pattern_cursor_y < synth->patterns_height) {
                cSynthResetTrackProgress(synth, pattern_cursor_y-1+visual_pattern_offset, 0);
            } else {
                cSynthResetTrackProgress(synth, current_track, 0);
            }
        } else {
            cSynthResetTrackProgress(synth, current_track, 0);
        }
    } else {
        // note off to all voices when stopping playing.
        for(int v_i = 0; v_i < synth->max_voices; v_i++) {
            synth->voices[v_i]->note_on = false;
        }
        playing = false;
    }
}

static void toggle_editing(void) {
    
    if(editing == true) {
        editing = false;
        set_info_timer("editing off");
    } else {
        editing = true;
        set_info_timer("editing on");
    }
}

static void move_notes_up(void) {
    
    int cursor_x = visual_cursor_x/5;
    int pattern_at_cursor = synth->patterns[cursor_x][current_track];
    cSynthMoveNotes(synth, true, false, cursor_x, visual_cursor_y, pattern_at_cursor);
}

static void move_notes_down(void) {
    
    int cursor_x = visual_cursor_x/5;
    int pattern_at_cursor = synth->patterns[cursor_x][current_track];
    cSynthMoveNotes(synth, false, true, cursor_x, visual_cursor_y, pattern_at_cursor);
}

void handle_key_up(SDL_Keysym* keysym) {
    
    redraw_screen = true;
    
    switch( keysym->sym ) {
        case SDLK_LGUI:
            modifier = false;
            break;
        case SDLK_LCTRL:
            modifier = false;
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
        case SDLK_LSHIFT:
            shift_down = false;
            break;
        
    }
    
}

void handle_key_down(SDL_Keysym* keysym) {
    
    redraw_screen = true;
    
    if(file_editor) {
        handle_key_down_file(keysym);
        return;
    } else {
        
        switch(keysym->sym) {
            case SDLK_m:
                if(pattern_editor) {
                    if(pattern_cursor_y == 0) {
                        if(synth->voices[pattern_cursor_x]->muted == 1) {
                            synth->voices[pattern_cursor_x]->muted = 0;
                        } else {
                            synth->voices[pattern_cursor_x]->muted = 1;
                        }
                    }
                }
                break;
            case SDLK_PLUS:
                if(instrument_editor) {
                    
                } else if(pattern_editor) {
                    change_param(true);
                }
                break;
            case SDLK_MINUS:
                
                if(instrument_editor) {
                    
                } else if(pattern_editor) {
                    change_param(false);
                }
                break;
            case SDLK_c:
                if(modifier) {
                    if(instrument_editor) {}
                    else if(file_editor) {}
                    else if(pattern_editor) {
                        // TODO make copy/paste for pattern editor.
                    } else {
                        if(editing) {
                            copy_notes(current_track, visual_cursor_x, visual_cursor_y, selection_x, selection_y, false, true);
                            set_info_timer("copy");
                        }
                        return;
                    }
                }
                break;
            case SDLK_x:
                if(modifier) {
                    if(instrument_editor) {}
                    else if(file_editor) {}
                    else if(pattern_editor) {
                        // TODO make copy/paste for pattern editor.
                    } else {
                        if(editing) {
                            copy_notes(current_track, visual_cursor_x, visual_cursor_y, selection_x, selection_y, true, true);
                            set_info_timer("cut");
                        }
                        return;
                    }
                }
                break;
            case SDLK_v:
                if(modifier) {
                    if(instrument_editor) {}
                    else if(file_editor) {}
                    else if(pattern_editor) {
                        // TODO make copy/paste for pattern editor.
                    } else {
                        if(editing) {
                            paste_notes(current_track, visual_cursor_x, visual_cursor_y);
                            set_info_timer("paste");
                        }
                        return;
                    }
                }
                break;
            case SDLK_a:
                if(pattern_editor) {
                    if(pattern_cursor_y > 0) {
                        if(synth->active_tracks[pattern_cursor_y-1+visual_pattern_offset] == 0) {
                            synth->active_tracks[pattern_cursor_y-1+visual_pattern_offset] = 1;
                        } else {
                            synth->active_tracks[pattern_cursor_y-1+visual_pattern_offset] = 0;
                        }
                    }
                }
                break;
            case SDLK_p:
                if(modifier) {
                    if(visualiser) {
                        visualiser = false;
                    } else {
                        visualiser = true;
                    }
                    return;
                }
                break;
            case SDLK_o:
                if(modifier) {
                    file_editor = true;
                    return;
                }
                break;
            case SDLK_e:
                if(modifier) {
                    file_editor = true;
                    file_settings->file_editor_save = true;
                    export_project = true;
                    return;
                }
                if(pattern_editor) {
                        if(pattern_cursor_y > 0 && pattern_cursor_y < 17) {
                            pattern_editor = false;
                            current_track = pattern_cursor_y-1+visual_pattern_offset;
                            visual_cursor_x = pattern_cursor_x*5;
                            set_info_timer("jump to track");
                        }
                    return;
                }
                break;
            case SDLK_s:
                if(modifier) {
                    file_editor = true;
                    file_settings->file_editor_save = true;
                    export_project = false;
                    return;
                } else {
                    if(pattern_editor) {
                        if(pattern_cursor_y > 0) {
                            if(synth->solo_track == pattern_cursor_y-1+visual_pattern_offset) {
                                synth->solo_track = -1;
                            } else {
                                synth->solo_track = pattern_cursor_y-1+visual_pattern_offset;
                            }
                        } else if(pattern_cursor_y == 0) {
                            if(synth->solo_voice == pattern_cursor_x) {
                                synth->solo_voice = -1;
                            } else {
                                synth->solo_voice = pattern_cursor_x;
                            }
                        }
                    }
                }
                break;
            case SDLK_f:
                if(modifier) {
                    if(follow) {
                        follow = false;
                        set_info_timer("follow: false");
                    } else {
                        follow = true;
                        set_info_timer("follow: true");
                    }
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
                break;
            case SDLK_LCTRL:
                modifier = true;
                break;
            case SDLK_ESCAPE:
                break;
            case SDLK_SPACE:
                toggle_playback();
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
                        set_info_timer_with_int("octave", octave);
                    } else if(pattern_editor) {
                        pattern_cursor_x -= 1;
                        check_pattern_cursor_bounds();
                    } else {
                        set_visual_cursor(-1, 0, true);
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
                        set_info_timer_with_int("octave", octave);
                    } else if(pattern_editor) {
                        pattern_cursor_x += 1;
                        check_pattern_cursor_bounds();
                    } else {
                        set_visual_cursor(1, 0, true);
                    }
                }
                break;
            case SDLK_UP:
                pressed_up = true;
                
                if(instrument_editor) {
                    
                } else if(modifier && pattern_editor) {
                    visual_pattern_offset -= 16;
                    if(visual_pattern_offset < 0){
                        visual_pattern_offset = 48;
                    }
                } else {
                    if(pattern_editor) {
                        pattern_cursor_y -= 1;
                        check_pattern_cursor_bounds();
                    } else {
                        if(playing && follow) {}
                        else if(modifier && editing) {
                            move_notes_up();
                            set_visual_cursor(0, -1, true);
                        }
                        else {
                            set_visual_cursor(0, -1, true);
                        }
                    }
                }
                break;
            case SDLK_DOWN:
                pressed_down = true;
                
                if(instrument_editor) {
                
                } else if(modifier && pattern_editor) {
                    visual_pattern_offset += 16;
                    if(visual_pattern_offset > 48){
                        visual_pattern_offset = 0;
                    }
                } else {
                    if(pattern_editor) {
                        pattern_cursor_y += 1;
                        check_pattern_cursor_bounds();
                    } else {
                        if(playing && follow) {}
                        else if(modifier && editing) {
                            move_notes_down();
                            set_visual_cursor(0, 1, true);
                        } else {
                            set_visual_cursor(0, 1, true);
                        }
                    }
                }
                break;
            case SDLK_BACKSPACE:
                if(instrument_editor) {}
                else if(pattern_editor) {}
                else if(editing) {
                    int x_count = visual_cursor_x%5;
                    
                    if(selection_enabled) {
                        copy_notes(current_track, visual_cursor_x, visual_cursor_y, selection_x, selection_y, true, false);
                    } else {
                        if(x_count == 0 || x_count == 1) {
                            cSynthRemoveTrackNode(synth, current_track, synth->track_cursor_x, synth->track_cursor_y);
                            cSynthRemoveTrackNodeParams(synth, current_track, synth->track_cursor_x, synth->track_cursor_y, true, false, false, false);
                            cSynthRemoveTrackNodeParams(synth, current_track, synth->track_cursor_x, synth->track_cursor_y, false, true, false, false);
                            cSynthRemoveTrackNodeParams(synth, current_track, synth->track_cursor_x, synth->track_cursor_y, false, false, true, false);
                            cSynthRemoveTrackNodeParams(synth, current_track, synth->track_cursor_x, synth->track_cursor_y, false, false, false, true);
                        } else if(x_count == 2) {
                            cSynthRemoveTrackNodeParams(synth, current_track, synth->track_cursor_x, synth->track_cursor_y, false, true, false, false);
                        } else if(x_count == 3) {
                            cSynthRemoveTrackNodeParams(synth, current_track, synth->track_cursor_x, synth->track_cursor_y, false, false, true, false);
                        } else if(x_count == 4) {
                            cSynthRemoveTrackNodeParams(synth, current_track, synth->track_cursor_x, synth->track_cursor_y, false, false, false, true);
                        }
                        if(!playing) {
                            set_visual_cursor(0, 1, true);
                        }
                    }
                }
                break;
            case SDLK_RETURN:
                if(instrument_editor) {
                    instrument_editor = false;
                } else {
                    if(pattern_editor) {
                        if(pattern_cursor_y == 17 || pattern_cursor_y == 18 || pattern_cursor_y == 19) {
                            
                            if(pattern_cursor_y < 19) {
                                int ins_nr = pattern_cursor_x;
                                if(pattern_cursor_y == 18) {
                                    ins_nr += 6;
                                }
                                selected_instrument_id = ins_nr;
                                if(instrument_editor) {
                                    instrument_editor = false;
                                } else {
                                    instrument_editor = true;
                                }
                            } else if(pattern_cursor_y == 19 && pattern_cursor_x < 4) {
                                int ins_nr = pattern_cursor_x;
                                ins_nr += 12;
                                selected_instrument_id = ins_nr;
                                if(instrument_editor) {
                                    instrument_editor = false;
                                } else {
                                    instrument_editor = true;
                                }
                            }
                        }
                    } else {
                        toggle_editing();
                    }
                }
                break;
            case SDLK_LSHIFT:
                shift_down = true;
                break;
            default:
                break;
        }
    }
    
    if(shift_down) {
        selection_enabled = true;
    }
    
    int x_count = visual_cursor_x%5;
    
    if(pattern_editor) {
        handle_pattern_keys(keysym);
        return;
    } else if(x_count == 1 && editing) {
        handle_instrument_keys(keysym);
        return;
    } else if((x_count > 1 && editing) && (x_count < 5 && editing)) {
        handle_effect_keys(keysym);
        return;
    } else {
        handle_note_keys(keysym);
    }
}

static void handle_note_keys(SDL_Keysym* keysym) {
    
    switch( keysym->sym ) {
        case SDLK_z:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 12);
            break;
        case SDLK_s:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 13);
            break;
        case SDLK_x:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 14);
            break;
        case SDLK_d:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 15);
            break;
        case SDLK_c:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 16);
            break;
        case SDLK_v:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 17);
            break;
        case SDLK_g:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 18);
            break;
        case SDLK_b:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 19);
            break;
        case SDLK_h:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 20);
            break;
        case SDLK_n:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 21);
            break;
        case SDLK_j:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 22);
            break;
        case SDLK_m:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 23);
            break;
        case SDLK_COMMA:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 24);
            break;
        case SDLK_l:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 25);
            break;
        case SDLK_PERIOD:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 26);
            break;
            
            //upper keyboard
        case SDLK_q:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 24);
            break;
        case SDLK_2:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 25);
            break;
        case SDLK_w:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 26);
            break;
        case SDLK_3:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 27);
            break;
        case SDLK_e:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 28);
            break;
        case SDLK_r:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 29);
            break;
        case SDLK_5:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 30);
            break;
        case SDLK_t:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 31);
            break;
        case SDLK_6:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 32);
            break;
        case SDLK_y:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 33);
            break;
        case SDLK_7:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 34);
            break;
        case SDLK_u:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 35);
            break;
        case SDLK_i:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 36);
            break;
        case SDLK_9:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 37);
            break;
        case SDLK_o:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 38);
            break;
        case SDLK_0:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 39);
            break;
        case SDLK_p:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 40);
            break;
            
        default:
            break;
    }
}

static void handle_pattern_keys(SDL_Keysym* keysym) {
    
    if(pattern_cursor_y > 0 && pattern_cursor_y < 17) {
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
        
        if(pattern < 0){
            pattern = 0;
        }
        
        int old_pattern = synth->patterns[pattern_cursor_x][pattern_cursor_y-1+visual_pattern_offset];
        if(old_pattern < 10) {
            old_pattern *= 10;
            pattern += old_pattern;
            if(pattern >= synth->patterns_height) {
                pattern = synth->patterns_height-1;
            }
        }
        
        synth->patterns[pattern_cursor_x][pattern_cursor_y-1+visual_pattern_offset] = pattern;
    }
}

void handle_instrument_keys(SDL_Keysym* keysym) {
    
    switch( keysym->sym ) {
        case SDLK_0:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 0);
            break;
        case SDLK_1:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 1);
            break;
        case SDLK_2:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 2);
            break;
        case SDLK_3:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 3);
            break;
        case SDLK_4:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 4);
            break;
        case SDLK_5:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 5);
            break;
        case SDLK_6:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 6);
            break;
        case SDLK_7:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 7);
            break;
        case SDLK_8:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 8);
            break;
        case SDLK_9:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 9);
            break;
        case SDLK_a:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 10);
            break;
        case SDLK_b:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 11);
            break;
        case SDLK_c:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 12);
            break;
        case SDLK_d:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 13);
            break;
        case SDLK_e:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 14);
            break;
        case SDLK_f:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 15);
            break;
        default:
            break;
    }
}

static void handle_effect_keys(SDL_Keysym* keysym) {
    
    switch( keysym->sym ) {
        case SDLK_a:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 10);
            break;
        case SDLK_b:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 11);
            break;
        case SDLK_c:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 12);
            break;
        case SDLK_d:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 13);
            break;
        case SDLK_e:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 14);
            break;
        case SDLK_f:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 15);
            break;
        case SDLK_0:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 0);
            break;
        case SDLK_1:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 1);
            break;
        case SDLK_2:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 2);
            break;
        case SDLK_3:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 3);
            break;
        case SDLK_4:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 4);
            break;
        case SDLK_5:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 5);
            break;
        case SDLK_6:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 6);
            break;
        case SDLK_7:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 7);
            break;
        case SDLK_8:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 8);
            break;
        case SDLK_9:
            add_track_node_with_octave(synth->track_cursor_x, synth->track_cursor_y, editing, 9);
            break;
        default:
            break;
    }
}

static void check_sdl_events(SDL_Event event) {
    
    while (SDL_PollEvent(&event)) {
        switch(event.type) {
            case SDL_QUIT:
                quit = true;
                break;
            case SDL_KEYDOWN:
                handle_key_down(&event.key.keysym);
                break;
            case SDL_KEYUP:
                handle_key_up(&event.key.keysym);
                break;
            case SDL_MOUSEMOTION:
                convert_input(event.motion.x, event.motion.y);
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
                if(event.button.button == SDL_BUTTON_LEFT) {
                    input->mouse1 = true;
                    input->touches[0]->active = true;
                    input->touches[0]->x = event.motion.x/4;
                    input->touches[0]->y = event.motion.y/4;
                }
                if(event.button.button == SDL_BUTTON_RIGHT) {
                    input->mouse2 = true;
                    input->touches[1]->active = true;
                    input->touches[1]->x = event.motion.x;
                    input->touches[1]->y = event.motion.y;
                }
                break;
            case SDL_MOUSEBUTTONUP:
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


//const double ChromaticRatio = 1.059463094359295264562;
const double Tao = 6.283185307179586476925;

// 256, 512, 1024, 2048, 4096, 8192

Uint16 bufferSize = 4096; // must be a power of two, decrease to allow for a lower syncCompensationFactor to allow for lower latency, increase to reduce risk of underrun
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

static void log_wave_data(float *floatStream, Uint32 floatStreamLength, Uint32 increment) {
    
    printf("\n\nwaveform data:\n\n");
    Uint32 i=0;
    for (i=0; i<floatStreamLength; i+=increment)
        printf("%4d:%2.16f\n", i, floatStream[i]);
    printf("\n\n");
}

static void render_audio(Sint16 *s_byteStream, int begin, int end, int length) {
    
    if(synth == NULL) {
        printf("audioCallback: synthContext is null, returning.\n");
        return;
    }
    
    Uint32 i;
    
    for(int v_i = 0; v_i < synth->max_voices; v_i++) {
        struct CVoice *voice = synth->voices[v_i];
        struct CInstrument *ins = voice->instrument;
        
        bool advance = false;
        if(synth->solo_voice > -1 && v_i != synth->solo_voice) {
            // if solo is active and this is a channel that is not the solo channel.
        } else if (synth->solo_voice > -1 && voice != NULL && ins != NULL && voice->note_on && voice->instrument != NULL) {
            // this is the solo channel. No care if muted.
            advance = true;
        } else if(voice != NULL && voice->muted == 0 && ins != NULL && voice->note_on && voice->instrument != NULL) {
            advance = true;
        }
        
        if(advance) {
            double d_sampleRate = synth->sample_rate;
            double d_waveformLength = voice->waveform_length;
            
            cSynthVoiceApplyEffects(synth, voice);
            double delta_phi = (double) (cSynthGetFrequency((double)voice->tone_with_fx) / d_sampleRate * (double)d_waveformLength);
            
            for (i = begin; i < end; i+=2) {
                if(voice->note_on) {
                    
                    double amp = 0;
                    double amp_left = 0;
                    double amp_right = 0;
                
                    cSynthAdvanceAmpTargets(synth, voice);
                    
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
                        voice->noteoff_slope_value -= bpm * synth->mod_noteoff_slope;
                        if(voice->noteoff_slope_value < 0) {
                            voice->noteoff_slope_value = 0;
                        }
                    } else {
                        amp = cSynthInstrumentVolumeByPos(ins, voice->adsr_cursor)*ins->volume_scalar;
                    }
                    
                    amp_left = amp * voice->amp_left;
                    amp_right = amp * voice->amp_right;
                    
                    voice->adsr_cursor += synth->mod_adsr_cursor;
                    
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
                                    voice->lowpass_last_sample = (int16_t)(voice->waveform[voice->phase_int]);
                                }
                            } else {
                                if(voice->phase_int < synth->wave_length) {
                                    voice->lowpass_last_sample = (int16_t)(voice->waveform[voice->phase_int]);
                                }
                            }
                            
                            voice->lowpass_next_sample = false;
                        }
                        
                        if(s_byteStream != NULL) {
                            int16_t sample_left = (int16_t)((voice->lowpass_last_sample * amp_left) * synth->master_amp);
                            int16_t sample_right = (int16_t)((voice->lowpass_last_sample * amp_right) * synth->master_amp);
                            s_byteStream[i] += sample_left;
                            s_byteStream[i+1] += sample_right;
                        }
                        
                    } else {
                        if(voice->waveform == synth->noise_table) {
                            if(voice->phase_int < synth->noise_length) {
                                int16_t sample = voice->waveform[voice->phase_int];
                                if(s_byteStream != NULL) {
                                    int16_t sample_left = (int16_t)((sample * amp_left) * synth->master_amp);
                                    int16_t sample_right = (int16_t)((sample * amp_right) * synth->master_amp);
                                    s_byteStream[i] += sample_left;
                                    s_byteStream[i+1] += sample_right;
                                }
                            }
                        } else if(voice->phase_int < synth->wave_length) {
                            int16_t sample = 0;
                            if(voice->waveform == synth->square_wave_table && voice->pwm_active) {
                                sample = cSynthGetPWMSample(synth, voice, voice->phase_int);
                            } else {
                                sample = voice->waveform[voice->phase_int];
                            }
                            
                            if(s_byteStream != NULL) {
                                int16_t sample_left = (int16_t)((sample * amp_left) * synth->master_amp);
                                int16_t sample_right = (int16_t)((sample * amp_right) * synth->master_amp);
                                s_byteStream[i] += sample_left;
                                s_byteStream[i+1] += sample_right;
                            }
                        }
                    }
                }
            }
        }
    }
    
    if(playing || exporting) {
        cSynthAdvanceTrack(synth, length);
    }
}

void audio_callback(void *unused, Uint8 *byteStream, int byteStreamLength) {
    
    memset(byteStream, 0, byteStreamLength);
    
    if(quit || exporting) {
        return;
    }
    
    Sint16 *s_byteStream = (Sint16 *)byteStream;
    int remain = byteStreamLength / 2;
    
    int chunk_size = 64;
    int iterations = remain/chunk_size;
    
    for(int i = 0; i < iterations; i++) {
        int begin = i*chunk_size;
        int end = (i*chunk_size) + chunk_size;
        render_audio(s_byteStream, begin, end, chunk_size);
    }
}

static void change_waveform(int plus) {
    
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

static void change_param(bool plus) {
    
    int x = pattern_cursor_x;
    int y = pattern_cursor_y;
    
    if(y == 0) {
        change_waveform(plus);
    } else if(y == 17 || y == 18) {
        //int ins_nr = x;
        // instruments
        if(y == 18) {
            //ins_nr += 6;
        }
    } else if(y == 20 && x == 0) {
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
       
        
    } else if(y == 20 && x == 2) {
        // active rows for all patterns
        int track_height = synth->track_height;
        if(plus) {
            track_height++;
            if(track_height >= synth->track_max_height) {
                track_height = synth->track_max_height-1;
            }
            synth->track_height = track_height;
            visual_track_height = track_height;
        } else {
            track_height--;
            if(track_height < 1) {
                track_height = 1;
            }
            synth->track_height = track_height;
            visual_track_height = track_height;
        }
        
    } else if(y == 20 && x == 3) {
        if(plus) {
            synth->arpeggio_speed++;
        } else {
            synth->arpeggio_speed--;
            if(synth->arpeggio_speed < 1) {
                synth->arpeggio_speed = 1;
            }
        }
    } else if(y == 20 && x == 4) {
        if(plus) {
            synth->swing++;
        } else {
            synth->swing--;
            if(synth->swing < 0) {
                synth->swing = 0;
            }
        }
    }
    else if(y == 20) {
        //nothing
    } else {
        // pattern nr.
        int pattern = synth->patterns[pattern_cursor_x][pattern_cursor_y-1+visual_pattern_offset];
        if(plus) {
            pattern++;
        } else {
            pattern--;
        }
        if(pattern >= synth->patterns_height) {
            pattern = 0;
        } else if(pattern < 0) {
            pattern = synth->patterns_height-1;
        }
        synth->patterns[pattern_cursor_x][pattern_cursor_y-1+visual_pattern_offset] = pattern;
    }
}

static void draw_line(int x0, int y0, int x1, int y1) {
    
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
        adsr_invert_y_render(i_pos_x, i_pos_y, 0xff00ff00);
        i++;
    }
}

static void render_instrument_editor(double dt) {

    struct CInstrument *ins = synth->instruments[selected_instrument_id];
    int max_nodes = ins->adsr_nodes;
    int inset_x = 10;
    int inset_y = 30;
    double speed = 0.0004*dt;
    if(modifier) {
        speed *= 0.1;
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
    int i;
    
    for(i = 0; i < 2000; i++) {
        double g_pos = (i*(pos_factor*0.001)) + inset_x;
        int top_line_y = (int)(amp_factor + inset_y);
        int bottom_line_y = 0 + inset_y;
        adsr_invert_y_render(g_pos, top_line_y, cengine_color_white);
        adsr_invert_y_render(g_pos, bottom_line_y, cengine_color_white);
    }
    
    for(i = 0; i < max_nodes-1; i++) {
        struct CadsrNode *node = ins->adsr[i];
        struct CadsrNode *node2 = ins->adsr[i+1];
        double g_amp = (node->amp*amp_factor) + inset_y;
        double g_pos = (node->pos*pos_factor) + inset_x;
        double g_amp2 = (node2->amp*amp_factor) + inset_y;
        double g_pos2 = (node2->pos*pos_factor) + inset_x;
        draw_line((int)g_pos, (int)g_amp, (int)g_pos2, (int)g_amp2);
    }
    
    // render dots for nodes
    for(i = 0; i < max_nodes; i++) {
        struct CadsrNode *node = ins->adsr[i];
        double g_amp = (node->amp*amp_factor) + inset_y;
        double g_pos = (node->pos*pos_factor) + inset_x;
        for(int x = -2; x < 2; x++) {
            for(int y = -2; y < 2; y++) {
                int color = cengine_color_red;
                if(i == selected_instrument_node_index) {
                    color = cengine_color_green;
                }
                adsr_invert_y_render(g_pos+x, g_amp+y, color);
            }
        }
    }
    
    char cval[10];
    char c = cSynthGetCharFromParam((char)selected_instrument_id);
    sprintf(cval, "Ins %c", c);
    cEngineRenderLabelWithParams(raster2d, cval, 1, 2, cengine_color_white, cengine_color_black);
}

static void adsr_invert_y_render(double x, double y, int color) {
    int i_x = (int)x;
    int i_y = (int)y/-1+170;
    if(check_screen_bounds(i_x, i_y)) {
        raster2d[i_x][i_y] = color;
    }
}

static void render_pattern_mapping(void) {
    
    int inset_x = 5;
    int inset_y = 1;
    for (int x = 0; x < synth->patterns_and_voices_width; x++) {
        for (int y = 0; y < synth->patterns_and_voices_height; y++) {
            
            int bg_color = cengine_color_black;
            int color = cengine_color_white;
            if(x == pattern_cursor_x && y == pattern_cursor_y) {
                bg_color = cengine_color_magenta;
                color = cengine_color_black;
            }
            
            if(y == 0) {
                int wave_color = color;
                if(synth->solo_voice > -1) {
                    if(x == synth->solo_voice) {
                        wave_color = cengine_color_blue;
                    } else {
                        wave_color = cengine_color_dull_red;
                    }
                } else if(synth->voices[x]->muted == 1) {
                    wave_color = cengine_color_dull_red;
                }
                int val = synth->patterns_and_voices[x][y];
                char cval[3];
                sprintf(cval, "%d", val);
                cEngineRenderLabelWithParams(raster2d, get_wave_type_as_char(val), x*10+inset_x, y+inset_y, wave_color, bg_color);
            } else if(y == 17) {
                char cval[10];
                int ins_nr = x;
                char c = cSynthGetCharFromParam((char)ins_nr);
                sprintf(cval, "Ins %c", c);
                cEngineRenderLabelWithParams(raster2d, cval, x*10+inset_x, y+inset_y, color, bg_color);
            } else if(y == 18) {
                char cval[10];
                int ins_nr = x;
                ins_nr += 6;
                char c = cSynthGetCharFromParam((char)ins_nr);
                sprintf(cval, "Ins %c", c);
                cEngineRenderLabelWithParams(raster2d, cval, x*10+inset_x, y+inset_y, color, bg_color);
            } else if(y == 19 && x < 4) {
                char cval[10];
                int ins_nr = x;
                ins_nr += 12;
                char c = cSynthGetCharFromParam((char)ins_nr);
                sprintf(cval, "Ins %c", c);
                cEngineRenderLabelWithParams(raster2d, cval, x*10+inset_x, y+inset_y, color, bg_color);
            } else if(y == 20 && x == 0) {
                char cval[10];
                sprintf(cval, "BPM %d", synth->bpm);
                cEngineRenderLabelWithParams(raster2d, cval, x*10+inset_x, y+inset_y, color, bg_color);
            } else if(y == 20 && x == 1) {
                //nothing
                cEngineRenderLabelWithParams(raster2d, "-", x*10+inset_x, y+inset_y, color, bg_color);
            } else if(y == 20 && x == 2) {
                char cval[20];
                sprintf(cval, "Rows %d", synth->track_height);
                cEngineRenderLabelWithParams(raster2d, cval, x*10+inset_x, y+inset_y, color, bg_color);
            } else if(y == 20 && x == 3) {
                char cval[20];
                sprintf(cval, "Arp %d", synth->arpeggio_speed);
                cEngineRenderLabelWithParams(raster2d, cval, x*10+inset_x, y+inset_y, color, bg_color);
            } else if(y == 20 && x == 4) {
                char cval[20];
                sprintf(cval, "Swing %d", synth->swing);
                cEngineRenderLabelWithParams(raster2d, cval, x*10+inset_x, y+inset_y, color, bg_color);
            }
            else if(y == 19) {
                //nothing
                cEngineRenderLabelWithParams(raster2d, "-", x*10+inset_x, y+inset_y, color, bg_color);
            }
            else if(y == 20) {
                //nothing
                cEngineRenderLabelWithParams(raster2d, "-", x*10+inset_x, y+inset_y, color, bg_color);
            }
            else if(y == 21) {
                //nothing
                cEngineRenderLabelWithParams(raster2d, "-", x*10+inset_x, y+inset_y, color, bg_color);
            }
            else {

                if(synth->active_tracks[y-1+visual_pattern_offset] == 1) {
                    bg_color = cengine_color_dull_green;
                    color = cengine_color_black;
                    if(synth->solo_track == y-1+visual_pattern_offset) {
                        bg_color = cengine_color_blue;
                    }
                }
                
                if(synth->solo_track == y-1) {
                    bg_color = cengine_color_blue;
                    color = cengine_color_black;
                }
                
                if(y-1+visual_pattern_offset == synth->current_track && playing) {
                    if(synth->current_track == synth->solo_track) {
                        bg_color = cengine_color_blue;
                    } else {
                        bg_color = cengine_color_green;
                    }
                    color = cengine_color_black;
                }
                
                if(x == pattern_cursor_x && y == pattern_cursor_y) {
                    bg_color = cengine_color_magenta;
                    color = cengine_color_black;
                }
                
                int pattern = synth->patterns[x][y-1+visual_pattern_offset];
                char cval[3];
                sprintf(cval, "%d", pattern);
                cEngineRenderLabelWithParams(raster2d, cval, x*10+inset_x, y+inset_y, color, bg_color);
                
                if(x == 0) {
                    // print track numbers
                    int track_nr = y-1+visual_pattern_offset;
                    char cval[3];
                    sprintf(cval, "%d", track_nr);
                    int x_offset = 1;
                    if(track_nr < 10) {
                        x_offset = 2;
                    }
                    cEngineRenderLabelWithParams(raster2d, cval, x_offset, y+1, cengine_color_white, cengine_color_black);
                }
            }
        }
    }
    
    int pattern_at_cursor = -1;
    int track_at_cursor = -1;
    if(pattern_cursor_y > 0 && pattern_cursor_y < 17) {
        track_at_cursor = pattern_cursor_y-1+visual_pattern_offset;
        pattern_at_cursor = synth->patterns[pattern_cursor_x][pattern_cursor_y-1+visual_pattern_offset];
    }
    
    if(track_at_cursor > -1 && pattern_at_cursor > -1) {
        char cval[20];
        sprintf(cval, "p:%d t:%d", pattern_at_cursor, track_at_cursor);
        cEngineRenderLabelWithParams(raster2d, cval, 55, 23, cengine_color_white, cengine_color_black);
    }
}

static char *get_wave_type_as_char(int type) {
    
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

static void draw_wave_types(void) {
    
    for (int x = 0; x < synth->patterns_and_voices_width; x++) {
        int val = synth->patterns_and_voices[x][0];
        int wave_color = cengine_color_white;
        if(synth->solo_voice > -1) {
            if(x == synth->solo_voice) {
                wave_color = cengine_color_blue;
            } else {
                wave_color = cengine_color_dull_red;
            }
        } else if(synth->voices[x]->muted == 1) {
            wave_color = cengine_color_dull_red;
        }
        cEngineRenderLabelWithParams(raster2d, get_wave_type_as_char(val), 2+x*10, -visual_cursor_y+5, wave_color, cengine_color_black);
    }
}

static void render_visuals(void) {
    
    for(int x = 0; x < s_width; x++) {
        for(int y = 0; y < s_height; y++) {
            raster2d[x][y] = rand();
        }
    }
}

int is_in_bounds(int x, int y, int width, int height) {
    
    if(x > -1 && y > -1 && x < width && y < height) {
        return 1;
    } else {
        return 0;
    }
}

static void render_track(double dt) {
    
    if(instrument_editor && !file_editor) {
        render_instrument_editor(dt);
        return;
    } else if(pattern_editor && !file_editor) {
        render_pattern_mapping();
        return;
    } else if(file_editor) {
        render_files();
        return;
    } else if(visualiser) {
        render_visuals();
        return;
    }
    
    int x_count = 0;
    int offset_x = 0;
    int inset_x = 2;
    int inset_y = 6;
    
    int cursor_x = visual_cursor_x/5;
    cSynthUpdateTrackCursor(synth, cursor_x, visual_cursor_y);
    int track_progress_int = synth->track_progress_int;
    
    draw_wave_types();
    
    if(follow && playing) {
        int diff = track_progress_int - visual_cursor_y;
        set_visual_cursor(0, diff, false);
    } else {
        track_progress_int = visual_cursor_y;
    }
    
    int node_x = -1;
    
    for (int y = 0; y < synth->track_height; y++) {
        offset_x = 0;
        for (int x = 0; x < visual_track_width; x++) {
            
            int bg_color = cengine_color_black;
            int color = cengine_color_white;
            
            //int foreground_color = cengine_color_white;
            if(x >= 0 && x < 5) bg_color = cengine_color_bg1;
            if(x >= 5 && x < 10) bg_color = cengine_color_bg2;
            if(x >= 10 && x < 15) bg_color = cengine_color_bg3;
            if(x >= 15 && x < 20) bg_color = cengine_color_bg4;
            if(x >= 20 && x < 25) bg_color = cengine_color_bg5;
            if(x >= 25 && x < 30) bg_color = cengine_color_bg6;
            
            node_x = (int)floor(x/5);
            
            if(selection_enabled) {
                int sel_x = (int)floor(selection_x/5);
                int vis_x = (int)floor(visual_cursor_x/5);
                
                bool inside_selection_x = false;
                bool inside_selection_y = false;
                if(sel_x < vis_x) {
                    if(node_x >= sel_x && node_x <= vis_x) {
                        inside_selection_x = true;
                    }
                } else {
                    if(node_x >= vis_x && node_x <= sel_x) {
                        inside_selection_x = true;
                    }
                }
                
                if(selection_y < visual_cursor_y) {
                    if(y >= selection_y && y <= visual_cursor_y) {
                        inside_selection_y = true;
                    }
                } else {
                    if(y >= visual_cursor_y && y <= selection_y) {
                        inside_selection_y = true;
                    }
                }
                
                if(inside_selection_x && inside_selection_y && editing) {
                    color = cengine_color_black;
                    bg_color = cengine_color_cyan;
                    if(visual_cursor_x == x && visual_cursor_y == y) {
                        bg_color = cengine_color_magenta;
                    }
                }
            }

            if(synth->track_progress_int == y && playing == 1) {
                if(synth->current_track == current_track) {
                    bg_color = cengine_color_green;
                    color = cengine_color_black;
                }
            }
            
            if(visual_cursor_x == x && visual_cursor_y == y) {
                if(editing == 1) {
                    bg_color = cengine_color_magenta;
                    color = cengine_color_black;
                } else {
                    bg_color = cengine_color_dull_red;
                }
            }
            
            
            int pos_y = inset_y+y-track_progress_int;
            
            if(x == 0 || x == 5 || x == 10 || x == 15 || x == 20 || x == 25) {
                
                int pattern = synth->patterns[node_x][current_track];
                struct CTrackNode *t = synth->track[pattern][node_x][y];
                if(t != NULL && t->tone_active) {
                    int tone = t->tone;
                    char *ctone = cSynthToneToChar(tone);
                    cEngineRenderLabelWithParams(raster2d, ctone, inset_x+x+offset_x, pos_y, color, bg_color);
                } else {
                    cEngineRenderLabelWithParams(raster2d, " - ", inset_x+x+offset_x, pos_y, color, bg_color);
                }
                offset_x += 3;
            } else {
                int pattern = synth->patterns[node_x][current_track];
                
                struct CTrackNode *t = synth->track[pattern][node_x][y];
                if(t != NULL) {
                    if(x_count == 1) {
                        if(t->instrument != NULL) {
                            char cval[20];
                            char c = cSynthGetCharFromParam((char)t->instrument_nr);
                            sprintf(cval, "%c", c);
                            cEngineRenderLabelWithParams(raster2d, cval, inset_x+x+offset_x, pos_y, color, bg_color);
                        } else {
                            cEngineRenderLabelWithParams(raster2d, "-", inset_x+x+offset_x, pos_y, color, bg_color);
                        }
                    } else if(x_count == 2) {
                        //effect type
                        char cval[20];
                        sprintf(cval, "%c", t->effect);
                        cEngineRenderLabelWithParams(raster2d, cval, inset_x+x+offset_x, pos_y, color, bg_color);
                    } else if(x_count == 3) {
                        //effect type
                        char cval[20];
                        sprintf(cval, "%c", t->effect_param1);
                        cEngineRenderLabelWithParams(raster2d, cval, inset_x+x+offset_x, pos_y, color, bg_color);
                    } else if(x_count == 4) {
                        //effect type
                        char cval[20];
                        sprintf(cval, "%c", t->effect_param2);
                        cEngineRenderLabelWithParams(raster2d, cval, inset_x+x+offset_x, pos_y, color, bg_color);
                    }
                } else {
                    cEngineRenderLabelWithParams(raster2d, "-", inset_x+x+offset_x, pos_y, color, bg_color);
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
        
        int pattern_at_cursor = synth->patterns[cursor_x][current_track];
        current_pattern = pattern_at_cursor;
        char cval[20];
        sprintf(cval, "p:%d t:%d", current_pattern, current_track);
        cEngineRenderLabelWithParams(raster2d, cval, 55, 23, cengine_color_white, cengine_color_black);
    }
}

SDL_Window *window = NULL;
SDL_Texture *texture = NULL;
SDL_Renderer *renderer = NULL;
SDL_GLContext context;

static void setup_sdl(void) {
    
    SDL_Init(SDL_INIT_VIDEO);
    
	// Get current display mode of all displays.
	int i;
	SDL_DisplayMode current;
	for(i = 0; i < SDL_GetNumVideoDisplays(); ++i) {
		int should_be_zero = SDL_GetCurrentDisplayMode(i, &current);
		if(should_be_zero != 0) {
			// In case of error...
			SDL_Log("Could not get display mode for video display #%d: %s", i, SDL_GetError());
			st_pause();
		} else {
			// On success, print the current display mode.
			SDL_Log("Display #%d: current display mode is %dx%dpx @ %dhz. \n", i, current.w, current.h, current.refresh_rate);
		}
	}
	
    //SDL_WINDOW_FULLSCREEN
    
    window = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL /*| SDL_WINDOW_FULLSCREEN*/);
	if(window != NULL) {
		
		context = SDL_GL_CreateContext(window);
		if(context == NULL) {
			printf("\nFailed to create context: %s\n", SDL_GetError());
			st_pause();
		}

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
            printf("Failed to create renderer: %s", SDL_GetError());
			st_pause();
        }
	} else {
        printf("Failed to create window:%s", SDL_GetError());
		st_pause();
    }
}

static void setup_synth(void) {
    
    synth = cSynthContextNew();
    synth->interleaved = true;
    synth->chunk_size = 64;
    cSynthInit(synth);
    visual_track_height = synth->track_height;
}

static void setup_texture(void) {
    
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
        // contains an integer for every color/pixel on the sheet.
        raw_sheet = cAllocatorAlloc((sheet_width*sheet_height) * sizeof(unsigned int), "main.c raw_sheet 1");
        for(int r = 0; r < sheet_width*sheet_height; r++) {
            raw_sheet[r] = 0;
        }
    }
    
    // load gfx from source.
    for (int i = 0; i < 1024*16; i++) {
        raw_sheet[i] = chars_gfx[i];
    }
    cEngineWritePixelData(raw_sheet);
    
    if(load_gfx) {
        if(image != NULL) {
            SDL_FreeSurface(image);
        }
    } else {
        raw_sheet = cAllocatorFree(raw_sheet);
    }
}

static void destroy_sdl(void) {
	
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}

static int setup_sdl_audio(void) {
	
    SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER);
    SDL_AudioSpec want;
    SDL_zero(want);
    SDL_zero(audioSpec);
    
    want.freq = synth->sample_rate;
    want.format = AUDIO_S16LSB;
    want.channels = 2;
    want.samples = bufferSize;
    want.callback = audio_callback;
    
    printf("\naudioSpec want\n");
    printf("----------------\n");
    printf("sample rate:%d\n", want.freq);
    printf("channels:%d\n", want.channels);
    printf("samples:%d\n", want.samples);
    printf("----------------\n\n");
    
    AudioDevice = SDL_OpenAudioDevice(NULL, 0, &want, &audioSpec, 0);
    
    printf("\naudioSpec get\n");
    printf("----------------\n");
    printf("sample rate:%d\n", audioSpec.freq);
    printf("channels:%d\n", audioSpec.channels);
    printf("samples:%d\n", audioSpec.samples);
    printf("size:%d\n", audioSpec.size);
    printf("----------------\n");
    
    if (AudioDevice == 0) {
        printf("\nFailed to open audio: %s\n", SDL_GetError());
		st_pause();
        return 1;
    }
    
    if (audioSpec.format != want.format) {
        printf("\nCouldn't get requested audio format.\n");
		st_pause();
        return 2;
    }
    
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

static void setup_cengine(void) {
    
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
}

static void cleanup_synth(void) {
    printf("allocs before cleanup:\n");
    cAllocatorPrintAllocationCount();
    cleanup_data();
    printf("allocs after cleanup:\n");
    cAllocatorPrintAllocations();
    cAllocatorPrintAllocationCount();
    
    // If allocation tracking is on, clean up the last stuff.
    cAllocatorCleanup();
}

int fps_print_interval = 0;
int print_interval_limit = 100;
int old_time = 0;

static int get_delta(void) {
    int currentTime = SDL_GetTicks();
    int delta = 0;
    
    if(old_time == 0) {
        old_time = currentTime;
        delta = 16;
    } else {
        delta = currentTime-old_time;
        old_time = currentTime;
        if(delta <= 0) {
            delta = 1;
        }
        double fps = 1000/delta;
        if(fps_print_interval >= print_interval_limit) {
            //printf("fps:%f delta_time:%d\n", fps, delta);
        }
    }
    
    if(fps_print_interval >= print_interval_limit) {
        fps_print_interval = 0;
    }
    fps_print_interval++;
    
    return delta;
}

int last_dt = 16;

static void redraw_screen_function(void) {
    
}

static void main_loop(void) {
    
    int delay_ms = 64;
    SDL_LockAudioDevice(AudioDevice);
    check_sdl_events(event);
    update_and_render_info(last_dt);
    
    if(instrument_editor) {
        synth->needs_redraw = true;
    }
    
    if(redraw_screen || synth->needs_redraw) {
        render_track(last_dt);
        for (int r_x = 0; r_x < s_width; r_x++) {
            for (int r_y = 0; r_y < s_height; r_y++) {
                raster[r_x+r_y*s_width] = raster2d[r_x][r_y];
            }
        }
        for(int x = 0; x < s_width; x++) {
            for(int y = 0; y < s_height; y++) {
                raster2d[x][y] = cengine_color_bg;
            }
        }
        redraw_screen = false;
        synth->needs_redraw = false;
    }
    
    
    SDL_UpdateTexture(texture, NULL, raster, s_width * sizeof (unsigned int));
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
    SDL_UnlockAudioDevice(AudioDevice);
    
    int dt = get_delta();
    last_dt = dt;
    int wait_time = delay_ms-dt;
    if(dt < delay_ms) {
        if(fps_print_interval >= print_interval_limit) {
            //printf("dt:%d additional wait_time:%d\n", dt, wait_time);
        }
        SDL_Delay(wait_time);
    } else {
        if(fps_print_interval >= print_interval_limit) {
            //printf("frame time:%d\n", dt);
        }
    }
}

static void debug_log(char *str) {
    
    if(log_file_enabled) {
        FILE * fp;
        fp = fopen ("log.txt", "a");
        fprintf(fp, "%s\n", str);
        fclose(fp);
    }
}

static int get_buffer_size_from_index(int i) {
    
    switch (i) {
        case 1:
            return 256;
            break;
        case 2:
            return 512;
            break;
        case 3:
            return 1024;
            break;
        case 4:
            return 2048;
            break;
        case 5:
            return 4096;
            break;
        case 6:
            return 8192;
            break;
        case 7:
            return 16384;
            break;
        default:
            return 4096;
            break;
    }

}

static void load_config() {
    
    char *param_buffer_size = "buffer_size";
    char *config_file_name = "config.json";
    cJSON *root = NULL;
    cJSON *object = NULL;
    bool success = false;
    char *b = load_file(config_file_name);
    if(b != NULL) {
        root = cJSON_Parse(b);
        if(root != NULL) {
            object = cJSON_GetObjectItem(root, param_buffer_size);
            if(object != NULL) {
                int buffer_index_value = object->valueint;
                bufferSize = (uint16_t)get_buffer_size_from_index(buffer_index_value);
                printf("setting buffersize from config:%d\n", bufferSize);
                success = true;
            }
            cJSON_Delete(root);
        }
        cAllocatorFree(b);
    }
    
    if(!success) {
        printf("could not find config file. Writing config.json.\n");
        bufferSize = 4096;
        FILE * fp;
        fp = fopen ("config.json", "w+");
        fprintf(fp, "%s", "{\"buffer_size\" : 5}");
        fclose(fp);
    }
}

static void st_pause(void) {
    
	SDL_Delay(5000);
}
static void st_log(char *message) {
    
    printf("*** %s \n", message);
}

int main(int argc, char* argv[])
{
    
    //load_config();
    st_log("started executing.");
    
    setup_data();
    st_log("setup data successful.");
    
    setup_cengine();
    st_log("setup cengine successful.");
    
    setup_synth();
    st_log("setup synth successful.");
    
    if(run_with_sdl) {
        setup_texture();
        st_log("setup texture successful.");
    
        setup_sdl();
        st_log("setup SDL successful.");
    
        setup_sdl_audio();
        st_log("setup SDL audio successful.");
    }
    
    if(run_with_sdl) {
        if (texture != NULL) {
            while (!quit) {
                main_loop();
            }
        }
    } else {
        //sleep(5);
    }

    cleanup_synth();
    st_log("synth cleanup successful.");
    
    if(run_with_sdl) {
        destroy_sdl();
        st_log("SDL cleanup successful.");
    
        SDL_CloseAudioDevice(AudioDevice);
        st_log("SDL audio cleanup successful.");
        
        SDL_Quit();
        st_log("SDL quit successful.");
    }
    
    return 0;
}

static void export_wav(char *filename) {
    
    if(playing) {
        toggle_playback();
    }
    
    // find starting pattern
    int starting_track = 0;
    for(int i = 0; i < synth->patterns_height; i++) {
        if(synth->active_tracks[i] == 1) {
            starting_track = i;
            break;
        }
    }
    
    synth->current_track = starting_track;
    cSynthResetTrackProgress(synth, starting_track, 0);
    exporting = true;
    synth->looped = false;
    
    unsigned long buffer_size = 0;
    int chunk_size = 64;
    Sint16 *buffer = NULL;
    int iterations = INT16_MAX;
   
    // TODO analyze and set modifier (master_amp) for maximize volume for export.
    
    
    // calculate size of file and allocate buffer etc
    for(int i = 0; i < iterations; i++) {
        int begin = i*chunk_size;
        int end = (i*chunk_size) + chunk_size;
        render_audio(NULL, begin, end, chunk_size);
        if(synth->looped) {
            synth->looped = false;
            break;
        }
        buffer_size += chunk_size*2;
    }
    
    printf("export buffer size:%ld\n", buffer_size);
    synth->current_track = starting_track;
    cSynthResetTrackProgress(synth, starting_track, 0);
    exporting = true;
    synth->looped = false;
    
    // alloc buffer of
    buffer = cAllocatorAlloc(sizeof(Sint16) * buffer_size, "export buffer");
    
    // render to buffer
    for(int i = 0; i < iterations; i++) {
        int begin = i*chunk_size;
        int end = (i*chunk_size) + chunk_size;
        render_audio(buffer, begin, end, chunk_size);
        if(synth->looped) {
            synth->looped = false;
            break;
        }
    }
    
    write_wav(filename, buffer_size, buffer, synth->sample_rate);
    printf("total buffer size: %lu\n", buffer_size);
    cAllocatorFree(buffer);
    exporting = false;
    
    printf("\n\n");
}

static void write_little_endian(unsigned int word, int num_bytes, FILE *wav_file) {
    
    unsigned buf;
    while(num_bytes > 0) {
        buf = word & 0xff;
        fwrite(&buf, 1,1, wav_file);
        num_bytes--;
        word >>= 8;
    }
}

static void write_wav(char *filename, unsigned long num_samples, short int *data, int s_rate) {
    
    FILE* wav_file;
    unsigned int sample_rate;
    unsigned int num_channels;
    unsigned int bytes_per_sample;
    unsigned int byte_rate;
    unsigned long i;    /* counter for samples */
    
    num_channels = 2;
    bytes_per_sample = 2;
    
    if (s_rate <= 0) sample_rate = 44100;
    else sample_rate = (unsigned int) s_rate;
    
    byte_rate = sample_rate * num_channels * bytes_per_sample;
    
    wav_file = fopen(filename, "wb");
    //assert(wav_file);   /* make sure it opened */
    
    /* write RIFF header */
    fwrite("RIFF", 1, 4, wav_file);
    write_little_endian((unsigned int)(36 + bytes_per_sample * num_samples * num_channels), 4, wav_file);
    fwrite("WAVE", 1, 4, wav_file);
    
    /* write fmt  subchunk */
    fwrite("fmt ", 1, 4, wav_file);
    write_little_endian(16, 4, wav_file);   /* SubChunk1Size is 16 */
    write_little_endian(1, 2, wav_file);    /* PCM is format 1 */
    write_little_endian(num_channels, 2, wav_file);
    write_little_endian(sample_rate, 4, wav_file);
    write_little_endian(byte_rate, 4, wav_file);
    write_little_endian(num_channels*bytes_per_sample, 2, wav_file);  /* block align */
    write_little_endian(8*bytes_per_sample, 2, wav_file);  /* bits/sample */
    
    /* write data subchunk */
    fwrite("data", 1, 4, wav_file);
    write_little_endian((unsigned int)(bytes_per_sample * num_samples * num_channels), 4, wav_file);
    for (i = 0; i < num_samples; i++) {
        if(i < num_samples/2) {
            write_little_endian((unsigned int)(data[i]), bytes_per_sample, wav_file);
        }
    }
    
    fclose(wav_file);
}

