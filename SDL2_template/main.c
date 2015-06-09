
/*
codesign -f -s - LICENSE.webp.txt
codesign -f -s - webp
codesign --force -s "Developer ID Application: Harry Lundstrom" /Library/Frameworks/SDL2_image.framework/Versions/A/Frameworks/webp.framework
*/

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
#include <limits.h>
#include "file_settings.h"
#include <SDL2/SDL.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#ifdef _WIN64
//define something for Windows (64-bit)
    #define platform_windows
	#include "dir_win.h"
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
    #include "osx_settings.h"
#endif
#elif __linux
// linux
#elif __unix // all unices not caught above
// Unix
#elif __posix
// POSIX
#endif


static bool load_gfx = false; // used when loading in new GFX from file.
//static unsigned int **sheet = NULL; // used when loading in new GFX from file.
static const bool debuglog = false;
static const bool errorlog = true;
static const int passive_render_delay_ms = 16;
static const int active_render_delay_ms = 16;
static const bool lock_device = false;
static bool log_file_enabled = true;
static bool run_with_sdl = true;
static bool redraw_screen = true;
static bool passive_rendering = true;
static bool preview_enabled = true;
static char *conf_default_dir = NULL;
static int current_pattern = 0;
static int current_track = 0;
static int quit = 0;
static char *title = "snibbetracker test (experimental)";
static struct CInput *input = NULL;
static unsigned int *raster = NULL;
static unsigned int **raster2d = NULL;
static unsigned int *raw_sheet = NULL;
static unsigned int **credits2d = NULL;
static int width = 256*4;
static int height = 144*4;
static int s_width = 256*2;
static int s_height = 144*2;
static bool playing = false;
static bool exporting = false;
static bool editing = false;
static bool modifier = false;
static bool shift_down = false;
static bool selection_enabled = false;
//static bool selection_set = false;
static bool follow = false;
static bool visualiser = false;
static bool credits = false;
static bool help = false;
static int help_index = 0;
static int help_index_max = 7;
static bool export_project = false;
static int octave = 2;
static int visual_pattern_offset = 0;
static int visual_track_width = 30;
static int visual_track_height = 16;
static int visual_cursor_x = 0;
static int visual_cursor_y = 0;
static int selection_x = 0;
static int selection_y = 0;
static int last_selection_x = 0;
static int last_selection_y = 0;
static int last_selection_w = 0;
static int last_selection_h = 0;
static int last_copied_pattern_x = 0;
static int last_copied_pattern_y = 0;
static bool pattern_editor = false;
static int pattern_cursor_x = 0;
static int pattern_cursor_y = 0;
static bool instrument_editor = false;
static bool instrument_editor_effects = false;
static int instrument_editor_effects_x = 0;
static int instrument_editor_effects_y = 0;
static int visual_instrument_effects = 5;
static int selected_instrument_id = 0;
static int selected_instrument_node_index = 1;
static int copied_instrument = -1;
static bool file_editor = false;
static bool file_editor_confirm_action = false;
static bool file_editor_existing_file = false;
static bool pressed_left = false;
static bool pressed_right = false;
static bool pressed_up = false;
static bool pressed_down = false;
static bool tempo_editor = false;
static int tempo_selection_x = 0;
static int tempo_selection_y = 0;
static struct CSynthContext *synth = NULL;
static struct CTimer *info_timer = NULL;
static char *info_string = NULL;
static struct FileSettings *file_settings = NULL;
static int envelop_node_camera_offset = 0;


// credits
static float c_hue_a = 0;
static float c_hue_r = 0;
static float c_hue_g = 0;
static float c_hue_b = 0;
static float c_new_hue_r = 0;
static float c_new_hue_g = 0;
static float c_new_hue_b = 0;
static bool credits_higher_contrast = false;
static bool credits_scanlines_x = false;
static bool credits_scanlines_y = false;
static float credits_hue_rotation = 0;
static float credits_hue_rotation_inc = 0.2f;
static bool credits_init = false;
static double credits_x = 0;
static double credits_y = 0;
static double credits_x_inc = 0.435;
static double credits_y_inc = 0.351;
static int credits_cursor_x = 0;
static int credits_cursor_y = 0;
static bool credits_up = false;
static bool credits_down = false;
static bool credits_left = false;
static bool credits_right = false;
static int credits_color = 0xFFCCCCCC;
static int credits_bg_color = 0xFF000000;
static int credits_brush_color = 0xFF000000;
static void resetColorValues();
static void TtransformHSV(float H, float S, float V);
static void renderPixels(unsigned int **data, int start_x, int start_y, int w, int h, float rotation);



#define MAX_TOUCHES 8
#define sheet_width 1024
#define sheet_height 1024
int fullscreen = 0;



#define cengine_color_dull_red 0xFF771111
#define cengine_color_red 0xFFFF0000
#define cengine_color_green 0xFF00FF00
#define cengine_color_cyan 0xFF00FFFF
#define cengine_color_blue 0xFF0000FF
#define cengine_color_black 0xFF000000
#define cengine_color_white 0xFFCCCCCC
#define cengine_color_grey 0xFF666666
#define cengine_color_magenta 0xFFFF00FF
#define cengine_color_dull_green 0xFF117711

#define cengine_color_bg 0xFF000000
#define cengine_color_bg1 0xFF332222
#define cengine_color_bg2 0xFF223322
#define cengine_color_bg3 0xFF222233
#define cengine_color_bg4 0xFF332233
#define cengine_color_bg5 0xFF333322
#define cengine_color_bg6 0xFF223333

#define cengine_color_bg1_highlight 0xFF443333
#define cengine_color_bg2_highlight 0xFF334433
#define cengine_color_bg3_highlight 0xFF333344
#define cengine_color_bg4_highlight 0xFF443344
#define cengine_color_bg5_highlight 0xFF444433
#define cengine_color_bg6_highlight 0xFF334444



unsigned int color_info_text_bg = cengine_color_black;
unsigned int color_file_name_text = cengine_color_red;
unsigned int color_inactive_instrument_node = cengine_color_red;
unsigned int color_active_instrument_node = cengine_color_green;
unsigned int color_envelop = cengine_color_green;
unsigned int color_inactive_text = cengine_color_grey;
unsigned int color_text = cengine_color_white;
unsigned int color_text_bg = cengine_color_black;
unsigned int color_marker = cengine_color_dull_red;
unsigned int color_solo = cengine_color_blue;
unsigned int color_solo_text = cengine_color_black;
unsigned int color_mute = cengine_color_dull_red;
unsigned int color_mute_text = cengine_color_black;
unsigned int color_active_row = cengine_color_dull_green;
unsigned int color_active_row_text = cengine_color_black;
unsigned int color_playing_row = cengine_color_green;
unsigned int color_playing_row_text = cengine_color_black;
unsigned int color_edit_marker = cengine_color_magenta;
unsigned int color_edit_text = cengine_color_black;
unsigned int color_selection_marker = cengine_color_cyan;
unsigned int color_selection_text = cengine_color_black;
unsigned int color_bg = cengine_color_black;
unsigned int color_bg1 = cengine_color_bg1;
unsigned int color_bg2 = cengine_color_bg2;
unsigned int color_bg3 = cengine_color_bg3;
unsigned int color_bg4 = cengine_color_bg4;
unsigned int color_bg5 = cengine_color_bg5;
unsigned int color_bg6 = cengine_color_bg6;
unsigned int color_bg1_highlight = cengine_color_bg1_highlight;
unsigned int color_bg2_highlight = cengine_color_bg2_highlight;
unsigned int color_bg3_highlight = cengine_color_bg3_highlight;
unsigned int color_bg4_highlight = cengine_color_bg4_highlight;
unsigned int color_bg5_highlight = cengine_color_bg5_highlight;
unsigned int color_bg6_highlight = cengine_color_bg6_highlight;


static void init_file_settings(void);
static bool file_exists(char *path);
static void handle_credits_keys(SDL_Keysym* keysym);
static void handle_help_keys(SDL_Keysym* keysym);
static void handle_key_down_file(SDL_Keysym* keysym);
static void set_list_file_name(void);
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
static void copy_instrument(int instrument);
static void paste_instrument(int instrument);
static void copy_notes(int track, int cursor_x, int cursor_y, int selection_x, int selection_y, bool cut, bool store);
static void paste_notes(int track, int cursor_x, int cursor_y);
static void copy_pattern(int cursor_x, int cursor_y);
static void paste_pattern(int cursor_x, int cursor_y);
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
static void handle_tempo_keys(SDL_Keysym* keysym);
static void instrument_effect_remove();
static void handle_instrument_effect_keys(SDL_Keysym* keysym);
static void check_sdl_events(SDL_Event event);
static int get_delta(void);
static void log_wave_data(float *floatStream, Uint32 floatStreamLength, Uint32 increment);
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
static void render_help(void);
static void render_credits(void);
static void render_tempo_editor(double dt);
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
static void load_config(void);
static bool parse_config(char *json);
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
    f->file_moved_in_list = false;
    
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

static bool file_exists(char *path) {
    
    bool exists = false;
    FILE * fp;
    fp = fopen (path, "r");
    if(fp != NULL) {
        exists = true;
        fclose(fp);
    }
    return exists;
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
                    char *file_path = cAllocatorAlloc(sizeof(char)*file_settings->file_name_max_length, "load_path chars");
                    if(conf_default_dir != NULL) {
                        sprintf(file_path, "%s%s.wav", conf_default_dir, file_settings->file_name);
                    } else {
                        sprintf(file_path, "%s.wav", file_settings->file_name);
                    }
                    
                    if(file_editor_confirm_action) {
                        export_wav(file_path);
                        export_project = false;
                        exit_file_editor();
                        file_editor_confirm_action = false;
                    } else {
                        if(file_exists(file_path)) {
                            file_editor_existing_file = true;
                            file_editor_confirm_action = true;
                        } else {
                            //file_editor_existing_file = false;
                            // just proceed.
                            export_wav(file_path);
                            export_project = false;
                            exit_file_editor();
                            file_editor_confirm_action = false;
                        }
                        if(debuglog) { printf("confirm action\n"); }
                    }
                    cAllocatorFree(file_path);
                }
            } else {
                if(file_settings->file_name != NULL && file_settings->file_editor_save && file_editor_confirm_action) {
                    save_project_file();
                    file_editor_confirm_action = false;
                } else if(file_settings->file_name != NULL) {
                    file_editor_existing_file = false;
                    char *file_path = cAllocatorAlloc(sizeof(char)*file_settings->file_name_max_length, "load_path chars");
                    if(conf_default_dir != NULL) {
                        sprintf(file_path, "%s%s.json", conf_default_dir, file_settings->file_name);
                    } else {
                        sprintf(file_path, "%s.json", file_settings->file_name);
                    }
                    if(file_editor_confirm_action) {
                        load_project_file(file_path);
                        visual_track_height = synth->track_height;
                        file_editor_confirm_action = false;
                    } else {
                        // save project
                        file_editor_confirm_action = true;
                        if(file_settings->file_editor_save) {
                            if(file_exists(file_path)) {
                                file_editor_existing_file = true;
                                if(debuglog) { printf("confirm action\n"); }
                            } else {
                                // just proceed
                                save_project_file();
                                file_editor_confirm_action = false;
                            }
                        } else {
                            // load file.
                            file_editor_confirm_action = true;
                            if(debuglog) { printf("confirm action\n"); }
                        }
                    }
                    cAllocatorFree(file_path);
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
            set_list_file_name();
            break;
        case SDLK_DOWN:
            if(file_settings->file_cursor_y < file_settings->file_dir_max_length-1) {
                if(file_settings->file_dirs[file_settings->file_cursor_y+1] != NULL) {
                    file_settings->file_cursor_y++;
					
                }
            }
            set_list_file_name();
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


static void set_list_file_name(void) {
    
    // must remove file ending..
    char *list_name = file_settings->file_dirs[file_settings->file_cursor_y];
	if(list_name != NULL) {
		int length = (int)strlen(list_name);
        file_settings->file_name = cAllocatorFree(file_settings->file_name);
        char *temp_chars = cAllocatorAlloc(sizeof(char)*file_settings->file_name_max_length, "file name chars");
        sprintf(temp_chars, "%s", list_name);
        file_settings->file_name = temp_chars;
        
        // a list_name cannot be added unless it has the correct file endings, so we can assume
        // it will be of at least a certain size.
        if(file_settings->file_name[length-2] == 'a') {
            // wav file
            file_settings->file_name[length-4] = '\0';
        } else {
            file_settings->file_name[length-5] = '\0';
        }
    }
}

static void exit_file_editor(void) {
    
    for (int i = 0; i < file_settings->file_dir_max_length; i++) {
        if (file_settings->file_dirs[i] != NULL) {
            file_settings->file_dirs[i] = cAllocatorFree(file_settings->file_dirs[i]);
        }
    }
    file_settings->file_path = cAllocatorFree(file_settings->file_path);
    //file_settings->file_name = cAllocatorFree(file_settings->file_name);
    file_editor = false;
    file_settings->file_editor_save = false;
    file_settings->reload_dirs = true;
    file_editor_confirm_action = false;
    file_editor_existing_file = false;
}


static int getDirectoryList(char *dir_string) {
#if defined(platform_osx)
    return getDirectoryListPosix(dir_string, file_settings);
#elif defined(platform_windows)
    return getDirectoryListWin(dir_string, file_settings);
#endif
}

static void render_files(void) {
    
    if(file_settings->file_dirs[0] == NULL && conf_default_dir != NULL) {
        getDirectoryList(conf_default_dir);
    }
	
	/*
	else  {
		#if defined(platform_windows)
			getDirectoryListWin("", file_settings);
		#endif
    }*/
	
    int offset_y = 0;
    for (int i = 0; i < file_settings->file_dir_max_length; i++) {
        if (file_settings->file_dirs[i] != NULL) {
            if(i == file_settings->file_cursor_y) {
                cEngineRenderLabelWithParams(raster2d, file_settings->file_dirs[i], 0, offset_y-file_settings->file_cursor_y+10, color_text, -1);
            } else {
                cEngineRenderLabelWithParams(raster2d, file_settings->file_dirs[i], 0, offset_y-file_settings->file_cursor_y+10, color_inactive_text, -1);
            }
        }
        offset_y++;
    }
    
    int offset_x = 0;
    
    
    if(conf_default_dir != NULL) {
        cEngineRenderLabelWithParams(raster2d, "path:                                                                                            ", offset_x, 23, color_text, color_text_bg);
        cEngineRenderLabelWithParams(raster2d, conf_default_dir, offset_x+5, 23, color_text, color_text_bg);
    }

    int file_name_offset = 0;
    if(export_project && !file_editor_confirm_action) {
        cEngineRenderLabelWithParams(raster2d, "export wav as:                                                                                            ", offset_x, 22, color_file_name_text, color_text_bg);
        file_name_offset = 1;
    } else if(!file_settings->file_editor_save && !file_editor_confirm_action) {
        cEngineRenderLabelWithParams(raster2d, "open project:                                                                                            ", offset_x, 22, color_file_name_text, color_text_bg);
        file_name_offset = 0;
    } else if(!file_editor_confirm_action) {
        cEngineRenderLabelWithParams(raster2d, "save project as:                                                                                            ", offset_x, 22, color_file_name_text, color_text_bg);
        file_name_offset = 3;
    } else if(file_editor_existing_file) {
         cEngineRenderLabelWithParams(raster2d, "overwrite file? [PRESS RETURN]                                                                                             ", offset_x, 22, color_file_name_text, color_text_bg);
    } else {
        cEngineRenderLabelWithParams(raster2d, "current project will be reset. continue? [PRESS RETURN]                                                                                            ", offset_x, 22, color_file_name_text, color_text_bg);
    }
    
    if(file_settings->file_name != NULL && !file_editor_confirm_action) {
        cEngineRenderLabelWithParams(raster2d, file_settings->file_name, offset_x+13+file_name_offset, 22, color_file_name_text, color_text_bg);
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
            long sz = 0;
            sz = ftell(fp);
            //printf("file size:%ld\n", sz);
            char *b = NULL;
            b = cAllocatorAlloc(sizeof(char)*sz, "load file chars");
            if(b != NULL) {
                fseek(fp, 0, SEEK_SET);
                fread(b, sz, 1, fp);
                return b;
            } else {
                if(errorlog) { printf("buffer is null\n"); }
            }
            fclose(fp);
        } else {
            if(errorlog) { printf("file pointer is null\n"); }
        }
    } else {
        if(errorlog) { printf("cannot load, path is null\n"); }
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
        if(errorlog) { printf("could not load file.\n"); }
		set_info_timer("could not load file.");
    }
}

static void save_project_file(void) {

    if(file_settings->file_name != NULL) {
        char *save_path = cAllocatorAlloc(sizeof(char)*file_settings->file_name_max_length, "save_path chars");
        if(conf_default_dir != NULL) {
            sprintf(save_path, "%s%s.json", conf_default_dir, file_settings->file_name);
        } else {
            sprintf(save_path, "%s.json", file_settings->file_name);
        }
        
        cJSON *root = cSynthSaveProject(synth);
        if(root != NULL) {
            FILE * fp;
            fp = fopen(save_path, "w+");
            if(fp!= NULL) {
                char *json_print = cJSON_PrintUnformatted(root);
                fprintf(fp, "%s", json_print);
                fclose(fp);
                cJSON_Delete(root);
                free(json_print);
                set_info_timer(save_path);
            } else {
                set_info_timer("could not save file. Set proper path in config.");
            }
        }
        cAllocatorFree(save_path);
        exit_file_editor();
    } else {
        if(errorlog) { printf("cannot save, filename or path is null\n"); }
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
            if(errorlog) { printf("setInfoTimerWithInt: string too large\n"); }
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
            if(errorlog) { printf("setInfoTimerWithInt: string too large\n"); }
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
            cEngineRenderLabelWithParams(raster2d, info_string, 0, 23, color_text, color_info_text_bg);
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
    
    credits2d = cAllocatorAlloc(s_width * sizeof(unsigned int *), "main.c credits 2");
    if(credits2d == NULL) {
        fprintf(stderr, "out of memory\n");
    } else {
        for(i = 0; i < s_width; i++) {
            credits2d[i] = cAllocatorAlloc(s_height * sizeof(unsigned int), "main.c credits 3");
            if(credits2d[i] == NULL)
            {
                fprintf(stderr, "out of memory\n");
            }
        }
    }
    
    for(r_x = 0; r_x < s_width; r_x++) {
        for(r_y = 0; r_y < s_height; r_y++) {
            if(credits2d != NULL && credits2d[r_x] != NULL) {
                credits2d[r_x][r_y] = 0;
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
    for(i = 0; i < s_width; i++) {
        credits2d[i] = cAllocatorFree(credits2d[i]);
    }
    credits2d = cAllocatorFree(credits2d);
    cSynthCleanup(synth);
    cEngineCleanup();
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
    
    conf_default_dir = cAllocatorFree(conf_default_dir);
}

static void copy_instrument(int instrument) {
    
    if (instrument > -1 && instrument < synth->max_instruments) {
        copied_instrument = instrument;
    }
}

static void paste_instrument(int instrument) {
    
    if (instrument > -1 && instrument < synth->max_instruments
        && copied_instrument > -1 && copied_instrument < synth->max_instruments) {
        struct CInstrument *ins = synth->instruments[instrument];
        struct CInstrument *copied_ins = synth->instruments[copied_instrument];
        for(int c = 0; c < ins->adsr_nodes; c++) {
            ins->adsr[c]->amp = copied_ins->adsr[c]->amp;
            ins->adsr[c]->pos = copied_ins->adsr[c]->pos;
        }
        ins->adsr[0]->amp = copied_ins->adsr[0]->amp;
        ins->adsr[0]->pos = copied_ins->adsr[0]->pos;
        ins->adsr[1]->amp = copied_ins->adsr[1]->amp;
        ins->adsr[1]->pos = copied_ins->adsr[1]->pos;
        ins->adsr[2]->amp = copied_ins->adsr[2]->amp;
        ins->adsr[2]->pos = copied_ins->adsr[2]->pos;
        ins->adsr[3]->amp = copied_ins->adsr[3]->amp;
        ins->adsr[3]->pos = copied_ins->adsr[3]->pos;
        ins->adsr[4]->amp = copied_ins->adsr[4]->amp;
        ins->adsr[4]->pos = copied_ins->adsr[4]->pos;
        for(int c = 0; c < synth->max_instrument_effects; c++) {
            if (synth->instrument_effects[instrument][c] != NULL) {
                synth->instrument_effects[instrument][c] = cAllocatorFree(synth->instrument_effects[instrument][c]);
            }
            if (synth->instrument_effects[copied_instrument][c] != NULL) {
                synth->instrument_effects[instrument][c] = cSynthCopyTracknode(synth->instrument_effects[copied_instrument][c]);
            }
        }
    }
}

static void copy_notes(int track, int cursor_x, int cursor_y, int selection_x, int selection_y, bool cut, bool store) {
    
    cSynthCopyNotesFromSelection(synth, track, cursor_x, cursor_y, selection_x, selection_y, cut, store);
    last_selection_x = cursor_x;
    last_selection_y = cursor_y;
    last_selection_w = selection_x;
    last_selection_h = selection_y;
}

static void paste_notes(int track, int cursor_x, int cursor_y) {
    
    cSynthPasteNotesToPos(synth, track, cursor_x, cursor_y);
    selection_x = cursor_x + abs(last_selection_x - last_selection_w);
    selection_y = cursor_y + abs(last_selection_y - last_selection_h);
    selection_enabled = true;
}

static void copy_pattern(int cursor_x, int cursor_y) {
    
    if(cursor_y > 0 && cursor_y < 17) {
        last_copied_pattern_x = cursor_x;
        last_copied_pattern_y = cursor_y-1+visual_pattern_offset;
        if(debuglog) { printf("copy pattern x:%d y:%d", last_copied_pattern_x, last_copied_pattern_y); }
    }
    
}
static void paste_pattern(int cursor_x, int cursor_y) {
    
    int target_x = cursor_x;
    int target_y = cursor_y;
    if(target_y > 0 && target_y < 17) {
        target_y = target_y-1+visual_pattern_offset;
        cSynthPasteNotesFromPattern(synth, last_copied_pattern_x, last_copied_pattern_y, target_x, target_y);
        if(debug_log) { printf("paste pattern to x:%d y:%d", last_copied_pattern_x, last_copied_pattern_y); }
    }
}

static void add_track_node_with_octave(int x, int y, bool editing, int value) {
    
    //printf("track node value:%d\n", value);
    
    int x_count = visual_cursor_x%5;
    
    if(instrument_editor || pattern_editor || !editing) {
        // only allow preview of notes in editor
        cSynthAddTrackNode(synth, current_track, x, y, false, true, value+(octave*12), playing);
    } else {
        
        if(!editing) {
            cSynthAddTrackNode(synth, current_track, x, y, false, true, value+(octave*12), playing);
        } else {
            
            bool move_down = false;
            if(x_count == 0) {
                cSynthAddTrackNode(synth, current_track, x, y, editing, true, value+(octave*12), playing);
                if(editing) {
                    move_down = true;
                }
            }
            
            if(x_count == 1 && editing) {
                cSynthAddTrackNodeParams(synth, current_track, x, y, value, -1, -1, -1);
                // change instrument
                synth->current_instrument = value;
                move_down = true;
            }
            
            if(x_count == 2 && editing) {
                // change effect
                cSynthAddTrackNodeParams(synth, current_track, x, y, -1, (char)value, -1, -1);
                move_down = true;
            }
            
            if(x_count == 3 && editing) {
                // change param2
                cSynthAddTrackNodeParams(synth, current_track, x, y, -1, -1, (char)value, -1);
                move_down = true;
            }
            
            if(x_count == 4 && editing) {
                // change param1
                cSynthAddTrackNodeParams(synth, current_track, x, y, -1, -1, -1, (char)value);
                move_down = true;
            }
            
            if(move_down) {
                if(follow) {
                    if(playing) {}
                    else {
                        set_visual_cursor(0, 1, true);
                    }
                } else {
                    set_visual_cursor(0, 1, true);
                }
            }
        }
    }
}

// only move across active tracks
static void set_visual_cursor(int diff_x, int diff_y, bool user) {
    
    if(diff_x == 0 && diff_y == 1) {
        //printf("moving down true\n");
    }
    
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
    } else if(!editing) {
        selection_x = visual_cursor_x;
        selection_y = visual_cursor_y;
        selection_enabled = false;
    } else if(editing) {
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
            if(pattern_cursor_y > 0 && pattern_cursor_y < 17) {
                cSynthResetTrackProgress(synth, pattern_cursor_y-1+visual_pattern_offset, 0);
            } else {
                cSynthResetTrackProgress(synth, current_track, 0);
            }
        } else {
            cSynthResetTrackProgress(synth, current_track, 0);
        }
        cSynthResetTempoIndex(synth);
        synth->tempo_skip_step = true;
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
    
    if(help) {
        handle_help_keys(keysym);
        return;
    }
    
    if(credits) {
        handle_credits_keys(keysym);
        return;
    }
    
    if(file_editor) {
        handle_key_down_file(keysym);
        return;
    } else {
        
        switch(keysym->sym) {
            case SDLK_HOME:
                if(tempo_editor) {
                    tempo_selection_y = 0;
                } else if(instrument_editor) {
                    int ins_id = selected_instrument_id-1;
                    if(ins_id < 0) {
                        ins_id = synth->max_instruments-1;
                    }
                    selected_instrument_id = ins_id;
                    //printf("selected ins:%d", ins_id);
                } else if(pattern_editor) {
                    pattern_cursor_y = 0;
                } else {
                    visual_cursor_y = 0;
                }
                break;
            case SDLK_END:
                if(tempo_editor) {
                    tempo_selection_y = synth->tempo_height-1;
                } else if(instrument_editor) {
                    int ins_id = selected_instrument_id+1;
                    if(ins_id >= synth->max_instruments) {
                        ins_id = 0;
                    }
                    selected_instrument_id = ins_id;
                    //printf("selected ins:%d", ins_id);
                } else if(pattern_editor) {
                    pattern_cursor_y = synth->patterns_and_voices_height-1;
                }  else {
                    visual_cursor_y = synth->track_height-1;
                }
                break;
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
                        if(pattern_cursor_y == 17 || pattern_cursor_y == 18 || pattern_cursor_y == 19) {
                            int instrument = -1;
                            if(pattern_cursor_y < 19) {
                                int ins_nr = pattern_cursor_x;
                                if(pattern_cursor_y == 18) {
                                    ins_nr += 6;
                                }
                                instrument = ins_nr;
                            } else if(pattern_cursor_y == 19 && pattern_cursor_x < 4) {
                                int ins_nr = pattern_cursor_x;
                                ins_nr += 12;
                                instrument = ins_nr;
                            }
                            if(instrument > -1) {
                                set_info_timer("copy instrument");
                                copy_instrument(instrument);
                            }
                        } else {
                            copy_pattern(pattern_cursor_x, pattern_cursor_y);
                            set_info_timer("copy pattern");
                        }
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
                        if(pattern_cursor_y == 17 || pattern_cursor_y == 18 || pattern_cursor_y == 19) {
                            int instrument = -1;
                            if(pattern_cursor_y < 19) {
                                int ins_nr = pattern_cursor_x;
                                if(pattern_cursor_y == 18) {
                                    ins_nr += 6;
                                }
                                instrument = ins_nr;
                            } else if(pattern_cursor_y == 19 && pattern_cursor_x < 4) {
                                int ins_nr = pattern_cursor_x;
                                ins_nr += 12;
                                instrument = ins_nr;
                            }
                            if(instrument > -1) {
                                set_info_timer("paste instrument");
                                paste_instrument(instrument);
                            }
                        } else {
                            paste_pattern(pattern_cursor_x, pattern_cursor_y);
                            set_info_timer("paste pattern");
                        }
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
                    if(pattern_cursor_y > 0 && pattern_cursor_y < 17) {
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
                if(instrument_editor) {
                }
                else if(modifier) {
                    file_editor = true;
                    file_settings->file_editor_save = true;
                    export_project = true;
                    return;
                }
                else if(pattern_editor) {
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
                
                if(tempo_editor) {
                    tempo_selection_x--;
                    if(tempo_selection_x < 0) {
                        tempo_selection_x = synth->tempo_width-1;
                    }
                } else if(instrument_editor) {
                    if(instrument_editor_effects) {
                        instrument_editor_effects_x--;
                        if(instrument_editor_effects_x < 0) {
                            instrument_editor_effects_x = 2;
                        }
                    }
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
                
                if(tempo_editor) {
                    tempo_selection_x++;
                    if(tempo_selection_x >= synth->tempo_width) {
                        tempo_selection_x = 0;
                    }
                } else if(instrument_editor) {
                    if(instrument_editor_effects) {
                        instrument_editor_effects_x++;
                        if(instrument_editor_effects_x > 2) {
                            instrument_editor_effects_x = 0;
                        }
                    }
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
                if(tempo_editor) {
                    tempo_selection_y--;
                    if(tempo_selection_y < 0) {
                        tempo_selection_y = synth->tempo_height-1;
                    }
                } else if(instrument_editor) {
                    if(instrument_editor_effects) {
                        instrument_editor_effects_y--;
                        if(instrument_editor_effects_y < 0) {
                            instrument_editor_effects_y = visual_instrument_effects-1;
                        }
                    }
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
                if(tempo_editor) {
                    tempo_selection_y++;
                    if(tempo_selection_y >= synth->tempo_height) {
                        tempo_selection_y = 0;
                    }
                } else if(instrument_editor) {
                    if(instrument_editor_effects) {
                        instrument_editor_effects_y++;
                        if(instrument_editor_effects_y >= visual_instrument_effects) {
                            instrument_editor_effects_y = 0;
                        }
                    }
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
            case SDLK_DELETE:
                if(instrument_editor) {
                    if(instrument_editor_effects) {
                        instrument_effect_remove();
                    }
                }
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
                        
                        if(follow) {
                            if(playing) {}
                            else {
                                set_visual_cursor(0, 1, true);
                            }
                        } else {
                            set_visual_cursor(0, 1, true);
                        }
                    }
                }
                break;
            case SDLK_RETURN:
                
                if(tempo_editor) {
                    if(modifier) {
                        if (playing) {
                            synth->pending_tempo_column = tempo_selection_x;
                        } else {
                            synth->current_tempo_column = tempo_selection_x;
                        }
                    } else {
                        tempo_editor = false;
                    }
                } else if(instrument_editor) {
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
                        } else if(pattern_cursor_y == 20 && pattern_cursor_x == 4) {
                            tempo_editor = true;
                        } else if(pattern_cursor_y == 21 && pattern_cursor_x == 5) {
                            credits = true;
                        } else if(pattern_cursor_y == 21 && pattern_cursor_x == 1) {
                            help = true;
                        }
                    } else {
                        toggle_editing();
                    }
                }
                break;
            case SDLK_LSHIFT:
                if(instrument_editor) {
                    if(instrument_editor_effects) {
                        instrument_editor_effects = false;
                    } else {
                        instrument_editor_effects = true;
                    }
                } else {
                    shift_down = true;
                }
                break;
            default:
                break;
        }
    }
    
    if(shift_down && editing) {
        selection_enabled = true;
    }
    
    int x_count = visual_cursor_x%5;
    
    if(tempo_editor) {
        handle_tempo_keys(keysym);
        return;
    } else if(instrument_editor && instrument_editor_effects) {
        handle_instrument_effect_keys(keysym);
        return;
    } else if(pattern_editor) {
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


static void handle_tempo_keys(SDL_Keysym* keysym) {
    
    bool zero = false;
    bool move_cursor_down = false;
    int cursor_x = tempo_selection_x;
    int cursor_y = tempo_selection_y;
    char value = -1;
    
    switch(keysym->sym) {
        case SDLK_PLUS:
            if(cursor_y == 0) {
                synth->tempo_map[cursor_x][cursor_y]->bpm++;
                if(synth->tempo_map[cursor_x][cursor_y]->bpm > 999) {
                    synth->tempo_map[cursor_x][cursor_y]->bpm = 999;
                }
            }
            break;
        case SDLK_MINUS:
            if(cursor_y == 0) {
                synth->tempo_map[cursor_x][cursor_y]->bpm--;
                if(synth->tempo_map[cursor_x][cursor_y]->bpm < 0) {
                    synth->tempo_map[cursor_x][cursor_y]->bpm = 0;
                }
            }
            break;
        case SDLK_BACKSPACE:
        case SDLK_DELETE:
            zero = true;
			if(cursor_y > 0) {
				move_cursor_down = true;
			}
            break;
        case SDLK_a:
			if(cursor_y > 0) {
				if(synth->tempo_map[cursor_x][cursor_y]->active) {
					// check so that it's not the last one active. We need at least one.
					bool other_active_exists = false;
					for (int i = 1; i < synth->tempo_height; i++) {
						if(synth->tempo_map[cursor_x][i]->active && i != cursor_y) {
							other_active_exists = true;
						}
					}
					if(other_active_exists) {
						synth->tempo_map[cursor_x][cursor_y]->active = false;
					}
					move_cursor_down = true;
				} else {
					synth->tempo_map[cursor_x][cursor_y]->active = true;
					move_cursor_down = true;
				}
				cSynthUpdateHighlightInterval(synth);
			}
            break;
        case SDLK_0:
            value = 0;
            break;
        case SDLK_1:
            value = 1;
            break;
        case SDLK_2:
            value = 2;
            break;
        case SDLK_3:
            value = 3;
            break;
        case SDLK_4:
            value = 4;
            break;
        case SDLK_5:
            value = 5;
            break;
        case SDLK_6:
            value = 6;
            break;
        case SDLK_7:
            value = 7;
            break;
        case SDLK_8:
            value = 8;
            break;
        case SDLK_9:
            value = 9;
            break;
        default:
            break;
    }
    
    if(cursor_y > 0) {
        if(zero) {
            synth->tempo_map[cursor_x][cursor_y]->ticks = 1;
            move_cursor_down = true;
        } else if(value > 0) {
            synth->tempo_map[cursor_x][cursor_y]->ticks = value;
            move_cursor_down = true;
        }
        
    } else if(cursor_y == 0) {
        // bpm
        if(zero) {
            synth->tempo_map[cursor_x][cursor_y]->bpm = 0;
        } else if(value > -1) {
            int old_bpm = synth->tempo_map[cursor_x][cursor_y]->bpm;
            int number = value;
            if(old_bpm < 100) {
                old_bpm *= 10;
                number += old_bpm;
            } else if(old_bpm < 10) {
                old_bpm *= 10;
                number += old_bpm;
            }
            if(number >= 999) {
                number = 999;
            }
            synth->tempo_map[cursor_x][cursor_y]->bpm = number;
        }
    }
    
    if(move_cursor_down) {
        tempo_selection_y++;
        if(tempo_selection_y >= synth->tempo_height) {
            tempo_selection_y = 1;
        }
    }
}

static void handle_note_keys(SDL_Keysym* keysym) {
    
    //int cursor_y = synth->track_cursor_y;
    int cursor_y = visual_cursor_y;
    
    
    switch( keysym->sym ) {
        case SDLK_z:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 12);
            break;
        case SDLK_s:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 13);
            break;
        case SDLK_x:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 14);
            break;
        case SDLK_d:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 15);
            break;
        case SDLK_c:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 16);
            break;
        case SDLK_v:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 17);
            break;
        case SDLK_g:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 18);
            break;
        case SDLK_b:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 19);
            break;
        case SDLK_h:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 20);
            break;
        case SDLK_n:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 21);
            break;
        case SDLK_j:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 22);
            break;
        case SDLK_m:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 23);
            break;
        case SDLK_COMMA:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 24);
            break;
        case SDLK_l:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 25);
            break;
        case SDLK_PERIOD:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 26);
            break;
            
            //upper keyboard
        case SDLK_q:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 24);
            break;
        case SDLK_2:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 25);
            break;
        case SDLK_w:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 26);
            break;
        case SDLK_3:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 27);
            break;
        case SDLK_e:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 28);
            break;
        case SDLK_r:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 29);
            break;
        case SDLK_5:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 30);
            break;
        case SDLK_t:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 31);
            break;
        case SDLK_6:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 32);
            break;
        case SDLK_y:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 33);
            break;
        case SDLK_7:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 34);
            break;
        case SDLK_u:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 35);
            break;
        case SDLK_i:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 36);
            break;
        case SDLK_9:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 37);
            break;
        case SDLK_o:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 38);
            break;
        case SDLK_0:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 39);
            break;
        case SDLK_p:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 40);
            break;
            
        default:
            break;
    }
}

static void handle_pattern_keys(SDL_Keysym* keysym) {
    
    bool zero = false;
    int number = 0;
    switch(keysym->sym) {
        case SDLK_BACKSPACE:
        case SDLK_DELETE:
            zero = true;
            break;
        case SDLK_0:
            number = 0;
            break;
        case SDLK_1:
            number = 1;
            break;
        case SDLK_2:
            number = 2;
            break;
        case SDLK_3:
            number = 3;
            break;
        case SDLK_4:
            number = 4;
            break;
        case SDLK_5:
            number = 5;
            break;
        case SDLK_6:
            number = 6;
            break;
        case SDLK_7:
            number = 7;
            break;
        case SDLK_8:
            number = 8;
            break;
        case SDLK_9:
            number = 9;
            break;
        default:
            return;
            break;
    }

    
    if(pattern_cursor_y > 0 && pattern_cursor_y < 17) {
        if(zero) {
            synth->patterns[pattern_cursor_x][pattern_cursor_y-1+visual_pattern_offset] = 0;
        } else {
            if(number < 0){
                number = 0;
            }
            int old_pattern = synth->patterns[pattern_cursor_x][pattern_cursor_y-1+visual_pattern_offset];
            if(old_pattern < 10) {
                old_pattern *= 10;
                number += old_pattern;
                if(number >= synth->patterns_height) {
                    number = synth->patterns_height-1;
                }
            }
            synth->patterns[pattern_cursor_x][pattern_cursor_y-1+visual_pattern_offset] = number;
        }
    }
    
    // master amp
    if(pattern_cursor_y == 20 && pattern_cursor_x == 1) {
        if(zero) {
            synth->master_amp_percent = 0;
            synth->master_amp = 0;
        } else {
            int old_master_amp = synth->master_amp_percent;
            
            if(old_master_amp < 1000) {
                old_master_amp *= 10;
                number += old_master_amp;
            } else if(old_master_amp < 100) {
                old_master_amp *= 10;
                number += old_master_amp;
            } else if(old_master_amp < 10) {
                old_master_amp *= 10;
                number += old_master_amp;
            }
            
            if(number >= 9999) {
                number = 9999;
            }
            
            synth->master_amp_percent = number;
            synth->master_amp = synth->master_amp_percent*0.01;
        }
    }
    
    // rows
    if(pattern_cursor_y == 20 && pattern_cursor_x == 2) {
        if(zero) {
            synth->track_height = 1;
            visual_track_height = synth->track_height;
        } else {
            int track_height = synth->track_height;
            if(track_height < 10) {
                track_height *= 10;
                number += track_height;
            }
            if(number >= synth->track_max_height) {
                number = synth->track_max_height-1;
            }
            synth->track_height = number;
            if(synth->track_height < 1) {
                synth->track_height = 1;
            }
            visual_track_height = synth->track_height;
        }
    }
    
    // arp
    if(pattern_cursor_y == 20 && pattern_cursor_x == 3) {
        if(zero) {
            synth->arpeggio_speed = 1;
        } else {
            int old_arp = synth->arpeggio_speed;
            if(old_arp < 100) {
                old_arp *= 10;
                number += old_arp;
            } else if(old_arp < 10) {
                old_arp *= 10;
                number += old_arp;
            }
            if(number >= 600) {
                number = 600;
            }
            synth->arpeggio_speed = number;
        }
    }
}

void handle_instrument_keys(SDL_Keysym* keysym) {
    
    int cursor_y = visual_cursor_y;
    
    switch( keysym->sym ) {
        case SDLK_0:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 0);
            break;
        case SDLK_1:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 1);
            break;
        case SDLK_2:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 2);
            break;
        case SDLK_3:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 3);
            break;
        case SDLK_4:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 4);
            break;
        case SDLK_5:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 5);
            break;
        case SDLK_6:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 6);
            break;
        case SDLK_7:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 7);
            break;
        case SDLK_8:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 8);
            break;
        case SDLK_9:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 9);
            break;
        case SDLK_a:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 10);
            break;
        case SDLK_b:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 11);
            break;
        case SDLK_c:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 12);
            break;
        case SDLK_d:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 13);
            break;
        case SDLK_e:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 14);
            break;
        case SDLK_f:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 15);
            break;
        default:
            break;
    }
}

static void handle_effect_keys(SDL_Keysym* keysym) {
    
    //int cursor_y = synth->track_cursor_y;
    int cursor_y = visual_cursor_y;
    
    switch( keysym->sym ) {
        case SDLK_a:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 10);
            break;
        case SDLK_b:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 11);
            break;
        case SDLK_c:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 12);
            break;
        case SDLK_d:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 13);
            break;
        case SDLK_e:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 14);
            break;
        case SDLK_f:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 15);
            break;
        case SDLK_0:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 0);
            break;
        case SDLK_1:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 1);
            break;
        case SDLK_2:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 2);
            break;
        case SDLK_3:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 3);
            break;
        case SDLK_4:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 4);
            break;
        case SDLK_5:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 5);
            break;
        case SDLK_6:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 6);
            break;
        case SDLK_7:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 7);
            break;
        case SDLK_8:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 8);
            break;
        case SDLK_9:
            add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 9);
            break;
        default:
            break;
    }
}


static void instrument_effect_remove() {
    
    int instrument = selected_instrument_id;
    
    struct CTrackNode *node = synth->instrument_effects[instrument][instrument_editor_effects_y];
    if(node == NULL) {
        node = cSynthNewTrackNode();
        synth->instrument_effects[instrument][instrument_editor_effects_y] = node;
    }
    
    if(instrument_editor_effects_x == 0) {
        node->effect = '-';
        node->effect_value = -1;
    }
    
    if(instrument_editor_effects_x == 1) {
        node->effect_param1 = '-';
        node->effect_param1_value = -1;
    }
    
    if(instrument_editor_effects_x == 2) {
        node->effect_param2 = '-';
        node->effect_param2_value = -1;
    }
}

static void handle_instrument_effect_keys(SDL_Keysym* keysym) {
    
    int instrument = selected_instrument_id;
    char value = -1;
    
    switch(keysym->sym) {
        case SDLK_a:
            value = 10;
            break;
        case SDLK_b:
            value = 11;
            break;
        case SDLK_c:
            value = 12;
            break;
        case SDLK_d:
            value = 13;
            break;
        case SDLK_e:
            value = 14;
            break;
        case SDLK_f:
            value = 15;
            break;
        case SDLK_0:
            value = 0;
            break;
        case SDLK_1:
            value = 1;
            break;
        case SDLK_2:
            value = 2;
            break;
        case SDLK_3:
            value = 3;
            break;
        case SDLK_4:
            value = 4;
            break;
        case SDLK_5:
            value = 5;
            break;
        case SDLK_6:
            value = 6;
            break;
        case SDLK_7:
            value = 7;
            break;
        case SDLK_8:
            value = 8;
            break;
        case SDLK_9:
            value = 9;
            break;
        default:
            break;
            
    }
    
    //printf("value:%d ins:%d", value, instrument);
    
    if(value > -1) {
        struct CTrackNode *node = synth->instrument_effects[instrument][instrument_editor_effects_y];
        if(node == NULL) {
            node = cSynthNewTrackNode();
            synth->instrument_effects[instrument][instrument_editor_effects_y] = node;
        }
        
        if(instrument_editor_effects_x == 0) {
            node->effect = cSynthGetCharFromParam(value);
            node->effect_value = value;
        }
        
        if(instrument_editor_effects_x == 1) {
            node->effect_param1 = cSynthGetCharFromParam(value);
            node->effect_param1_value = value;
        }
        
        if(instrument_editor_effects_x == 2) {
            node->effect_param2 = cSynthGetCharFromParam(value);
            node->effect_param2_value = value;
        }
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
                /*
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
                 */
        }
    }
}


//const double ChromaticRatio = 1.059463094359295264562;
const double Tao = 6.283185307179586476925;

// 256, 512, 1024, 2048, 4096, 8192, 16384, 32768

Uint16 bufferSize = 8192; // must be a power of two, decrease to allow for a lower syncCompensationFactor to allow for lower latency, increase to reduce risk of underrun
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

void audio_callback(void *unused, Uint8 *byteStream, int byteStreamLength) {
    
    memset(byteStream, 0, byteStreamLength);
    
    if(quit || exporting) {
        return;
    }
    
    Sint16 *s_byteStream = (Sint16 *)byteStream;
    int remain = byteStreamLength / 2;
    
    long chunk_size = 64;
    int iterations = remain/chunk_size;
    
    for(long i = 0; i < iterations; i++) {
        long begin = i*chunk_size;
        long end = (i*chunk_size) + chunk_size;
        cSynthRenderAudio(synth, s_byteStream, begin, end, chunk_size, playing, exporting);
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
    } else if(y == 20 && x == 1) {
        //master amp
        int amp = synth->master_amp_percent;
        if(plus) {
            amp++;
            synth->master_amp_percent = amp;
        } else {
            amp--;
            if(amp < 1) {
                amp = 1;
            }
            synth->master_amp_percent = amp;
        }
        synth->master_amp = synth->master_amp_percent*0.01;
        
    } else if(y == 21 && x == 0) {
        //toggle preview 
        if(synth->preview_enabled) {
            synth->preview_enabled = false;
        } else {
            synth->preview_enabled = true;
        }
    /*
     This is now auto set from tempo.
    } else if(y == 21 && x == 1) {
        if(plus) {
            synth->track_highlight_interval++;
        } else {
            synth->track_highlight_interval--;
        }
        if(synth->track_highlight_interval < 2) {
            synth->track_highlight_interval = 2;
        }
    */
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
    } else if(y == 20) {
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
        if(instrument_editor_effects) {
            adsr_invert_y_render(i_pos_x, i_pos_y, color_inactive_text);
        } else {
            adsr_invert_y_render(i_pos_x, i_pos_y, color_envelop);
        }
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
    
    if(selected_instrument_node_index > 0 && !instrument_editor_effects) {
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
        if(instrument_editor_effects) {
            adsr_invert_y_render(g_pos, top_line_y, color_inactive_text);
            adsr_invert_y_render(g_pos, bottom_line_y, color_inactive_text);
        } else {
            adsr_invert_y_render(g_pos, top_line_y, color_text);
            adsr_invert_y_render(g_pos, bottom_line_y, color_text);
        }
    }
    
    for(i = 0; i < max_nodes-1; i++) {
        struct CadsrNode *node = ins->adsr[i];
        struct CadsrNode *node2 = ins->adsr[i+1];
        double g_amp = (node->amp*amp_factor) + inset_y;
        double g_pos = (node->pos*pos_factor) + inset_x - envelop_node_camera_offset;
        double g_amp2 = (node2->amp*amp_factor) + inset_y;
        double g_pos2 = (node2->pos*pos_factor) + inset_x - envelop_node_camera_offset;
        draw_line((int)g_pos, (int)g_amp, (int)g_pos2, (int)g_amp2);
    }
    
    // render dots for nodes
    for(i = 0; i < max_nodes; i++) {
        struct CadsrNode *node = ins->adsr[i];
        double g_amp = (node->amp*amp_factor) + inset_y;
        double g_pos = (node->pos*pos_factor) + inset_x;
        if(i == selected_instrument_node_index) {
            envelop_node_camera_offset = (int)(g_pos) - (s_width/2);
            if(envelop_node_camera_offset < 0) {
                envelop_node_camera_offset = 0;
            }
        }
        for(int x = -2; x < 2; x++) {
            for(int y = -2; y < 2; y++) {
                int color = color_inactive_instrument_node;
                if(instrument_editor_effects) {
                    color = color_inactive_text;
                } else if(i == selected_instrument_node_index) {
                    color = color_active_instrument_node;
                }
                adsr_invert_y_render(g_pos+x-envelop_node_camera_offset, g_amp+y, color);
            }
        }
    }
    
    char cval[64];
    char c = cSynthGetCharFromParam((char)selected_instrument_id);
    sprintf(cval, "instrument %c", c);
    cEngineRenderLabelWithParams(raster2d, cval, 1, 2, color_text, -1);
    
    // render preset instrument effects.
    int offset_y = 13;
    for (int x = 0; x < 3; x++) {
        for (int y = 0; y < visual_instrument_effects; y++) {
            
            //effect type
            char cval[20];
            struct CTrackNode *t = synth->instrument_effects[selected_instrument_id][y];
            
            int color = color_text;
            int bg_color = -1;
            if(x == instrument_editor_effects_x && y == instrument_editor_effects_y && instrument_editor_effects) {
                color = color_edit_text;
                bg_color = color_edit_marker;
            } else if(!instrument_editor_effects){
                color = color_inactive_text;
            }
            
            if(t != NULL) {
                
                if(x == 0) {
                    sprintf(cval, "%c", t->effect);
                    cEngineRenderLabelWithParams(raster2d, cval, x+1, y+offset_y, color, bg_color);
                }
                
                if(x == 1) {
                    sprintf(cval, "%c", t->effect_param1);
                    cEngineRenderLabelWithParams(raster2d, cval, x+1, y+offset_y, color, bg_color);
                }
                
                if(x == 2) {
                    sprintf(cval, "%c", t->effect_param2);
                    cEngineRenderLabelWithParams(raster2d, cval, x+1, y+offset_y, color, bg_color);
                }
            } else {
                cEngineRenderLabelWithParams(raster2d, "-", x+1, y+offset_y, color, bg_color);
            }
        }
    }
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
            
            int bg_color = -1;
            int color = color_text;
            if(x == pattern_cursor_x && y == pattern_cursor_y) {
                bg_color = color_edit_marker;
                color = color_edit_text;
            }
            
            if(y == 0) {
                int wave_color = color;
                if(synth->solo_voice > -1) {
                    if(x == synth->solo_voice) {
                        wave_color = color_solo_text;
                        bg_color = color_solo;
                    } else {
                        wave_color = color_mute_text;
                        bg_color = color_mute;
                    }
                } else if(synth->voices[x]->muted == 1) {
                    wave_color = color_mute_text;
                    bg_color = color_mute;
                }
                
                if(x == pattern_cursor_x && y == pattern_cursor_y) {
                    bg_color = color_edit_marker;
                    wave_color = color_edit_text;
                }
                
                int val = synth->patterns_and_voices[x][y];
                char cval[3];
                sprintf(cval, "%d", val);
                cEngineRenderLabelWithParams(raster2d, get_wave_type_as_char(val), x*10+inset_x, y+inset_y, wave_color, bg_color);
            } else if(y == 17) {
                char cval[10];
                int ins_nr = x;
                char c = cSynthGetCharFromParam((char)ins_nr);
                sprintf(cval, "ins %c", c);
                cEngineRenderLabelWithParams(raster2d, cval, x*10+inset_x, y+inset_y, color, bg_color);
            } else if(y == 18) {
                char cval[10];
                int ins_nr = x;
                ins_nr += 6;
                char c = cSynthGetCharFromParam((char)ins_nr);
                sprintf(cval, "ins %c", c);
                cEngineRenderLabelWithParams(raster2d, cval, x*10+inset_x, y+inset_y, color, bg_color);
            } else if(y == 19 && x < 4) {
                char cval[10];
                int ins_nr = x;
                ins_nr += 12;
                char c = cSynthGetCharFromParam((char)ins_nr);
                sprintf(cval, "ins %c", c);
                cEngineRenderLabelWithParams(raster2d, cval, x*10+inset_x, y+inset_y, color, bg_color);
            } else if(y == 20 && x == 1) {
                //nothing
                char cval[10];
                sprintf(cval, "amp %d%%", synth->master_amp_percent);
                if(synth->audio_clips) {
                    bg_color = color_file_name_text;
                    synth->audio_clips = false;
                }
                cEngineRenderLabelWithParams(raster2d, cval, x*10+inset_x, y+inset_y, color, bg_color);
            } else if(y == 20 && x == 2) {
                char cval[20];
                sprintf(cval, "rows %d", synth->track_height);
                cEngineRenderLabelWithParams(raster2d, cval, x*10+inset_x, y+inset_y, color, bg_color);
            } else if(y == 20 && x == 3) {
                char cval[20];
                sprintf(cval, "arp %d", synth->arpeggio_speed);
                cEngineRenderLabelWithParams(raster2d, cval, x*10+inset_x, y+inset_y, color, bg_color);
            } else if(y == 20 && x == 4) {
                //char cval[20];
                //sprintf(cval, "Groove %d", synth->swing);
                cEngineRenderLabelWithParams(raster2d, "tempo", x*10+inset_x, y+inset_y, color, bg_color);
            } else if(y == 21 && x == 0) {
                if(synth->preview_enabled) {
                    cEngineRenderLabelWithParams(raster2d, "preview 1", x*10+inset_x, y+inset_y, color, bg_color);
                } else {
                    cEngineRenderLabelWithParams(raster2d, "preview 0", x*10+inset_x, y+inset_y, color, bg_color);
                }
            } else if(y == 21 && x == 1) {
                //char cval[20];
                //sprintf(cval, "Beats %d", synth->track_highlight_interval);
                cEngineRenderLabelWithParams(raster2d, "help", x*10+inset_x, y+inset_y, color, bg_color);
            } else if(y == 21 && x == 5) {
                cEngineRenderLabelWithParams(raster2d, "credits", x*10+inset_x, y+inset_y, color, bg_color);
            } else if(y == 19) {
                //nothing
                cEngineRenderLabelWithParams(raster2d, "-", x*10+inset_x, y+inset_y, color, bg_color);
            } else if(y == 20) {
                //nothing
                cEngineRenderLabelWithParams(raster2d, "-", x*10+inset_x, y+inset_y, color, bg_color);
            } else if(y == 21) {
                //nothing
                cEngineRenderLabelWithParams(raster2d, "-", x*10+inset_x, y+inset_y, color, bg_color);
            } else {
                
                color = color_inactive_text;
                
                if(synth->active_tracks[y-1+visual_pattern_offset] == 1) {
                    bg_color = color_active_row;
                    color = color_active_row_text;
                    if(synth->solo_track == y-1+visual_pattern_offset) {
                        bg_color = color_solo;
                        color = color_solo_text;
                    }
                }
                
                if(synth->solo_track == y-1) {
                    bg_color = color_solo;
                    color = color_solo_text;
                }
                
                if(y-1+visual_pattern_offset == synth->current_track && playing) {
                    if(synth->current_track == synth->solo_track) {
                        bg_color = color_solo;
                    } else {
                        bg_color = color_playing_row;
                    }
                    color = color_playing_row_text;
                }
                
                if(x == pattern_cursor_x && y == pattern_cursor_y) {
                    bg_color = color_edit_marker;
                    color = color_edit_text;
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
                    cEngineRenderLabelWithParams(raster2d, cval, x_offset, y+1, color_text, -1);
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
        cEngineRenderLabelWithParams(raster2d, cval, 55, 23, cengine_color_white, -1);
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
        int wave_color = color_text;
        if(synth->solo_voice > -1) {
            if(x == synth->solo_voice) {
                wave_color = color_solo;
            } else {
                wave_color = color_mute;
            }
        } else if(synth->voices[x]->muted == 1) {
            wave_color = color_mute;
        }
        cEngineRenderLabelWithParams(raster2d, get_wave_type_as_char(val), 2+x*10, -visual_cursor_y+5, wave_color, -1);
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

static void render_tempo_editor(double dt) {
    
    int inset_x = 5;
    int inset_y = 1;

    /* if playing, make the column swap pending to let the current bar finish before switching. Make the pending bar blink to indicate the coming change.*/
    
    if(synth->pending_tempo_column > -1) {
		redraw_screen = true;
        synth->pending_tempo_blink_counter += dt;
        if (synth->pending_tempo_blink_counter > 200) {
            synth->pending_tempo_blink_counter_toggle = !synth->pending_tempo_blink_counter_toggle;
            synth->pending_tempo_blink_counter = 0;
        }
    }
    
    for(int x = 0; x < synth->tempo_width; x++) {
        for(int y = 0; y < synth->tempo_height; y++) {
            
            if(x == 0 && y > 0) {
                // print track numbers
                int track_nr = y-1;
                char cval[3];
                sprintf(cval, "%d", track_nr);
                int x_offset = 1;
                if(track_nr < 10) {
                    x_offset = 2;
                }
                cEngineRenderLabelWithParams(raster2d, cval, x_offset, y+1, color_text, -1);
            }

            int color = color_inactive_text;
            int bg_color = -1;
            
            char cval[10];
            struct CTempoNode *t = synth->tempo_map[x][y];
            
            
            if(synth->current_tempo_column == x) {
                if(t->active) {
                    bg_color = color_active_row;
                    color = color_active_row_text;
                } else {
                    color = color_text;
                }
                
                if(synth->tempo_index == y && playing) {
                    if(t->active) {
                        bg_color = color_playing_row;
                        color = color_playing_row_text;
                    }
                }
            } else if(synth->pending_tempo_column == x) {
                if(t->active && synth->pending_tempo_blink_counter_toggle) {
                    bg_color = color_active_row;
                    color = color_active_row_text;
                }
            } else {
                if(t->active) {
                    bg_color = color_inactive_text;
                    color = color_active_row_text;
                }
            }
            
            if(tempo_selection_x == x && tempo_selection_y == y) {
                bg_color = color_edit_marker;
                color = color_edit_text;
            }
            
            if (y == 0) {
                int bpm = t->bpm;
                sprintf(cval, "BPM %d", bpm);
            } else {
                char node_value = t->ticks;
                char c = cSynthGetCharFromParam((char)node_value);
                sprintf(cval, "%c", c);
            }
            cEngineRenderLabelWithParams(raster2d, cval, x*10+inset_x, y+inset_y, color, bg_color);
        }
    }
}

static void render_track(double dt) {
    
    if(help) {
        render_help();
        return;
    } else if(credits) {
        render_credits();
        return;
    } else if(instrument_editor && !file_editor) {
        render_instrument_editor(dt);
        return;
    } else if(tempo_editor && !file_editor) {
        render_tempo_editor(dt);
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
    int node_y = -1;
    int node_y_bright = 0;
    
    for (int y = 0; y < synth->track_height; y++) {
        offset_x = 0;
        node_y++;
        if(node_y > synth->track_highlight_interval-1) {
            node_y = 0;
        }
        for (int x = 0; x < visual_track_width; x++) {
            
            int bg_color = bg_color;
            int color = color_text;
            
            if(x >= 0 && x < 5) {
                bg_color = color_bg1;
                if(node_y == node_y_bright) {
                    bg_color = color_bg1_highlight;
                }
            }
            if(x >= 5 && x < 10) {
                bg_color = color_bg2;
                if(node_y == node_y_bright) {
                    bg_color = color_bg2_highlight;
                }
            }
            if(x >= 10 && x < 15) {
                bg_color = color_bg3;
                if(node_y == node_y_bright) {
                    bg_color = color_bg3_highlight;
                }
            }
            if(x >= 15 && x < 20) {
                bg_color = color_bg4;
                if(node_y == node_y_bright) {
                    bg_color = color_bg4_highlight;
                }
            }
            if(x >= 20 && x < 25) {
                bg_color = color_bg5;
                if(node_y == node_y_bright) {
                    bg_color = color_bg5_highlight;
                }
            }
            if(x >= 25 && x < 30) {
                bg_color = color_bg6;
                if(node_y == node_y_bright) {
                    bg_color = color_bg6_highlight;
                }
            }
            
            node_x = (int)floor(x/5);
            
            if(selection_enabled && editing) {
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
                    color = color_selection_text;
                    bg_color = color_selection_marker;
                    if(visual_cursor_x == x && visual_cursor_y == y) {
                        bg_color = color_edit_marker;
                    }
                }
            }

            if(synth->track_progress_int == y && playing == 1) {
                if(synth->current_track == current_track) {
                    bg_color = color_playing_row;
                    color = color_playing_row_text;
                }
            }
            
            if(visual_cursor_x == x && visual_cursor_y == y) {
                if(editing == 1) {
                    bg_color = color_edit_marker;
                    color = color_edit_text;
                } else {
                    bg_color = color_marker;
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
        sprintf(cval, "p:%d t:%d r:%d", current_pattern, current_track, visual_cursor_y);
        cEngineRenderLabelWithParams(raster2d, cval, 50, 23, color_text, -1);
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
            if(debuglog) { SDL_Log("Could not get display mode for video display #%d: %s", i, SDL_GetError()); }
			st_pause();
		} else {
			// On success, print the current display mode.
            if(debuglog) { SDL_Log("Display #%d: current display mode is %dx%dpx @ %dhz. \n", i, current.w, current.h, current.refresh_rate); }
		}
	}
	
    //SDL_WINDOW_FULLSCREEN
    if(fullscreen) {
        window = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN);
    } else {
        window = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL /*| SDL_WINDOW_FULLSCREEN*/);
    }
    
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
            
            if(fullscreen) {
                SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");  // make the scaled rendering look smoother.
                SDL_RenderSetLogicalSize(renderer, width, height);
            }
            
            SDL_GL_SetSwapInterval(1);
            char title_string[256];
            sprintf(title_string, "%s (build:%d)", title, synth->build_number);
            SDL_SetWindowTitle(window, title_string);
            visual_track_height = synth->track_height;
        } else {
            if(errorlog) { printf("Failed to create renderer: %s", SDL_GetError()); }
			st_pause();
        }
	} else {
        if(errorlog) { printf("Failed to create window:%s", SDL_GetError()); }
		st_pause();
    }
}

static void setup_synth(void) {
    
    synth = cSynthContextNew();
    synth->interleaved = true;
    synth->debuglog = debuglog;
    synth->errorlog = errorlog;
    synth->chunk_size = 64;
    synth->preview_enabled = preview_enabled;
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
    
    if(debuglog) {
        printf("\naudioSpec want\n");
        printf("----------------\n");
        printf("sample rate:%d\n", want.freq);
        printf("channels:%d\n", want.channels);
        printf("samples:%d\n", want.samples);
        printf("----------------\n\n");
    }
    
    AudioDevice = SDL_OpenAudioDevice(NULL, 0, &want, &audioSpec, 0);
    
    if(debuglog) {
        printf("\naudioSpec get\n");
        printf("----------------\n");
        printf("sample rate:%d\n", audioSpec.freq);
        printf("channels:%d\n", audioSpec.channels);
        printf("samples:%d\n", audioSpec.samples);
        printf("size:%d\n", audioSpec.size);
        printf("----------------\n");
    }
    
    if (AudioDevice == 0) {
        if(errorlog) { printf("\nFailed to open audio: %s\n", SDL_GetError()); }
		st_pause();
        return 1;
    }
    
    if (audioSpec.format != want.format) {
        if(errorlog) { printf("\nCouldn't get requested audio format.\n"); }
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
    if(debuglog) { printf("allocs before cleanup:\n"); }
    cAllocatorPrintAllocationCount();
    cleanup_data();
    if(debuglog) { printf("allocs after cleanup:\n"); }
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
    
    int delay_ms = active_render_delay_ms;
    if(passive_rendering == false) {
        delay_ms = passive_render_delay_ms;
    }
    
    if(lock_device) {
        SDL_LockAudioDevice(AudioDevice);
    }
    
    check_sdl_events(event);
    update_and_render_info(last_dt);
    
    if(instrument_editor) {
        synth->needs_redraw = true;
    }
    
    if(credits) {
        synth->needs_redraw = true;
    }
    
	if(tempo_editor) {
		synth->needs_redraw = true; 
	}
    
    if(redraw_screen || synth->needs_redraw || !passive_rendering) {
        render_track(last_dt);
        
        for (int r_x = 0; r_x < s_width; r_x++) {
            for (int r_y = 0; r_y < s_height; r_y++) {
                raster[r_x+r_y*s_width] = raster2d[r_x][r_y];
            }
        }
        
        for(int x = 0; x < s_width; x++) {
            for(int y = 0; y < s_height; y++) {
                raster2d[x][y] = color_bg;
            }
        }
        redraw_screen = false;
        synth->needs_redraw = false;
    }
    
    
    SDL_UpdateTexture(texture, NULL, raster, s_width * sizeof (unsigned int));
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
    
    if(lock_device) {
        SDL_UnlockAudioDevice(AudioDevice);
    }
    
    int dt = get_delta();
    last_dt = dt;
    int wait_time = delay_ms-dt;
    if(dt < 10) {
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

static void load_config(void) {
    
    bool success = false;
    char *b = load_file("config.txt");
    
    if(b != NULL) {
        success = false;
        success = parse_config(b);
        cAllocatorFree(b);
    }
    
    if(!success) {
        if(debuglog) { printf("could not find config file. Writing config.txt.\n"); }
        bufferSize = 8192;
        FILE * fp;
        fp = fopen("config.txt", "w+");
        if(fp != NULL) {
            fprintf(fp, "%s", "{\"buffer_size\":8192,\"buffer_size_info\":\"Must be a power of two, for example 256, 512, 1024, 2048, 4096, 8192, 16384\", \"working_dir_path\":\"\",\"working_dir_path_info\":\"Use full path like /Users/d/Desktop/snibbetracker_workspace/ dir must be created manually. The default (empty path) will use the directory the executable is in. \",\"passive_rendering\":true, \"fullscreen\":false}");
            fclose(fp);
        }
    }
}

static unsigned int get_color_from_json_config(cJSON *color_obj) {
    
    cJSON *rgb_obj = NULL;
    unsigned int ret = 0;
    if(color_obj != NULL) {
        rgb_obj = cJSON_GetObjectItem(color_obj, "r");
        int r = rgb_obj->valueint;
        rgb_obj = cJSON_GetObjectItem(color_obj, "g");
        int g = rgb_obj->valueint;
        rgb_obj = cJSON_GetObjectItem(color_obj, "b");
        int b = rgb_obj->valueint;
        
        // 255 255 255 makes it transparent for some reason..
        
        if(r > 254) {
            r = 254;
        }
        
        if(g > 254) {
            g = 254;
        }
        
        if(b > 254) {
            b = 254;
        }
        
        unsigned char c_r = (unsigned char)r;
        unsigned char c_g = (unsigned char)g;
        unsigned char c_b = (unsigned char)b;
        ret = (255 << 24) | ((unsigned char)c_r << 16) | ((unsigned char)c_g << 8) | (unsigned char)c_b;
    }
    
    return ret;
}

static bool parse_config(char *json) {
    
    cJSON *root = NULL;
    cJSON *object = NULL;
    char *param_buffer_size = "buffer_size";
    char *param_working_dir_path = "working_dir_path";
    char *param_passive_rendering = "passive_rendering";
    char *param_fullscreen = "fullscreen";
    char *param_preview = "preview";
    
    char *param_color_info_text_bg = "color_info_text_bg";
    char *param_color_file_name_text = "color_file_name_text";
    char *param_color_inactive_instrument_node = "color_inactive_instrument_node";
    char *param_color_active_instrument_node = "color_active_instrument_node";
    char *param_color_envelop = "color_envelop";
    char *param_color_inactive_text = "color_inactive_text";
    char *param_color_text = "color_text";
    char *param_color_text_bg = "color_text_bg";
    char *param_color_marker = "color_marker";
    char *param_color_solo = "color_solo";
    char *param_color_solo_text = "color_solo_text";
    char *param_color_mute = "color_mute";
    char *param_color_mute_text = "color_mute_text";
    char *param_color_active_row = "color_active_row";
    char *param_color_active_row_text = "color_active_row_text";
    char *param_color_playing_row = "color_playing_row";
    char *param_color_playing_row_text = "color_playing_row_text";
    char *param_color_edit_marker = "color_edit_marker";
    char *param_color_edit_text = "color_edit_text";
    char *param_color_selection_marker = "color_selection_marker";
    char *param_color_selection_text = "color_selection_text";
    char *param_color_bg = "color_bg";
    char *param_color_bg1 = "color_bg1";
    char *param_color_bg2 = "color_bg2";
    char *param_color_bg3 = "color_bg3";
    char *param_color_bg4 = "color_bg4";
    char *param_color_bg5 = "color_bg5";
    char *param_color_bg6 = "color_bg6";
    char *param_color_bg1_highlight = "color_bg1_highlight";
    char *param_color_bg2_highlight = "color_bg2_highlight";
    char *param_color_bg3_highlight = "color_bg3_highlight";
    char *param_color_bg4_highlight = "color_bg4_highlight";
    char *param_color_bg5_highlight = "color_bg5_highlight";
    char *param_color_bg6_highlight = "color_bg6_highlight";
    

    root = cJSON_Parse(json);
    if(root != NULL) {
        
        // buffer size
        object = cJSON_GetObjectItem(root, param_buffer_size);
        if(object != NULL) {
            bufferSize = 8192;
            int buffer_index_value = object->valueint;
            bufferSize = (Uint16)buffer_index_value;
        } else {
            if(errorlog) { printf("could not find buffersize in config.\n"); }
        }
        
        // path
        bool path_in_config = false;
        object = cJSON_GetObjectItem(root, param_working_dir_path);
        if(object != NULL) {
            char *path = object->valuestring;
            if(path != NULL) {
                if(strlen(path) > 0) {
                    conf_default_dir = cAllocatorAlloc((1024 * sizeof(char*)), "conf default dir");
                    sprintf(conf_default_dir, "%s", path);
                    if(debuglog) { printf("path in config:%s\n", conf_default_dir); }
                    path_in_config = true;
                }
            } else {
                if(debuglog) { printf("could not find path in config 1.\n"); }
            }
        } else {
            if(debuglog) { printf("could not find path in config 2.\n"); }
        }
        
        if (!path_in_config) {
            #if defined(platform_osx)
                // get default path from ObjC.
                char *default_dir = get_user_default_dir();
                conf_default_dir = cAllocatorAlloc((1024 * sizeof(char*)), "conf default dir");
                sprintf(conf_default_dir, "%s", default_dir);
                if(debuglog) { printf("using default dir:%s\n", conf_default_dir); }
                free(default_dir);
            #elif defined(platform_windows)
				conf_default_dir = cAllocatorAlloc((1024 * sizeof(char*)), "conf default dir 2");
				sprintf(conf_default_dir, "%s", "");
			#endif
        }
        
        // passive rendering
        object = cJSON_GetObjectItem(root, param_passive_rendering);
        if(object != NULL) {
            bool passive_render_val = object->valueint;
            if(passive_render_val) {
                if(debuglog) { printf("passive rendering in config is true\n"); }
                passive_rendering = true;
            } else {
                if(debuglog) { printf("passive rendering in config is false\n"); }
                passive_rendering = false;
            }
        } else {
            if(debuglog) { printf("could not find passive rendering in config.\n"); }
        }
        
        // fullscreen
        object = cJSON_GetObjectItem(root, param_fullscreen);
        if(object != NULL) {
            bool fullscreen_value = object->valueint;
            if(fullscreen_value) {
                if(debuglog) { printf("fullscreen in config is true\n"); }
                fullscreen = true;
            } else {
                if(debuglog) { printf("fullscreen in config is false\n"); }
                fullscreen = false;
            }
        } else {
            if(debuglog) { printf("could not find fullscreen in config.\n"); }
        }
        
        // preview
        object = cJSON_GetObjectItem(root, param_preview);
        if(object != NULL) {
            bool fullscreen_value = object->valueint;
            if(fullscreen_value) {
                if(debuglog) { printf("preview in config is true\n"); }
                preview_enabled = true;
            } else {
                if(debuglog) { printf("preview in config is false\n"); }
                preview_enabled = false;
            }
        } else {
            if(debuglog) { printf("could not find preview in config.\n"); }
        }
        
        object = cJSON_GetObjectItem(root, param_color_info_text_bg);
        if(object != NULL) { color_info_text_bg = get_color_from_json_config(object); }
        
        object = cJSON_GetObjectItem(root, param_color_file_name_text);
        if(object != NULL) { color_file_name_text = get_color_from_json_config(object); }
        
        object = cJSON_GetObjectItem(root, param_color_inactive_instrument_node);
        if(object != NULL) { color_inactive_instrument_node = get_color_from_json_config(object); }
        
        object = cJSON_GetObjectItem(root, param_color_active_instrument_node);
        if(object != NULL) { color_active_instrument_node = get_color_from_json_config(object); }
        
        object = cJSON_GetObjectItem(root, param_color_envelop);
        if(object != NULL) { color_envelop = get_color_from_json_config(object); }
        
        object = cJSON_GetObjectItem(root, param_color_inactive_text);
        if(object != NULL) { color_inactive_text = get_color_from_json_config(object); }
        
        object = cJSON_GetObjectItem(root, param_color_text);
        if(object != NULL) { color_text = get_color_from_json_config(object); }
        
        object = cJSON_GetObjectItem(root, param_color_text_bg);
        if(object != NULL) { color_text_bg = get_color_from_json_config(object); }
       
        object = cJSON_GetObjectItem(root, param_color_marker);
        if(object != NULL) { color_marker = get_color_from_json_config(object); }
        
        object = cJSON_GetObjectItem(root, param_color_solo);
        if(object != NULL) { color_solo = get_color_from_json_config(object); }
        
        object = cJSON_GetObjectItem(root, param_color_solo_text);
        if(object != NULL) { color_solo_text = get_color_from_json_config(object); }
        
        object = cJSON_GetObjectItem(root, param_color_mute);
        if(object != NULL) { color_mute = get_color_from_json_config(object); }
        
        object = cJSON_GetObjectItem(root, param_color_mute_text);
        if(object != NULL) { color_mute_text = get_color_from_json_config(object); }
        
        object = cJSON_GetObjectItem(root, param_color_active_row);
        if(object != NULL) { color_active_row = get_color_from_json_config(object); }
        
        object = cJSON_GetObjectItem(root, param_color_active_row_text);
        if(object != NULL) { color_active_row_text = get_color_from_json_config(object); }
        
        object = cJSON_GetObjectItem(root, param_color_playing_row);
        if(object != NULL) { color_playing_row = get_color_from_json_config(object); }
        
        object = cJSON_GetObjectItem(root, param_color_playing_row_text);
        if(object != NULL) { color_playing_row_text = get_color_from_json_config(object); }
        
        object = cJSON_GetObjectItem(root, param_color_edit_marker);
        if(object != NULL) { color_edit_marker = get_color_from_json_config(object); }
        
        object = cJSON_GetObjectItem(root, param_color_edit_text);
        if(object != NULL) { color_edit_text = get_color_from_json_config(object); }
       
        object = cJSON_GetObjectItem(root, param_color_selection_marker);
        if(object != NULL) { color_selection_marker = get_color_from_json_config(object); }
        
        object = cJSON_GetObjectItem(root, param_color_selection_text);
        if(object != NULL) { color_selection_text = get_color_from_json_config(object); }
        
        
        object = cJSON_GetObjectItem(root, param_color_bg);
        if(object != NULL) { color_bg = get_color_from_json_config(object); }
        
        object = cJSON_GetObjectItem(root, param_color_bg1);
        if(object != NULL) { color_bg1 = get_color_from_json_config(object); }
        
        object = cJSON_GetObjectItem(root, param_color_bg2);
        if(object != NULL) { color_bg2 = get_color_from_json_config(object); }
        
        object = cJSON_GetObjectItem(root, param_color_bg3);
        if(object != NULL) { color_bg3 = get_color_from_json_config(object); }
        
        object = cJSON_GetObjectItem(root, param_color_bg4);
        if(object != NULL) { color_bg4 = get_color_from_json_config(object); }
        
        object = cJSON_GetObjectItem(root, param_color_bg5);
        if(object != NULL) { color_bg5 = get_color_from_json_config(object); }
        
        object = cJSON_GetObjectItem(root, param_color_bg6);
        if(object != NULL) { color_bg6 = get_color_from_json_config(object); }
        
        object = cJSON_GetObjectItem(root, param_color_bg1_highlight);
        if(object != NULL) { color_bg1_highlight = get_color_from_json_config(object); }
        
        object = cJSON_GetObjectItem(root, param_color_bg2_highlight);
        if(object != NULL) { color_bg2_highlight = get_color_from_json_config(object); }
        
        object = cJSON_GetObjectItem(root, param_color_bg3_highlight);
        if(object != NULL) { color_bg3_highlight = get_color_from_json_config(object); }
        
        object = cJSON_GetObjectItem(root, param_color_bg4_highlight);
        if(object != NULL) { color_bg4_highlight = get_color_from_json_config(object); }
        
        object = cJSON_GetObjectItem(root, param_color_bg5_highlight);
        if(object != NULL) { color_bg5_highlight = get_color_from_json_config(object); }
        
        object = cJSON_GetObjectItem(root, param_color_bg6_highlight);
        if(object != NULL) { color_bg6_highlight = get_color_from_json_config(object); }
        
        
        cJSON_Delete(root);
        return true;
    }
    return false;
}


static void st_pause(void) {
    
	SDL_Delay(5000);
}
static void st_log(char *message) {
    
    if(debuglog) {
        printf("*** %s \n", message);
    }
}

int main(int argc, char* argv[]) {
    
    #if defined(platform_windows)
        load_config();
        st_log("started executing.");
    #elif defined(platform_osx)
        //osx, load from bundle
        char *settings = get_settings_json();
        parse_config(settings);
        free(settings);
    #else
        //linux
    #endif
    
    
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
    cSynthResetTempoIndex(synth);
    exporting = true;
    synth->looped = false;
    
    unsigned long buffer_size = 0;
    long chunk_size = 64;
    Sint16 *buffer = NULL;
    long iterations = LONG_MAX;
   
    // calculate size of file and allocate buffer etc
    for(long i = 0; i < iterations; i++) {
        long begin = i*chunk_size;
        long end = (i*chunk_size) + chunk_size;
        cSynthRenderAudio(synth, NULL, begin, end, chunk_size, playing, exporting);
        if(synth->looped) {
            //printf("synth->looped is now true! breaking.\n");
            synth->looped = false;
            break;
        }
        buffer_size += chunk_size*2;
        //printf("buffer_size:%lu\n", buffer_size);
    }
    
    if(debuglog) { printf("export buffer size:%ld\n", buffer_size); }
    synth->current_track = starting_track;
    cSynthResetTrackProgress(synth, starting_track, 0);
    exporting = true;
    synth->looped = false;
    
    // alloc buffer
    buffer = cAllocatorAlloc(sizeof(Sint16) * buffer_size, "export buffer");
    for(int i = 0; i < buffer_size; i++) {
        buffer[i] = 0;
    }
    
    // render to buffer
    for(long i = 0; i < iterations; i++) {
        long begin = i*chunk_size;
        long end = (i*chunk_size) + chunk_size;
        cSynthRenderAudio(synth, buffer, begin, end, chunk_size, playing, exporting);
        if(synth->looped) {
            synth->looped = false;
            break;
        }
    }
    
    write_wav(filename, buffer_size, buffer, synth->sample_rate);
    if(debuglog) { printf("total buffer size: %lu\n", buffer_size); }
    cAllocatorFree(buffer);
    exporting = false;
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
    if(wav_file != NULL) {
        
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
    } else {
        set_info_timer("could not export file. Set proper path in config.");
    }
}


static void handle_credits_keys(SDL_Keysym* keysym) {
    
    credits_left = false;
    credits_right = false;
    credits_up = false;
    credits_down = false;
    
    switch( keysym->sym ) {
        case SDLK_RETURN:
        case SDLK_ESCAPE:
            credits = false;
            break;
        case SDLK_LEFT:
            credits_left = true;
            break;
        case SDLK_RIGHT:
            credits_right = true;
            break;
        case SDLK_UP:
            credits_up = true;
            break;
        case SDLK_DOWN:
            credits_down = true;
            break;
        case SDLK_BACKSPACE:
            break;
        case SDLK_SPACE:
            toggle_playback();
            break;
        default:
            break;
    }
}

static void handle_help_keys(SDL_Keysym* keysym) {
    
    switch( keysym->sym ) {
        case SDLK_RETURN:
        case SDLK_ESCAPE:
            help = false;
            break;
        case SDLK_LEFT:
            break;
        case SDLK_RIGHT:
            break;
        case SDLK_UP:
            help_index--;
            if(help_index < 0) {
                help_index = help_index_max-1;
            }
            break;
        case SDLK_DOWN:
            help_index++;
            if(help_index >= help_index_max) {
                help_index = 0;
            }
            break;
        case SDLK_BACKSPACE:
            break;
        case SDLK_SPACE:
            toggle_playback();
            break;
        default:
            break;
    }
}

static void render_help(void) {
    
    /*
     
     The movement should feel light fluid and responsive but still have weight.
     Most of the weight is in attacking, getting attacked, block, parry, roll etc.
     
     
     How to know which page relates to which view?
     If switching to a page automatically because of context, it can get annoying
     if you only want to read about effects.
     
     Only make scroller for now.
     */
    
    int x = 0;
    int offset_x = 1;
    int inset_x = 1;
    int y = 1;
    int color = color_text;
    int bg_color = -1;
    
    // make char* to
    int page = help_index;
    
    if(page == 0) {
        cEngineRenderLabelWithParams(raster2d, "trackview", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "----------------", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "- enter: toggle editing on/off.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "- space: play/stop.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "- arrow keys: move cursor.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "- tab: go to pattern view.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        #if defined(platform_windows)
            cEngineRenderLabelWithParams(raster2d, "- ctrl+left/right: change octave up/down.", inset_x+x+offset_x, y, color, bg_color);
            y++;
            cEngineRenderLabelWithParams(raster2d, "- ctrl+up/down: move notes below cursor.", inset_x+x+offset_x, y, color, bg_color);
            y++;
            cEngineRenderLabelWithParams(raster2d, "- ctrl+c/v: copy paste note (or selection).", inset_x+x+offset_x, y, color, bg_color);
            y++;
            cEngineRenderLabelWithParams(raster2d, "- ctrl+f: toggle play cursor follow.", inset_x+x+offset_x, y, color, bg_color);
            y++;
        #elif defined(platform_osx)
            cEngineRenderLabelWithParams(raster2d, "- cmd+left/right: change octave up/down.", inset_x+x+offset_x, y, color, bg_color);
            y++;
            cEngineRenderLabelWithParams(raster2d, "- cmd+up/down: move notes below cursor.", inset_x+x+offset_x, y, color, bg_color);
            y++;
            cEngineRenderLabelWithParams(raster2d, "- cmd+c/v: copy paste note (or selection).", inset_x+x+offset_x, y, color, bg_color);
            y++;
            cEngineRenderLabelWithParams(raster2d, "- cmd+f: toggle play cursor follow.", inset_x+x+offset_x, y, color, bg_color);
            y++;
        #endif
        cEngineRenderLabelWithParams(raster2d, "- shift+arrow keys: make selection.(if edit is on)", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "- character keys: play notes or edit effects.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "- home/end: go to top / bottom.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        y++;
        cEngineRenderLabelWithParams(raster2d, "[note] [instrument number] [effects]", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "E-2", inset_x+x+offset_x, y, color, cengine_color_bg1_highlight);
        cEngineRenderLabelWithParams(raster2d, "0", inset_x+x+offset_x+4, y, color, cengine_color_bg1_highlight);
        cEngineRenderLabelWithParams(raster2d, "047", inset_x+x+offset_x+6, y, color, cengine_color_bg1_highlight);
        y++;
        
        cEngineRenderLabelWithParams(raster2d, "1 / 7", 1, 22, color, bg_color);
    }
    
    if(page == 1) {
        cEngineRenderLabelWithParams(raster2d, "patternview", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "----------------", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "- arrow keys: move around grid.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "- plus/minus: cycle waveform, pattern numbers, rows etc.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "- enter: go to instrument view (when gridcursor is at Ins 0-F)", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "- tab: go to track view.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "- e: jump to trackview with current position.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "- m: mute track (or channel if cursor is at the top)", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "- a: activate/inactivate track.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "- s: solo track (or channel if cursor is at the top)", inset_x+x+offset_x, y, color, bg_color);
        y++;
        #if defined(platform_windows)
            cEngineRenderLabelWithParams(raster2d, "- ctrl+up/down: cycle tracks (0-63).", inset_x+x+offset_x, y, color, bg_color);
            y++;
            cEngineRenderLabelWithParams(raster2d, "- ctrl+c/v: copy paste track data.", inset_x+x+offset_x, y, color, bg_color);
            y++;
        #elif defined(platform_osx)
            cEngineRenderLabelWithParams(raster2d, "- cmd+up/down: cycle tracks (0-63).", inset_x+x+offset_x, y, color, bg_color);
            y++;
            cEngineRenderLabelWithParams(raster2d, "- cmd+c/v: copy paste track data.", inset_x+x+offset_x, y, color, bg_color);
            y++;
        #endif
        cEngineRenderLabelWithParams(raster2d, "- home/end: go to top / bottom.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "amp - master amplitude.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "rows - number of active rows in patterns.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "arp - general arpeggio speed.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "preview - 1 or 0. if notes are audiable when editing.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "tempo - open tempo editor.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "credits - show credits.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        /*
         Amp - master amplitude, used both for previewing and exporting. Shows red if clipping.
         Rows - number of active rows in patterns.
         Arp - arpeggio speed.
         Preview - toggle for if notes should be audiable when playing on the keyboard.
         Tempo - open tempo editor.
         Credits - show credits.
         */
        
        cEngineRenderLabelWithParams(raster2d, "2 / 7", 1, 22, color, bg_color);
    }
    
    if(page == 2) {
        cEngineRenderLabelWithParams(raster2d, "instrument view", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "----------------", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "- arrow keys: move node.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        #if defined(platform_windows)
            cEngineRenderLabelWithParams(raster2d, "- ctrl+arrow keys: move node slow.", inset_x+x+offset_x, y, color, bg_color);
            y++;
        #elif defined(platform_osx)
            cEngineRenderLabelWithParams(raster2d, "- cmd+arrow keys: move node slow.", inset_x+x+offset_x, y, color, bg_color);
            y++;
        #endif
        cEngineRenderLabelWithParams(raster2d, "- tab: cycle nodes.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "- spacebar: go to pattern view.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "- shift: toggle editing of envelop or effects.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "- home/end: cycle instruments.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        
        cEngineRenderLabelWithParams(raster2d, "3 / 7", 1, 22, color, bg_color);
    }
    
    if(page == 3) {
        cEngineRenderLabelWithParams(raster2d, "tempo view", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "----------------", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "- a: activate/inactivate row. each column", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "  must have at least one active row.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "- plus/minus: change BPM on top row.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "- 1-9: change BPM on top row, or beats.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        #if defined(platform_windows)
            cEngineRenderLabelWithParams(raster2d, "- ctrl+enter: switch tempo column. while", inset_x+x+offset_x, y, color, bg_color);
            y++;
        #elif defined(platform_osx)
            cEngineRenderLabelWithParams(raster2d, "- cmd+enter: switch tempo column. while", inset_x+x+offset_x, y, color, bg_color);
            y++;
        #endif
        cEngineRenderLabelWithParams(raster2d, "  playing, column will be armed and switched", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "  to when the current pattern has finished.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        
        cEngineRenderLabelWithParams(raster2d, "4 / 7", 1, 22, color, bg_color);
    }
    
    if(page == 4) {
        
        cEngineRenderLabelWithParams(raster2d, "global controls", inset_x+x+offset_x, y, color, bg_color);
        y++;
        #if defined(platform_windows)
            cEngineRenderLabelWithParams(raster2d, "----------------", inset_x+x+offset_x, y, color, bg_color);
            y++;
            cEngineRenderLabelWithParams(raster2d, "- ctrl+s: go to save view.", inset_x+x+offset_x, y, color, bg_color);
            y++;
            cEngineRenderLabelWithParams(raster2d, "- ctrl+o: go to load view.", inset_x+x+offset_x, y, color, bg_color);
            y++;
            cEngineRenderLabelWithParams(raster2d, "- ctrl+e: export to wav.", inset_x+x+offset_x, y, color, bg_color);
            y++;
        #elif defined(platform_osx)
            cEngineRenderLabelWithParams(raster2d, "----------------", inset_x+x+offset_x, y, color, bg_color);
            y++;
            cEngineRenderLabelWithParams(raster2d, "- cmd+s: go to save view.", inset_x+x+offset_x, y, color, bg_color);
            y++;
            cEngineRenderLabelWithParams(raster2d, "- cmd+o: go to load view.", inset_x+x+offset_x, y, color, bg_color);
            y++;
            cEngineRenderLabelWithParams(raster2d, "- cmd+e: export to wav.", inset_x+x+offset_x, y, color, bg_color);
            y++;
        #endif
        y++;
        cEngineRenderLabelWithParams(raster2d, "save/load view", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "----------------", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "- character keys: enter filename.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "- enter: save/load file.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "- escape: exit view.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        
        cEngineRenderLabelWithParams(raster2d, "5 / 7", 1, 22, color, bg_color);
        
    }
    
    if(page == 5) {
        cEngineRenderLabelWithParams(raster2d, "effects 1(2)", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "----------------", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "0xx - arpeggio (second tone halfsteps, third tone halfsteps)", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "      change speed in settings:Arp xx.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "1xx - arpeggio speed (speed, speed) use one of the values or", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "       both multiplied.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "3xx - portamento (speed, speed) uses a single value if other", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "      is 0 or a multiplication of both. sets the speed to when", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "      new notes will be reached.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "4xx - vibrato (speed, depth).", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "5xx - distortion (amp, amp).", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "6xx - link distortion (channel, [unused]) premix current", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "      channel with another channel (0-6).", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "7xx - detune (amount, amount) 88 is middle.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "8xx - PWM (linear position/oscillation depth, oscillation", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "      speed) on squarewave. if param2 is present, param1", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "      will be used for osc depth. FM for other wavetypes", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "      (depth, speed).", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "9xx - change waveform. (channel 0-5, wavetype 0-4: sine, saw,", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "      square, tri, noise).", inset_x+x+offset_x, y, color, bg_color);
        y++;
       
        
        cEngineRenderLabelWithParams(raster2d, "6 / 7", 1, 22, color, bg_color);
        
    }
    
    if(page == 6) {
        cEngineRenderLabelWithParams(raster2d, "effects 2(2)", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "----------------", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "Axx - (left amplitud, right amplitud) can be used for", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "      amplitude, pan och turning off a note.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "Bxx - downsample sweep down (linear, sweep) sweep works on", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "      noise channel. choose either linear or sweep.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "Cxx - downsample sweep up (linear, sweep) sweep works on", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "      noise channel. choose either linear or sweep.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "Dxx - ends pattern. D11 - jump to next pattern and reset", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "      tempo seq. D1x - reset tempo seq. D2x - switch", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "      tempo_seq column. x = tempo seq column (0-5).", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "Exx - pitch up (fast, slow) Works on non-noise channels.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "      both values can be combined to increase effect.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "Fxx - pitch down (fast, slow) Works on non-noise channels.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "      both values can be combined to increase effect.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        
        cEngineRenderLabelWithParams(raster2d, "7 / 7", 1, 22, color, bg_color);
    }
    /*
     effects
     ----------------
     0xx - arpeggio (second tone halfsteps, third tone halfsteps) change speed in settings:Arp xx.
     1xx - arpeggio speed (speed, speed) use one of the values or both multiplied.
     3xx - portamento (speed, speed) uses a single value if other is 0 or a multiplication of both. Sets the speed to when new notes will be reached.
     4xx - vibrato (speed, depth).
     5xx - distortion (amp, amp).
     6xx - link distortion (channel, [unused]) premix current channel with another channel (0-6).
     7xx - detune (amount, amount) 88 is middle.
     8xx - PWM (linear position/oscillation depth, oscillation speed) on squarewave. If param2 is present, param1 will be used for osc depth. FM for other wavetypes (depth, speed).
     9xx - change waveform. (channel 0-5, wavetype 0-4: sine, saw, square, tri, noise).
     Axx - (left amplitud, right amplitud) can be used for amplitude, pan och turning off a tone.
     Bxx - downsample sweep down (linear, sweep) Works best on noise channel. Choose either linear or sweep.
     Cxx - downsample sweep up (linear, sweep) Works best on noise channel. Choose either linear or sweep.
     Dxx - ends pattern. D11 - jump to next pattern and reset tempo seq. D1x - reset tempo seq. D2x - switch tempo_seq column. x = tempo seq column (0-5).
     Exx - pitch up (fast, slow) Works on non-noise channels. Both values can be combined to increase effect.
     Fxx - pitch down (fast, slow) Works on non-noise channels. Both values can be combined to increase effect.
    */
    
    /*
     
     tempo view
     ----------------
     - a: activate/inactivate row. each column must have at least one active row.
     - plus/minus: change BPM on top row.
     - 1-9: change BPM on top row, or beats.
     - modifier+enter: switch tempo column. while playing, column will be armed and switched to when the current pattern has finished.
     
     
     instrument view
     ----------------
     - arrow keys: move node.
     - modifier+arrow keys: move node slow.
     - tab: cycle nodes.
     - spacebar: go to pattern view.
     - shift: toggle editing of envelop or effects.
     - home/end: cycle instruments.
     
     trackview
     ----------------
     - spacebar: toggle editing on/off.
     - enter: play/stop.
     - arrow keys: move cursor.
     - tab: go to pattern view.
     - modifier+left/right: change octave up/down.
     - modifier+up/down: move notes below cursor.
     - shift+arrow keys: make selection.
     - modifier+c/v: copy paste note (or selection).
     - character keys: play notes or edit effects.
     - modifier+f: toggle play cursor follow.
     - home/end: go to top / bottom.
     track format explanation:
     a = note, b = instrument number, ccc = effects. 6 supported channels.
     a b ccc | a b ccc | a b ccc | a b ccc | a b ccc | a b ccc
     
     patternview
     ----------------
     - arrow keys: move around grid.
     - plus/minus: cycle waveform, pattern numbers, bpm, swing, active etc.
     - spacebar: go to instrument view (when gridcursor is at Ins 0-F)
     - tab: go to track view.
     - e: jump to trackview with current position.
     - m: mute track (or channel if cursor is at the top)
     - a: activate/inactivate track.
     - s: solo track (or channel if cursor is at the top)
     - modifier+up/down: cycle tracks (0-63).
     - modifier+c/v: copy paste track data.
     - home/end: go to top / bottom.
     
    
     
     
     global controls
     ----------------
     - modifier+s: go to save view
     - modifier+o: go to load view
     - modifier+e: export to wav.
     
     save view
     ----------------
     - arrow keys: navigate file system.
     - character keys: enter filename.
     - enter: save file at the current location.
     - escape: exit save view.
     
     load view
     ----------------
     - arrow keys: navigate file system.
     - enter: load the file.
     - escape: exit load view.
     
     effects
     ----------------
     0xx - arpeggio (second tone halfsteps, third tone halfsteps) change speed in settings:Arp xx.
     1xx - arpeggio speed (speed, speed) use one of the values or both multiplied.
     3xx - portamento (speed, speed) uses a single value if other is 0 or a multiplication of both. Sets the speed to when new notes will be reached.
     4xx - vibrato (speed, depth).
     5xx - distortion (amp, amp).
     6xx - link distortion (channel, [unused]) premix current channel with another channel (0-6).
     7xx - detune (amount, amount) 88 is middle.
     8xx - PWM (linear position/oscillation depth, oscillation speed) on squarewave. If param2 is present, param1 will be used for osc depth. FM for other wavetypes (depth, speed).
     9xx - change waveform. (channel 0-5, wavetype 0-4: sine, saw, square, tri, noise).
     Axx - (left amplitud, right amplitud) can be used for amplitude, pan och turning off a tone.
     Bxx - downsample sweep down (linear, sweep) Works best on noise channel. Choose either linear or sweep.
     Cxx - downsample sweep up (linear, sweep) Works best on noise channel. Choose either linear or sweep.
     Dxx - ends pattern. D11 - jump to next pattern and reset tempo seq. D1x - reset tempo seq. D2x - switch tempo_seq column. x = tempo seq column (0-5).
     Exx - pitch up (fast, slow) Works on non-noise channels. Both values can be combined to increase effect.
     Fxx - pitch down (fast, slow) Works on non-noise channels. Both values can be combined to increase effect.
     
     Amp - master amplitude, used both for previewing and exporting. Shows red if clipping.
     Active - number of active pattern rows.
     Rows - number of active rows in patterns.
     Arp - arpeggio speed.
     Preview - toggle for if notes should be audiable when playing on the keyboard.
     Tempo - open tempo editor.
     Credits - show credits.
     */
}

static void render_credits(void) {
    
    if(!credits_init) {
        resetColorValues();
        credits_init = true;
    }
    
    credits_hue_rotation += credits_hue_rotation_inc;
    if(credits_hue_rotation > 360) {
        credits_hue_rotation = 0;
    }
    
    if (credits_left) {
        credits_cursor_x--;
        if (credits_cursor_x < 10) {
            credits_cursor_x = 10;
        }
    } else if(credits_right) {
        credits_cursor_x++;
        if (credits_cursor_x > s_width) {
            credits_cursor_x = s_width;
        }
    } else if(credits_up) {
        credits_cursor_y--;
        if (credits_cursor_y < 10) {
            credits_cursor_y = 0;
        }
    } else if(credits_down) {
        credits_cursor_y++;
        if (credits_cursor_y > s_height) {
            credits_cursor_y = s_height;
        }
    }
    
    
    redraw_screen = true;
    int int_x = 0;
    int inset_x = 0;
    int inset_y = 0;
    int int_y = 4;
    int inc = 10;
    
    
    int brush_size = 10;
    for(int x = credits_cursor_x-brush_size; x < credits_cursor_x+brush_size; x++) {
        for(int y = credits_cursor_y-brush_size; y < credits_cursor_y+brush_size; y++) {
            if(check_screen_bounds(x, y)) {
                credits2d[x][y] = credits_brush_color;
            }
        }
    }
    
    int color = credits_color;
    int bg_color = credits_bg_color;
    
    credits_x += credits_x_inc;
    credits_y += credits_y_inc;
    
    int_x = (int)credits_x;
    int_y = (int)credits_y;
    
    bool change_colors = false;
    if(credits_x > s_width-235) {
        credits_x_inc = -credits_x_inc;
        credits_x = s_width-236;
        change_colors = true;
    } else if(credits_x < 0) {
        credits_x_inc = -credits_x_inc;
        credits_x = 1;
        change_colors = true;
    }
    
    if(credits_y > s_height-195) {
        credits_y_inc = -credits_y_inc;
        credits_y = s_height-196;
        change_colors = true;
    } else if(credits_y < 0) {
        credits_y_inc = -credits_y_inc;
        credits_y = 1;
        change_colors = true;
    }
    
    
    if(change_colors) {
        
        if(credits_x_inc < 0) {
            credits_x_inc = (double)(rand() % 120 / 100.0);
            credits_x_inc = -credits_x_inc;
        } else {
            credits_x_inc = (double)(rand() % 120 / 100.0);
        }
        if(credits_y_inc < 0) {
            credits_y_inc = (double)(rand() % 120 / 100.0);
            credits_y_inc = -credits_y_inc;
        } else {
            credits_y_inc = (double)(rand() % 120 / 100.0);
        }
        
        credits_hue_rotation_inc = (float)(rand() % 100 / 100.0);
        credits_bg_color = rand();
        credits_color = abs(INT_MAX-credits_bg_color);
        credits_brush_color = rand();
        
        if(rand() % 2 == 1) {
            credits_bg_color = -1;
        }
        
        credits_higher_contrast = false;
        credits_scanlines_x = false;
        credits_scanlines_y = false;
        
        /*
        if(rand() % 2 == 1) {
            credits_higher_contrast = true;
        }
        
        if(rand() % 2 == 1) {
            credits_scanlines_x = true;
        }
        
        if(rand() % 2 == 1) {
            credits_scanlines_y = true;
        }
        */
        
        change_colors = false;
    }
    
    
    cEngineRenderLabelByPixelPos(credits2d, "_code_and_design_", int_x+inset_x, int_y+inset_y, color, bg_color);
    inset_y+=inc;
    cEngineRenderLabelByPixelPos(credits2d, "   lundstroem", int_x+inset_x, int_y+inset_y, color, bg_color);
    inset_y+=inc;
    inset_y+=inc;
    cEngineRenderLabelByPixelPos(credits2d, "_feedback_and_testing_", int_x+inset_x, int_y+inset_y, color, bg_color);
    inset_y+=inc;
    cEngineRenderLabelByPixelPos(credits2d, "   salkinitzor", int_x+inset_x, int_y+inset_y, color, bg_color);
    inset_y+=inc;
    cEngineRenderLabelByPixelPos(credits2d, "   nordloef", int_x+inset_x, int_y+inset_y, color, bg_color);
    inset_y+=inc;
    cEngineRenderLabelByPixelPos(credits2d, "   Linde", int_x+inset_x, int_y+inset_y, color, bg_color);
    inset_y+=inc;
    cEngineRenderLabelByPixelPos(credits2d, "   sunfl0wr", int_x+inset_x, int_y+inset_y, color, bg_color);
    inset_y+=inc;
    cEngineRenderLabelByPixelPos(credits2d, "   Rockard", int_x+inset_x, int_y+inset_y, color, bg_color);
    inset_y+=inc;
    cEngineRenderLabelByPixelPos(credits2d, "   0c0", int_x+inset_x, int_y+inset_y, color, bg_color);
    inset_y+=inc;
    inset_y+=inc;
    cEngineRenderLabelByPixelPos(credits2d, "_special_thanks_to_", int_x+inset_x, int_y+inset_y, color, bg_color);
    inset_y+=inc;
    cEngineRenderLabelByPixelPos(credits2d, "   OlofsonArcade", int_x+inset_x, int_y+inset_y, color, bg_color);
    inset_y+=inc;
    cEngineRenderLabelByPixelPos(credits2d, "   goto80", int_x+inset_x, int_y+inset_y, color, bg_color);
    inset_y+=inc;
    cEngineRenderLabelByPixelPos(credits2d, "   lolloise", int_x+inset_x, int_y+inset_y, color, bg_color);
    inset_y+=inc;
    cEngineRenderLabelByPixelPos(credits2d, "   crabinfo", int_x+inset_x, int_y+inset_y, color, bg_color);
    inset_y+=inc;
    inset_y+=inc;
    cEngineRenderLabelByPixelPos(credits2d, "_etc_", int_x+inset_x, int_y+inset_y, color, bg_color);
    inset_y+=inc;
    cEngineRenderLabelByPixelPos(credits2d, "   (C) Palestone Software 2015", int_x+inset_x, int_y+inset_y, color, bg_color);
    
    int hue_x = int_x;
    int hue_y = int_y;
    int hue_width = 240;
    int hue_height = 195;
   
    if (hue_x >= s_width) {
        hue_x = s_width-1;
    }
    
    if (hue_y >= s_height) {
        hue_y = s_height-1;
    }

    if (hue_width+hue_x >= s_width) {
        hue_width = s_width-hue_x-1;
        if(hue_width < 0) {
            hue_width = 0;
        }
    }
    
    if (hue_height+hue_y > s_height) {
        hue_height = s_height-hue_y-1;
        if(hue_height < 0) {
            hue_height = 0;
        }
    }
    
    if(hue_x+hue_width <= s_width && hue_y+hue_height <= s_height && hue_x > 0 && hue_y > 0) {
        renderPixels(credits2d, hue_x, hue_y, hue_width, hue_height, credits_hue_rotation);
    }
    
    for(int x = 0; x < s_width; x++) {
        for(int y = 0; y < s_height; y++) {
            raster2d[x][y] = credits2d[x][y];
        }
    }
}



static void resetColorValues(void)
{
    c_hue_a = 0;
    c_hue_r = 0;
    c_hue_g = 0;
    c_hue_b = 0;
    
    //c_new_hue_a = 0;
    c_new_hue_r = 0;
    c_new_hue_g = 0;
    c_new_hue_b = 0;
}



static void TtransformHSV(float H, float S, float V)
{
    float VSU = 0.1f;
    float VSW = 0.1f;
    
    if(credits_higher_contrast) {
        VSU = (float)(credits_hue_rotation * 0.01);
        VSW = (float)(credits_hue_rotation * 0.01);
    } else {
        VSU = (float)(V*S*cos(H*M_PI/180.0f));
        VSW = (float)(V*S*sin(H*M_PI/180));
    }
    
    c_new_hue_r = (float)(.299*V+.701*VSU+.168*VSW)*c_hue_r
    + (float)(.587*V-.587*VSU+.330*VSW)*c_hue_g
    + (float)(.114*V-.114*VSU-.497*VSW)*c_hue_b;
    
    c_new_hue_g = (float)(.299*V-.299*VSU-.328*VSW)*c_hue_r
    + (float)(.587*V+.413*VSU+.035*VSW)*c_hue_g
    + (float)(.114*V-.114*VSU+.292*VSW)*c_hue_b;
    
    c_new_hue_b = (float)(.299*V-.3*VSU+1.25*VSW)*c_hue_r
    + (float)(.587*V-.588*VSU-1.05*VSW)*c_hue_g
    + (float)(.114*V+.886*VSU-.203*VSW)*c_hue_b;
}

static void renderPixels(unsigned int **data, int start_x, int start_y, int w, int h, float rotation) {
    
    if (data != NULL) {

        for(int x = start_x; x < w+start_x; x++) {

            if(credits_scanlines_x) {
                if (x % 2 == 0) {
                    continue;
                }
            }
            
            for(int y = start_y; y < h+start_y; y++) {

                if(credits_scanlines_y) {
                    if (y % 2 == 0) {
                        continue;
                    }
                }
                
                unsigned int rast = data[x][y];
                unsigned char red = rast & 0xff;
                unsigned char green = (rast >> 8) & 0xff;
                unsigned char blue = (rast >> 16) & 0xff;
                
                float rscale = red /255.0f;
                float gscale = green /255.0f;
                float bscale = blue /255.0f;
                
                c_hue_r = rscale;
                c_hue_g = gscale;
                c_hue_b = bscale;
                

                TtransformHSV(rotation, 1.0f, 0.99f);
                
                c_new_hue_r = (float)(c_new_hue_r*255.0);
                c_new_hue_g = (float)(c_new_hue_g*255.0);
                c_new_hue_b = (float)(c_new_hue_b*255.0);
                
                if (c_new_hue_r > 255) {
                    c_new_hue_r = 255;
                }
                
                if (c_new_hue_g > 255) {
                    c_new_hue_g = 255;
                }
                
                if (c_new_hue_b > 255) {
                    c_new_hue_b = 255;
                }
                
                if(c_new_hue_r < 0) {
                    c_new_hue_r = 0;
                }
                
                if(c_new_hue_g < 0) {
                    c_new_hue_g = 0;
                }
                
                if(c_new_hue_b < 0) {
                    c_new_hue_b = 0;
                }
                
                data[x][y] = (255 << 24) | ((unsigned char)c_new_hue_b << 16) | ((unsigned char)c_new_hue_g << 8) | (unsigned char)c_new_hue_r;
                
            }
        }
    }
}


//static void renderPixels(char *data, int w, int h, float rotation)
//{
//    if (data != NULL)
//    {
//        // **** You have a pointer to the image data ****
//        // **** Do stuff with the data here ****
//        
//        for(int i = 0; i < w*h; i++)
//        {
//            
//            int offset = 4*i;
//            unsigned char alpha = data[offset];
//            unsigned char red = data[offset+1];
//            unsigned char green = data[offset+2];
//            unsigned char blue = data[offset+3];
//            
//            float ascale = alpha /255.0f;
//            float rscale = red /255.0f;
//            float gscale = green /255.0f;
//            float bscale = blue /255.0f;
//            
//            c_hue_a = ascale;
//            c_hue_r = rscale;
//            c_hue_g = gscale;
//            c_hue_b = bscale;
//            
//            c_new_hue_a = ascale;
//            
//            TtransformHSV(rotation, 1.0, 0.99);
//            
//            c_new_hue_a = (c_new_hue_a*255.0);
//            c_new_hue_r = (c_new_hue_r*255.0);
//            c_new_hue_g = (c_new_hue_g*255.0);
//            c_new_hue_b = (c_new_hue_b*255.0);
//            
//            if (c_new_hue_a > 255)
//            {
//                c_new_hue_a = 255;
//            }
//            if (c_new_hue_r > 255)
//            {
//                c_new_hue_r = 255;
//            }
//            if (c_new_hue_g > 255)
//            {
//                c_new_hue_g = 255;
//            }
//            if (c_new_hue_b > 255)
//            {
//                c_new_hue_b = 255;
//            }
//            
//            if(c_new_hue_a < 0)
//                c_new_hue_a = 0;
//            
//            if(c_new_hue_r < 0)
//                c_new_hue_r = 0;
//            
//            if(c_new_hue_g < 0)
//                c_new_hue_g = 0;
//            
//            if(c_new_hue_b < 0)
//                c_new_hue_b = 0;
//            
//            //data[offset] = new_hue_a;
//            
//            /*for (int r_x = 0; r_x < s_width; r_x++) {
//             for (int r_y = 0; r_y < s_height; r_y++) {
//             raster[r_x+r_y*s_width] = raster2d[r_x][r_y];
//             }
//             }*/
//            
//            /*
//            data[offset+1] = c_new_hue_r;
//            data[offset+2] = c_new_hue_g;
//            data[offset+3] = c_new_hue_b;
//            */
//        }
//    }
//}
