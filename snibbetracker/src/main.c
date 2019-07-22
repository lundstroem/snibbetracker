
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

static const bool debuglog = false;
static const bool errorlog = false;
static const int passive_render_delay_ms = 16;
static const int active_render_delay_ms = 16;
static bool log_file_enabled = false;
static bool run_with_sdl = true;
static bool redraw_screen = true;
static bool passive_rendering = true;
static bool preview_enabled = true;
static char *conf_default_dir = NULL;
static int current_pattern = 0;
static int current_track = 0;
static int quit = 0;
static char *title = "snibbetracker 1.1.1";
static struct CInput *input = NULL;
static unsigned int *raster = NULL;
static unsigned int **raster2d = NULL;
static unsigned int *raw_sheet = NULL;
static unsigned int **credits2d = NULL;
static unsigned int **visualiser2d = NULL;
static int width = 256*4;
static int height = 144*4;
static int s_width = 256*2; // 512
static int s_height = 144*2; // 288
static bool playing = false;
static bool exporting = false;
static bool editing = false;
static bool modifier = false;
static bool shift_down = false;
static bool selection_enabled = false;
static bool follow = false;
static bool visualiser = false;
static bool credits = false;
static bool help = false;
static bool reset_confirmation = false;
static int help_index = 0;
static int help_index_max = 8;
static bool export_project = false;
static int octave = 2;
static int step_size = 1;
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
static bool custom_table = false;
static bool instrument_editor_effects = false;
static int instrument_editor_effects_x = 0;
static int instrument_editor_effects_y = 0;
static int visual_instrument_effects = 5;
static int selected_instrument_id = 0;
static int selected_instrument_node_index = 1;
static int selected_custom_table_node_index = 0;
static int copied_instrument = -1;
static bool file_editor = false;
static bool file_editor_confirm_action = false;
static bool file_editor_existing_file = false;
static bool pressed_left = false;
static bool pressed_right = false;
static bool pressed_up = false;
static bool pressed_down = false;
static bool tempo_editor = false;
static bool any_key_pressed = false;
static int tempo_selection_x = 0;
static int tempo_selection_y = 0;
static bool wavetable_editor = false;
static int wavetable_selection_x = 0;
static int wavetable_selection_y = 0;
static struct CSynthContext *synth = NULL;
static struct CTimer *info_timer = NULL;
static char *info_string = NULL;
static struct FileSettings *file_settings = NULL;
static int envelope_node_camera_offset = 0;
static int fullscreen = 0;
static int fps_print_interval = 0;
static int print_interval_limit = 100;
static int old_time = 0;
static int last_dt = 16;
static SDL_Keycode last_key = 0;

// 256, 512, 1024, 2048, 4096, 8192, 16384, 32768
Uint16 bufferSize = 4096; // must be a power of two, decrease to allow for a lower syncCompensationFactor to allow for lower latency, increase to reduce risk of underrun
Uint32 samplesPerFrame = 0;
Uint32 msPerFrame = 0;
double practicallySilent = 0.001;
SDL_atomic_t audioCallbackLeftOff;
Sint32 audioMainLeftOff = 0;
Sint8 audioMainAccumulator = 0;
SDL_AudioDeviceID AudioDevice;
SDL_AudioSpec audioSpec;
SDL_Event event;
SDL_bool running = SDL_TRUE;
SDL_Window *window = NULL;
SDL_Texture *texture = NULL;
SDL_Renderer *renderer = NULL;
SDL_GLContext context;

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
static void transformHSV(float H, float S, float V);
static void renderPixels(unsigned int **data, int start_x, int start_y, int w, int h, float rotation);

#define MAX_TOUCHES 8
#define sheet_width 1024
#define sheet_height 1024

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
#define cengine_color_transparent 0x00000000

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
unsigned int color_envelope = cengine_color_green;
unsigned int color_visualiser = cengine_color_green;
unsigned int color_visualiser_clipping = cengine_color_red;
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

static void reset_project(void);
static void init_file_settings(void);
static bool file_exists(char *path);
static void handle_credits_keys();
static void handle_help_keys();
static void handle_key_down_file();
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
static void update_info(double dt);
static void render_info(void);
static void init_data(void);
static void convert_input(int x, int y);
static void cleanup_data(void);
static void copy_instrument(int instrument);
static void paste_instrument(int instrument);
static void copy_notes(int track, int cursor_x, int cursor_y, int selection_x, int selection_y, bool cut, bool store);
static void transpose_selection(int track, int cursor_x, int cursor_y, int selection_x, int selection_y, bool up, int amount);
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
static void move_patterns(int direction);
static void change_octave(bool up);
void handle_key_up(SDL_Keysym* keysym);
void handle_key_down(SDL_Keysym* keysym);
static void sdl_key_set_locks(SDL_Keysym* keysym, bool down);
static void sdl_key_mapping(SDL_Keysym* keysym, bool down);
static void handle_reset_confirmation_keys(void);
static void handle_note_keys(void);
static void handle_pattern_keys(void);
static void handle_param_value(void);
static void handle_tempo_keys(void);
static void handle_wavetable_keys(void);
static void instrument_effect_remove(void);
static void handle_instrument_effect_keys(void);
static void check_sdl_events(SDL_Event event);
static int get_delta(void);
static void log_wave_data(float *floatStream, Uint32 floatStreamLength, Uint32 increment);
void audio_callback(void *unused, Uint8 *byteStream, int byteStreamLength);
static void clear_clipping_color_from_visualiser(void);
static void prepare_visualiser(Sint16 *s_byteStream, int byteStreamLength);
static void change_waveform(int plus);
static void change_param(bool plus);
static void draw_line(int x0, int y0, int x1, int y1);
static void render_custom_table(double dt);
static void render_instrument_editor(double dt);
static void adsr_invert_y_render(double x, double y, int color);
static void render_pattern_mapping(void);
static char *get_wave_type_as_char(int type);
static void draw_wave_types(void);
static void render_visualiser(void);
static void render_help(void);
static void set_view(int view);
static void render_credits(void);
static void render_tempo_editor(double dt);
static void render_reset_confirmation(double dt);
static void render_wavetable_editor(double dt);
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
static void copy_project_win(const char *name);
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

static void reset_project(void) {
    
    current_pattern = 0;
    current_track = 0;
    playing = false;
    exporting = false;
    editing = false;
    modifier = false;
    shift_down = false;
    selection_enabled = false;
    follow = false;
    visualiser = false;
    credits = false;
    help = false;
    help_index = 0;
    octave = 2;
    tempo_selection_x = 0;
    tempo_selection_y = 0;
    instrument_editor_effects_x = 0;
    instrument_editor_effects_y = 0;
    pattern_cursor_x = 0;
    pattern_cursor_y = 0;
    visual_pattern_offset = 0;
    visual_track_width = 30;
    visual_track_height = 16;
    visual_cursor_x = 0;
    visual_cursor_y = 0;
    selection_x = 0;
    selection_y = 0;
    last_selection_x = 0;
    last_selection_y = 0;
    last_selection_w = 0;
    last_selection_h = 0;
    last_copied_pattern_x = 0;
    last_copied_pattern_y = 0;
}

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
    f->file_name_limit = 40;
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

static void handle_key_down_file(void) {
   
    char c = 0;
    
    if(input->key_0) {
        c = '0';
        add_filename_char(c);
    } else if(input->key_1) {
        c = '1';
        add_filename_char(c);
    } else if(input->key_2) {
        c = '2';
        add_filename_char(c);
    } else if(input->key_3) {
        c = '3';
        add_filename_char(c);
    } else if(input->key_4) {
        c = '4';
        add_filename_char(c);
    } else if(input->key_5) {
        c = '5';
        add_filename_char(c);
    } else if(input->key_6) {
        c = '6';
        add_filename_char(c);
    } else if(input->key_7) {
        c = '7';
        add_filename_char(c);
    } else if(input->key_8) {
        c = '8';
        add_filename_char(c);
    } else if(input->key_9) {
        c = '9';
        add_filename_char(c);
    } else if(input->key_a) {
        c = 'a';
        add_filename_char(c);
    } else if(input->key_b) {
        c = 'b';
        add_filename_char(c);
    } else if(input->key_c) {
        c = 'c';
        add_filename_char(c);
    } else if(input->key_d) {
        c = 'd';
        add_filename_char(c);
    } else if(input->key_e) {
        c = 'e';
        add_filename_char(c);
    } else if(input->key_f) {
        c = 'f';
        add_filename_char(c);
    } else if(input->key_g) {
        c = 'g';
        add_filename_char(c);
    } else if(input->key_h) {
        c = 'h';
        add_filename_char(c);
    } else if(input->key_i) {
        c = 'i';
        add_filename_char(c);
    } else if(input->key_j) {
        c = 'j';
        add_filename_char(c);
    } else if(input->key_k) {
        c = 'k';
        add_filename_char(c);
    } else if(input->key_l) {
        c = 'l';
        add_filename_char(c);
    } else if(input->key_m) {
        c = 'm';
        add_filename_char(c);
    } else if(input->key_n) {
        c = 'n';
        add_filename_char(c);
    } else if(input->key_o) {
        c = 'o';
        add_filename_char(c);
    } else if(input->key_p) {
        c = 'p';
        add_filename_char(c);
    } else if(input->key_q) {
        c = 'q';
        add_filename_char(c);
    } else if(input->key_r) {
        c = 'r';
        add_filename_char(c);
    } else if(input->key_s) {
        c = 's';
        add_filename_char(c);
    } else if(input->key_t) {
        c = 't';
        add_filename_char(c);
    } else if(input->key_u) {
        c = 'u';
        add_filename_char(c);
    } else if(input->key_v) {
        c = 'v';
        add_filename_char(c);
    } else if(input->key_w) {
        c = 'w';
        add_filename_char(c);
    } else if(input->key_x) {
        c = 'x';
        add_filename_char(c);
    } else if(input->key_y) {
        c = 'y';
        add_filename_char(c);
    } else if(input->key_z) {
        c = 'z';
        add_filename_char(c);
    } else if(input->key_escape) {
        exit_file_editor();
    } else if(input->key_return) {
        if(export_project) {
            if(file_settings->file_name != NULL) {
                char *file_path = cAllocatorAlloc(sizeof(char)*file_settings->file_name_max_length, "load_path chars");
                if(conf_default_dir != NULL) {
                    snprintf(file_path, file_settings->file_name_max_length-1, "%s%s.wav", conf_default_dir, file_settings->file_name);
                } else {
                    snprintf(file_path, file_settings->file_name_max_length-1, "%s.wav", file_settings->file_name);
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
                    snprintf(file_path, file_settings->file_name_max_length-1, "%s%s.snibb", conf_default_dir, file_settings->file_name);
                } else {
                    snprintf(file_path, file_settings->file_name_max_length-1, "%s.snibb", file_settings->file_name);
                }
                if(file_editor_confirm_action) {
                    bool was_playing = playing;
                    if(was_playing) {
                        toggle_playback();
                    }
                    load_project_file(file_path);
                    visual_track_height = synth->track_height;
                    file_editor_confirm_action = false;
                    if(was_playing) {
                        toggle_playback();
                    }
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
    } else if(input->key_up) {
        file_settings->file_cursor_y--;
        if(file_settings->file_cursor_y < 0) {
            file_settings->file_cursor_y = 0;
        }
        set_list_file_name();
    } else if(input->key_down) {
        if(file_settings->file_cursor_y < file_settings->file_dir_max_length-1) {
            if(file_settings->file_dirs[file_settings->file_cursor_y+1] != NULL) {
                file_settings->file_cursor_y++;
            }
        }
        set_list_file_name();
    } else if(input->key_backspace) {
        remove_filename_char();
    } else if(input->key_escape) {
        exit_file_editor();
    }
}


static void set_list_file_name(void) {
    
    // must remove file ending..
    char *list_name = file_settings->file_dirs[file_settings->file_cursor_y];
	if(list_name != NULL) {
		int length = (int)strlen(list_name);
        file_settings->file_name = cAllocatorFree(file_settings->file_name);
        char *temp_chars = cAllocatorAlloc(sizeof(char)*file_settings->file_name_max_length, "file name chars");
        snprintf(temp_chars, file_settings->file_name_max_length-1, "%s", list_name);
        file_settings->file_name = temp_chars;
        
        // a list_name cannot be added unless it has the correct file endings, so we can assume
        // it will be of at least a certain size.
        if(file_settings->file_name[length-2] == 'a') {
            // wav file
            file_settings->file_name[length-4] = '\0';
            // it will be of at least a certain size.
        } else if(file_settings->file_name[length-2] == 'b') {
            // snibb file
            file_settings->file_name[length-6] = '\0';
        }
        else {
            // json file
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
	
    int offset_y = 0;
    for (int i = 0; i < file_settings->file_dir_max_length; i++) {
        if (file_settings->file_dirs[i] != NULL) {
            if(i == file_settings->file_cursor_y) {
                cEngineRenderLabelWithParams(raster2d, file_settings->file_dirs[i], 0, offset_y-file_settings->file_cursor_y+10, color_text, cengine_color_transparent);
            } else {
                cEngineRenderLabelWithParams(raster2d, file_settings->file_dirs[i], 0, offset_y-file_settings->file_cursor_y+10, color_inactive_text, cengine_color_transparent);
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
        snprintf(temp_chars, file_settings->file_name_max_length-1, "%s%c", file_settings->file_name, c);
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
        snprintf(temp_chars, file_settings->file_name_max_length-1, "%s", file_settings->file_name);
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
            if(errorlog) { printf("file not found at path %s\n", path); }
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
            snprintf(save_path, file_settings->file_name_max_length-1, "%s%s.snibb", conf_default_dir, file_settings->file_name);
        } else {
            snprintf(save_path, file_settings->file_name_max_length-1, "%s.snibb", file_settings->file_name);
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
            snprintf(info, max_size-1, "%s", string);
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
            snprintf(info, max_size-1, "%s:%d", string, data);
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

static void update_info(double dt) {
    
    if(info_string != NULL) {
        if(!cTimerIsReady(info_timer)) {
            cTimerAdvance(dt, info_timer);
            if(cTimerIsReady(info_timer)) {
                redraw_screen = true;
            }
        }
    }
}

static void render_info(void) {
    
    if(info_string != NULL) {
        if(!cTimerIsReady(info_timer)) {
            cEngineRenderLabelWithParams(raster2d, info_string, 0, 23, color_text, color_info_text_bg);
        }
    }
}

static void init_data(void) {
    
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
                raster2d[r_x][r_y] = color_bg;
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
    
    visualiser2d = cAllocatorAlloc(s_width * sizeof(unsigned int *), "main.c visualiser2d 2");
    if(visualiser2d == NULL) {
        fprintf(stderr, "out of memory\n");
    } else {
        for(i = 0; i < s_width; i++) {
            visualiser2d[i] = cAllocatorAlloc(s_height * sizeof(unsigned int), "main.c visualiser2d 3");
            if(visualiser2d[i] == NULL)
            {
                fprintf(stderr, "out of memory\n");
            }
        }
    }
    
    for(r_x = 0; r_x < s_width; r_x++) {
        for(r_y = 0; r_y < s_height; r_y++) {
            if(visualiser2d != NULL && visualiser2d[r_x] != NULL) {
                visualiser2d[r_x][r_y] = 0;
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
    
    for(i = 0; i < s_width; i++) {
        visualiser2d[i] = cAllocatorFree(visualiser2d[i]);
    }
    visualiser2d = cAllocatorFree(visualiser2d);
    
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

static void transpose_selection(int track, int cursor_x, int cursor_y, int selection_x, int selection_y, bool up, int amount) {
    
    cSynthTransposeSelection(synth, track, cursor_x, cursor_y, selection_x, selection_y, up, amount);
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
        if(debuglog) { printf("paste pattern to x:%d y:%d", last_copied_pattern_x, last_copied_pattern_y); }
    }
}

static void add_track_node_with_octave(int x, int y, bool editing, int value) {
    
    int x_count = visual_cursor_x%5;
    int instrument = synth->current_instrument;
    
    //only effects are allowed to go over F so limit it.
    
    if(instrument_editor || pattern_editor || visualiser || !editing) {
        if(instrument_editor) {
            instrument = selected_instrument_id;
        }
        cSynthAddTrackNode(synth, instrument, current_track, x, y, false, true, value+(octave*12), playing);
    } else {
        
        if(!editing) {
            cSynthAddTrackNode(synth, instrument, current_track, x, y, false, true, value+(octave*12), playing);
        } else {
            
            bool move_down = false;
            if(x_count == 0) {
                cSynthAddTrackNode(synth, instrument, current_track, x, y, editing, true, value+(octave*12), playing);
                if(editing) {
                    move_down = true;
                }
            }
            
            // only effects are allowed to go over F.
            if(x_count != 0 && x_count != 2 && value > 15) {
                printf("only effects are allowed to go over F.\n");
                return;
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
                        set_visual_cursor(0, step_size, true);
                    }
                } else {
                    set_visual_cursor(0, step_size, true);
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
        
        if(visual_cursor_y >= visual_track_height) {
            current_track = cSynthGetNextActiveTrack(current_track, synth, true);
            visual_cursor_y = visual_cursor_y - visual_track_height;
        }
        
        if(visual_cursor_y < 0) {
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
        clear_clipping_color_from_visualiser();
        set_info_timer("playback started");
    } else {
        // note off to all voices when stopping playing.
        for(int v_i = 0; v_i < synth->max_voices; v_i++) {
            synth->voices[v_i]->note_on = false;
        }
        cSynthResetOnLoopBack(synth);
        playing = false;
        synth->bitcrush_active = false;
        set_info_timer("playback stopped");
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

static void change_octave(bool up) {
    
    if(up) {
        octave++;
        if(octave > 7) {
            octave = 7;
        }
        set_info_timer_with_int("octave", octave);
    } else {
        octave--;
        if(octave < 0) {
            octave = 0;
        }
        set_info_timer_with_int("octave", octave);
    }
}

void handle_key_up(SDL_Keysym* keysym) {
    
    sdl_key_mapping(keysym, false);
    sdl_key_set_locks(keysym, false);
    
    if(last_key == keysym->sym) {
        any_key_pressed = false;
    }
    
    #if defined(platform_osx)
        if(!input->key_lock_lgui) {
            modifier = false;
        }
    #elif defined(platform_windows)
        if(!input->key_lock_lctrl) {
            modifier = false;
        }
    #endif
    
    if(!input->key_lock_left) {
        pressed_left = false;
    }
    if(!input->key_lock_right) {
        pressed_right = false;
    }
    if(!input->key_lock_up) {
        pressed_up = false;
    }
    if(!input->key_lock_down) {
        pressed_down = false;
    }
    if (!input->key_lock_lshift) {
        shift_down = false;
    }
    
    redraw_screen = true;
}

static void move_patterns(int direction) {
    int pattern_y = visual_pattern_offset+pattern_cursor_y;
    for(int x = 0; x < synth->patterns_width; x++) {
        int patterns[synth->patterns_height];
        for(int i = 0; i < synth->patterns_height; i++) {
            patterns[i] = synth->patterns[x][i];
        }
        if(direction == 1) {
            for(int i = 1; i < synth->patterns_height; i++) {
                if(i >= pattern_y-1) {
                    synth->patterns[x][i] = patterns[i-1];
                }
            }
            if(pattern_y-2 < synth->patterns_height) {
                synth->patterns[x][pattern_y-2] = 0;
            }
        } else {
            for(int i = 0; i < synth->patterns_height-1; i++) {
                if(i >= pattern_y-1) {
                    synth->patterns[x][i] = patterns[i+1];
                }
            }
            synth->patterns[x][synth->patterns_height-1] = 0;
        }
    }
}

void handle_key_down(SDL_Keysym* keysym) {
    
    sdl_key_mapping(keysym, true);
    sdl_key_set_locks(keysym, true);
    
    if(last_key != keysym->sym) {
        synth->preview_locked = false;
        synth->preview_started = false;
    }
    last_key = keysym->sym;
    
    any_key_pressed = true;
    redraw_screen = true;
    
    if(!file_editor) {
        if(input->key_f1) {
            set_view(0);
        }
        if(input->key_f2) {
            set_view(1);
        }
        if(input->key_f3) {
            set_view(2);
        }
        if(input->key_f4) {
            set_view(3);
        }
        if(input->key_f5) {
            set_view(4);
        }
        if(input->key_f6) {
            set_view(5);
        }
        if(input->key_f7) {
            set_view(6);
        }
        if(input->key_f8) {
            set_view(7);
        }
        if(input->key_f9) {
            set_view(8);
        }
    }
    
    if(help) {
        handle_help_keys();
        return;
    }
    
    if(credits) {
        handle_credits_keys();
        return;
    }
    
    if(reset_confirmation) {
        handle_reset_confirmation_keys();
    }
    
    if(file_editor) {
        handle_key_down_file();
        return;
    } else {

        
        if(input->key_i) {
            if(modifier) {
                if (instrument_editor) {
                    instrument_editor = false;
                } else {
                    tempo_editor = false;
                    visualiser = false;
                    wavetable_editor = false;
                    custom_table = false;
                    instrument_editor = true;
                }
                return;
            }
        }
        if(input->key_j) {
            if(modifier) {
                if (custom_table) {
                    custom_table = false;
                } else {
                    tempo_editor = false;
                    visualiser = false;
                    wavetable_editor = false;
                    instrument_editor = false;
                    custom_table = true;
                }
                return;
            }
        }
        if(input->key_t) {
            if(modifier) {
                if (tempo_editor) {
                    tempo_editor = false;
                } else {
                    instrument_editor = false;
                    visualiser = false;
                    wavetable_editor = false;
                    custom_table = false;
                    tempo_editor = true;
                }
                return;
            }
        }
        if(input->key_r) {
            if(modifier) {
                if (wavetable_editor) {
                    wavetable_editor = false;
                } else {
                    instrument_editor = false;
                    visualiser = false;
                    tempo_editor = false;
                    custom_table = false;
                    wavetable_editor = true;
                }
                return;
            }
        }
        if(input->key_q) {
            if(modifier) {
                // user will quit app on OSX.
                return;
            }
        }
        if(input->key_n) {
            if(modifier) {
                reset_confirmation = true;
                return;
            }
        }
        if(input->key_home) {
            if(tempo_editor) {
                tempo_selection_y = 0;
            } else if(instrument_editor) {
                int ins_id = selected_instrument_id-1;
                if(ins_id < 0) {
                    ins_id = synth->max_instruments-1;
                }
                selected_instrument_id = ins_id;
            } else if(pattern_editor && !instrument_editor && !visualiser && !tempo_editor && !wavetable_editor) {
                pattern_cursor_y = 0;
            } else {
                visual_cursor_y = 0;
            }
        }
        if(input->key_end) {
            if(tempo_editor) {
                tempo_selection_y = synth->tempo_height-1;
            } else if(instrument_editor) {
                int ins_id = selected_instrument_id+1;
                if(ins_id >= synth->max_instruments) {
                    ins_id = 0;
                }
                selected_instrument_id = ins_id;
            } else if(pattern_editor && !instrument_editor && !visualiser && !tempo_editor && !wavetable_editor) {
                pattern_cursor_y = synth->patterns_and_voices_height-1;
            }  else {
                visual_cursor_y = synth->track_height-1;
            }
        }
        if(input->key_m) {
            if(pattern_editor && !instrument_editor && !visualiser && !tempo_editor && !wavetable_editor) {
                if(pattern_cursor_y == 0) {
                    if(synth->voices[pattern_cursor_x]->muted == 1) {
                        synth->voices[pattern_cursor_x]->muted = 0;
                    } else {
                        synth->voices[pattern_cursor_x]->muted = 1;
                    }
                }
            }
        }
        if(input->key_plus) {
            if(instrument_editor) {
                change_octave(true);
            } else if(visualiser) {
                change_octave(true);
            } else if(pattern_editor) {
                change_param(true);
            } else if(tempo_editor) {
          
            } else if(wavetable_editor) {

            } else {
                if (editing && modifier) {
                    transpose_selection(current_track, visual_cursor_x, visual_cursor_y, selection_x, selection_y, true, 12);
                    set_info_timer("transpose octave up");
                } else if(editing) {
                    transpose_selection(current_track, visual_cursor_x, visual_cursor_y, selection_x, selection_y, true, 1);
                    set_info_timer("transpose halfnote up");
                }
            }
        }
        if(input->key_minus) {
            
            if(instrument_editor) {
                change_octave(false);
            } else if(visualiser) {
                change_octave(false);
            } else if(pattern_editor) {
                change_param(false);
            } else if(tempo_editor) {
                
            } else if(wavetable_editor) {
                
            } else {
                if (editing && modifier) {
                    transpose_selection(current_track, visual_cursor_x, visual_cursor_y, selection_x, selection_y, false, 12);
                    set_info_timer("transpose octave down");
                } else if(editing) {
                    transpose_selection(current_track, visual_cursor_x, visual_cursor_y, selection_x, selection_y, false, 1);
                    set_info_timer("transpose halfnote down");
                }
            }
        }
        if(input->key_c) {
            if(modifier) {
                if(instrument_editor) {}
                else if(file_editor) {}
                else if(pattern_editor && !instrument_editor && !visualiser && !tempo_editor && !wavetable_editor) {
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
                        set_info_timer("copy notes");
                    }
                    return;
                }
            }
        }
        if(input->key_x) {
            if(modifier) {
                if(instrument_editor) {}
                else if(file_editor) {}
                else if(pattern_editor) {
                    // cut for pattern editor is not supported.
                } else if(!instrument_editor && !visualiser && !tempo_editor && !wavetable_editor){
                    if(editing) {
                        copy_notes(current_track, visual_cursor_x, visual_cursor_y, selection_x, selection_y, true, true);
                        set_info_timer("cut notes");
                    }
                    return;
                }
            }
        }
        if(input->key_v) {
            if(modifier) {
                if(instrument_editor) {}
                else if(file_editor) {}
                else if(pattern_editor && !instrument_editor && !visualiser && !tempo_editor && !wavetable_editor) {
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
                        set_info_timer("paste notes");
                    }
                    return;
                }
            }
        }
        if(input->key_x) {
            if(pattern_editor && !instrument_editor && !visualiser && !tempo_editor && !wavetable_editor) {
                if(pattern_cursor_y > 0 && pattern_cursor_y < 17) {
                    if(synth->active_tracks[pattern_cursor_y-1+visual_pattern_offset] == 0) {
                        synth->active_tracks[pattern_cursor_y-1+visual_pattern_offset] = 1;
                    } else {
                        synth->active_tracks[pattern_cursor_y-1+visual_pattern_offset] = 0;
                    }
                }
            }
        }
        if(input->key_p) {
            if(modifier) {
                if(visualiser) {
                    visualiser = false;
                } else {
                    visualiser = true;
                    tempo_editor = false;
                    instrument_editor = false;
                }
                return;
            }
        }
        if(input->key_o) {
            if(modifier) {
                file_editor = true;
                return;
            }
        }
        if(input->key_e) {
            if(modifier) {
                file_editor = true;
                file_settings->file_editor_save = true;
                export_project = true;
                return;
            }
            else if(pattern_editor && !instrument_editor && !visualiser && !tempo_editor && !wavetable_editor) {
                if(pattern_cursor_y > 0 && pattern_cursor_y < 17) {
                    pattern_editor = false;
                    current_track = pattern_cursor_y-1+visual_pattern_offset;
                    visual_cursor_x = pattern_cursor_x*5;
                    set_info_timer("jump to track");
                }
                return;
            }
        }
        if(input->key_s) {
            if(modifier) {
                file_editor = true;
                file_settings->file_editor_save = true;
                export_project = false;
                return;
            } else {
                if(pattern_editor && !instrument_editor && !visualiser && !tempo_editor && !wavetable_editor) {
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
        }
        if(input->key_f) {
            if(modifier && !instrument_editor && !visualiser && !tempo_editor && !wavetable_editor) {
                if(follow) {
                    follow = false;
                    set_info_timer("follow: false");
                } else {
                    follow = true;
                    set_info_timer("follow: true");
                }
                return;
            }
        }
        if(input->key_tab) {
            if(instrument_editor) {
                struct CInstrument *ins = synth->instruments[selected_instrument_id];
                selected_instrument_node_index++;
                if(selected_instrument_node_index >= ins->adsr_nodes) {
                    selected_instrument_node_index = 1;
                }
            } else if(custom_table) {
                struct CInstrument *ins = synth->custom_instrument;
                selected_custom_table_node_index++;
                if(selected_custom_table_node_index >= ins->adsr_nodes) {
                    selected_custom_table_node_index = 0;
                }
            } else if(!instrument_editor && !visualiser && !tempo_editor && !wavetable_editor){
                if(pattern_editor) {
                    pattern_editor = false;
                } else {
                    pattern_editor = true;
                }
            }
        }
        #if defined(platform_osx)
            if(input->key_lgui) {
                modifier = true;
            }
        #elif defined(platform_windows)
            if(input->key_lctrl) {
                modifier = true;
            }
        #endif
        if(input->key_escape) {
            if(instrument_editor) {
                instrument_editor = false;
            } else if(visualiser) {
                visualiser = false;
            } else if(tempo_editor) {
                tempo_editor = false;
            } else if(wavetable_editor) {
                wavetable_editor = false;
            } else if(custom_table) {
                custom_table = false;
            } else if(help) {
                help = false;
            } else if(credits) {
                credits = false;
            } else if(pattern_editor) {
                pattern_editor = false;
            }
        }
        if(input->key_space) {
            toggle_playback();
        }
        if(input->key_left) {
            pressed_left = true;
            
            if(tempo_editor) {
                tempo_selection_x--;
                if(tempo_selection_x < 0) {
                    tempo_selection_x = synth->tempo_width-1;
                }
            } else if(custom_table) {
                
            } else if(wavetable_editor) {
                int inc = 1;
                if(wavetable_selection_y < 2) {
                    inc = 2;
                }
                wavetable_selection_x -= inc;
                if(wavetable_selection_x < 0) {
                    wavetable_selection_x = synth->wavetable_width-inc;
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
                    change_octave(false);
                } else if(pattern_editor) {
                    pattern_cursor_x -= 1;
                    check_pattern_cursor_bounds();
                } else {
                    set_visual_cursor(-1, 0, true);
                }
            }
        }
        if(input->key_right) {
            pressed_right = true;
            
            if(tempo_editor) {
                tempo_selection_x++;
                if(tempo_selection_x >= synth->tempo_width) {
                    tempo_selection_x = 0;
                }
            } else if(custom_table) {
                
            } else if(wavetable_editor) {
                int inc = 1;
                if(wavetable_selection_y < 2) {
                    inc = 2;
                }
                wavetable_selection_x += inc;
                if(wavetable_selection_x >= synth->wavetable_width) {
                    wavetable_selection_x = 0;
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
                    change_octave(true);
                } else if(pattern_editor) {
                    pattern_cursor_x += 1;
                    check_pattern_cursor_bounds();
                } else {
                    set_visual_cursor(1, 0, true);
                }
            }
        }
        if(input->key_up) {
            pressed_up = true;
            if(tempo_editor) {
                tempo_selection_y--;
                if(tempo_selection_y < 0) {
                    tempo_selection_y = synth->tempo_height-1;
                }
            } else if(custom_table) {
                
            } else if(wavetable_editor) {
                wavetable_selection_y--;
                if(wavetable_selection_y < 0) {
                    wavetable_selection_y = synth->wavetable_height-1;
                }
                if(wavetable_selection_y == 1){
                    if(wavetable_selection_x == 1 ||
                       wavetable_selection_x == 3 ||
                       wavetable_selection_x == 5 ||
                       wavetable_selection_x == 7 ||
                       wavetable_selection_x == 9 ||
                       wavetable_selection_x == 11)
                        wavetable_selection_x--;
                }
            } else if(instrument_editor) {
                if(instrument_editor_effects) {
                    instrument_editor_effects_y--;
                    if(instrument_editor_effects_y < 0) {
                        instrument_editor_effects_y = visual_instrument_effects-1;
                    }
                }
            } else if(shift_down && pattern_editor) {
                visual_pattern_offset -= 16;
                if(visual_pattern_offset < 0){
                    visual_pattern_offset = 48;
                }
            } else if(modifier && pattern_editor) {
                pattern_cursor_y -= 1;
                check_pattern_cursor_bounds();
                move_patterns(-1);
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
        }
        if(input->key_down) {
            pressed_down = true;
            if(tempo_editor) {
                tempo_selection_y++;
                if(tempo_selection_y >= synth->tempo_height) {
                    tempo_selection_y = 0;
                }
            } else if(custom_table) {
                
            } else if(wavetable_editor) {
                wavetable_selection_y++;
                if(wavetable_selection_y >= synth->wavetable_height) {
                    wavetable_selection_y = 0;
                }
            } else if(instrument_editor) {
                if(instrument_editor_effects) {
                    instrument_editor_effects_y++;
                    if(instrument_editor_effects_y >= visual_instrument_effects) {
                        instrument_editor_effects_y = 0;
                    }
                }
            } else if(shift_down && pattern_editor) {
                visual_pattern_offset += 16;
                if(visual_pattern_offset > 48){
                    visual_pattern_offset = 0;
                }
            } else if(modifier && pattern_editor) {
                pattern_cursor_y += 1;
                check_pattern_cursor_bounds();
                move_patterns(1);
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
        }
        if(input->key_backspace || input->key_delete) {
            if(instrument_editor) {
                if(instrument_editor_effects) {
                    instrument_effect_remove();
                }
            }
            else if(pattern_editor) {}
            else if(editing && !instrument_editor && !visualiser && !tempo_editor && !wavetable_editor) {
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
                        cSynthRemoveTrackNodeParams(synth, current_track, synth->track_cursor_x, synth->track_cursor_y, false, false, true, false);
                        cSynthRemoveTrackNodeParams(synth, current_track, synth->track_cursor_x, synth->track_cursor_y, false, false, false, true);
                    } else if(x_count == 3) {
                        cSynthRemoveTrackNodeParams(synth, current_track, synth->track_cursor_x, synth->track_cursor_y, false, false, true, false);
                    } else if(x_count == 4) {
                        cSynthRemoveTrackNodeParams(synth, current_track, synth->track_cursor_x, synth->track_cursor_y, false, false, false, true);
                    }
                    
                    if(follow) {
                        if(playing) {}
                        else {
                            set_visual_cursor(0, step_size, true);
                        }
                    } else {
                        set_visual_cursor(0, step_size, true);
                    }
                }
            }
        }
        if(input->key_return) {
            if(tempo_editor) {
                if(modifier) {
                    if (playing) {
                        synth->pending_tempo_column = tempo_selection_x;
                    } else {
                        synth->current_tempo_column = tempo_selection_x;
                    }
                } else {
                    
                }
            } else if(visualiser) {
                visualiser = false;
            } else if(wavetable_editor) {
                
            } else if(instrument_editor) {
                instrument_editor = false;
            } else {
                if(pattern_editor && !instrument_editor && !visualiser && !tempo_editor && !wavetable_editor) {
                    
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
                    } else if(pattern_cursor_y == 20 && pattern_cursor_x == 0) {
                        wavetable_editor = true;
                        return;
                    } else if(pattern_cursor_y == 21 && pattern_cursor_x == 2) {
                        custom_table = true;
                    } else if(pattern_cursor_y == 20 && pattern_cursor_x == 4) {
                        tempo_editor = true;
                    } else if(pattern_cursor_y == 20 && pattern_cursor_x == 5) {
                        visualiser = true;
                    } else if(pattern_cursor_y == 21 && pattern_cursor_x == 5) {
                        credits = true;
                    } else if(pattern_cursor_y == 21 && pattern_cursor_x == 1) {
                        help = true;
                    }
                } else {
                    if(!input->key_lock_return) {
                        toggle_editing();
                    }
                }
            }
        }
        if(input->key_lshift) {
            if(instrument_editor) {
                if(instrument_editor_effects) {
                    instrument_editor_effects = false;
                } else {
                    instrument_editor_effects = true;
                }
            } else {
                shift_down = true;
            }
        }
    }
    
    if(shift_down && editing) {
        selection_enabled = true;
    }
    
    if(!pattern_editor && !tempo_editor && !wavetable_editor && !instrument_editor && !visualiser && modifier) {
        bool stepsize_set = false;
        if(input->key_1) {
            step_size = 1;
            stepsize_set = true;
        } else if(input->key_2) {
            step_size = 2;
            stepsize_set = true;
        } else if(input->key_3) {
            step_size = 3;
            stepsize_set = true;
        } else if(input->key_4) {
            step_size = 4;
            stepsize_set = true;
        } else if(input->key_5) {
            step_size = 5;
            stepsize_set = true;
        } else if(input->key_6) {
            step_size = 6;
            stepsize_set = true;
        } else if(input->key_7) {
            step_size = 7;
            stepsize_set = true;
        } else if(input->key_8) {
            step_size = 8;
            stepsize_set = true;
        } else if(input->key_9) {
            step_size = 9;
            stepsize_set = true;
        }
        if(stepsize_set) {
            set_info_timer_with_int("stepsize", step_size);
        }
        return;
    }
    
    int x_count = visual_cursor_x%5;
    
    if(tempo_editor) {
        handle_tempo_keys();
        return;
    } else if(wavetable_editor) {
        handle_wavetable_keys();
        return;
    } else if(instrument_editor && instrument_editor_effects) {
        handle_instrument_effect_keys();
        return;
    } else if(pattern_editor && !instrument_editor && !visualiser) {
        handle_pattern_keys();
        return;
    } else if(x_count == 1 && editing && !instrument_editor && !visualiser) {
        handle_param_value();
        return;
    } else if((x_count > 1 && editing) && (x_count < 5 && editing) && !instrument_editor && !visualiser) {
        handle_param_value();
        return;
    } else {
        handle_note_keys();
    }
}

static void set_view(int view) {
    
    instrument_editor = false;
    visualiser = false;
    tempo_editor = false;
    wavetable_editor = false;
    custom_table = false;
    help = false;
    credits = false;
    pattern_editor = false;
    
    if (view == 1) {
        pattern_editor = true;
    } else if (view == 2) {
        instrument_editor = true;
    } else if (view == 3) {
        tempo_editor = true;
    } else if (view == 4) {
        wavetable_editor = true;
    } else if (view == 5) {
        custom_table = true;
    } else if (view == 6) {
        visualiser = true;
    } else if (view == 7) {
        help = true;
    } else if (view == 8) {
        credits = true;
    }
    
}

static void sdl_key_set_locks(SDL_Keysym* keysym, bool down) {
    
    switch(keysym->sym) {
        case SDLK_0: input->key_pending_lock_0 = down; break;
        case SDLK_1: input->key_pending_lock_1 = down; break;
        case SDLK_2: input->key_pending_lock_2 = down; break;
        case SDLK_3: input->key_pending_lock_3 = down; break;
        case SDLK_4: input->key_pending_lock_4 = down; break;
        case SDLK_5: input->key_pending_lock_5 = down; break;
        case SDLK_6: input->key_pending_lock_6 = down; break;
        case SDLK_7: input->key_pending_lock_7 = down; break;
        case SDLK_8: input->key_pending_lock_8 = down; break;
        case SDLK_9: input->key_pending_lock_9 = down; break;
            
        case SDLK_a: input->key_pending_lock_a = down; break;
        case SDLK_b: input->key_pending_lock_b = down; break;
        case SDLK_c: input->key_pending_lock_c = down; break;
        case SDLK_d: input->key_pending_lock_d = down; break;
        case SDLK_e: input->key_pending_lock_e = down; break;
        case SDLK_f: input->key_pending_lock_f = down; break;
        case SDLK_g: input->key_pending_lock_g = down; break;
        case SDLK_h: input->key_pending_lock_h = down; break;
        case SDLK_i: input->key_pending_lock_i = down; break;
        case SDLK_j: input->key_pending_lock_j = down; break;
        case SDLK_k: input->key_pending_lock_k = down; break;
        case SDLK_l: input->key_pending_lock_l = down; break;
        case SDLK_m: input->key_pending_lock_m = down; break;
        case SDLK_n: input->key_pending_lock_n = down; break;
        case SDLK_o: input->key_pending_lock_o = down; break;
        case SDLK_p: input->key_pending_lock_p = down; break;
        case SDLK_q: input->key_pending_lock_q = down; break;
        case SDLK_r: input->key_pending_lock_r = down; break;
        case SDLK_s: input->key_pending_lock_s = down; break;
        case SDLK_t: input->key_pending_lock_t = down; break;
        case SDLK_u: input->key_pending_lock_u = down; break;
        case SDLK_v: input->key_pending_lock_v = down; break;
        case SDLK_w: input->key_pending_lock_w = down; break;
        case SDLK_x: input->key_pending_lock_x = down; break;
        case SDLK_y: input->key_pending_lock_y = down; break;
        case SDLK_z: input->key_pending_lock_z = down; break;
            
        case SDLK_SPACE: input->key_pending_lock_space = down; break;
        case SDLK_PLUS:
        case SDLK_KP_PLUS:
            input->key_pending_lock_plus = down;
            break;
        case SDLK_MINUS:
        case SDLK_KP_MINUS:
            input->key_pending_lock_minus = down;
            break;
        case SDLK_TAB: input->key_pending_lock_tab = down; break;
        case SDLK_LGUI: input->key_pending_lock_lgui = down; break;
        case SDLK_LCTRL: input->key_pending_lock_lctrl = down; break;
        case SDLK_ESCAPE: input->key_pending_lock_escape = down; break;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            input->key_pending_lock_return = down;
            break;
        case SDLK_LEFT: input->key_pending_lock_left = down; break;
        case SDLK_RIGHT: input->key_pending_lock_right = down; break;
        case SDLK_UP: input->key_pending_lock_up = down; break;
        case SDLK_DOWN: input->key_pending_lock_down = down; break;
        case SDLK_LSHIFT: input->key_pending_lock_lshift = down; break;
        case SDLK_HOME: input->key_pending_lock_home = down; break;
        case SDLK_END: input->key_pending_lock_end = down; break;
        case SDLK_BACKSPACE: input->key_pending_lock_backspace = down; break;
        case SDLK_DELETE: input->key_pending_lock_delete = down; break;
        case SDLK_COMMA: input->key_pending_lock_comma = down; break;
        case SDLK_PERIOD: input->key_pending_lock_period = down; break;
        case SDLK_F1: input->key_pending_lock_f1 = down; break;
        case SDLK_F2: input->key_pending_lock_f2 = down; break;
        case SDLK_F3: input->key_pending_lock_f3 = down; break;
        case SDLK_F4: input->key_pending_lock_f4 = down; break;
        case SDLK_F5: input->key_pending_lock_f5 = down; break;
        case SDLK_F6: input->key_pending_lock_f6 = down; break;
        case SDLK_F7: input->key_pending_lock_f7 = down; break;
        case SDLK_F8: input->key_pending_lock_f8 = down; break;
        case SDLK_F9: input->key_pending_lock_f9 = down; break;
        case SDLK_F10: input->key_pending_lock_f10 = down; break;
        case SDLK_F11: input->key_pending_lock_f11 = down; break;
        case SDLK_F12: input->key_pending_lock_f12 = down; break;
    }
    
    if(!down) {
        switch(keysym->sym) {
            case SDLK_0: input->key_lock_0 = down; break;
            case SDLK_1: input->key_lock_1 = down; break;
            case SDLK_2: input->key_lock_2 = down; break;
            case SDLK_3: input->key_lock_3 = down; break;
            case SDLK_4: input->key_lock_4 = down; break;
            case SDLK_5: input->key_lock_5 = down; break;
            case SDLK_6: input->key_lock_6 = down; break;
            case SDLK_7: input->key_lock_7 = down; break;
            case SDLK_8: input->key_lock_8 = down; break;
            case SDLK_9: input->key_lock_9 = down; break;
                
            case SDLK_a: input->key_lock_a = down; break;
            case SDLK_b: input->key_lock_b = down; break;
            case SDLK_c: input->key_lock_c = down; break;
            case SDLK_d: input->key_lock_d = down; break;
            case SDLK_e: input->key_lock_e = down; break;
            case SDLK_f: input->key_lock_f = down; break;
            case SDLK_g: input->key_lock_g = down; break;
            case SDLK_h: input->key_lock_h = down; break;
            case SDLK_i: input->key_lock_i = down; break;
            case SDLK_j: input->key_lock_j = down; break;
            case SDLK_k: input->key_lock_k = down; break;
            case SDLK_l: input->key_lock_l = down; break;
            case SDLK_m: input->key_lock_m = down; break;
            case SDLK_n: input->key_lock_n = down; break;
            case SDLK_o: input->key_lock_o = down; break;
            case SDLK_p: input->key_lock_p = down; break;
            case SDLK_q: input->key_lock_q = down; break;
            case SDLK_r: input->key_lock_r = down; break;
            case SDLK_s: input->key_lock_s = down; break;
            case SDLK_t: input->key_lock_t = down; break;
            case SDLK_u: input->key_lock_u = down; break;
            case SDLK_v: input->key_lock_v = down; break;
            case SDLK_w: input->key_lock_w = down; break;
            case SDLK_x: input->key_lock_x = down; break;
            case SDLK_y: input->key_lock_y = down; break;
            case SDLK_z: input->key_lock_z = down; break;
                
            case SDLK_SPACE: input->key_lock_space = down; break;
            case SDLK_PLUS:
            case SDLK_KP_PLUS:
                input->key_pending_lock_plus = down;
                break;
            case SDLK_MINUS:
            case SDLK_KP_MINUS:
                input->key_pending_lock_minus = down;
                break;
            case SDLK_TAB: input->key_lock_tab = down; break;
            case SDLK_LGUI: input->key_lock_lgui = down; break;
            case SDLK_LCTRL: input->key_lock_lctrl = down; break;
            case SDLK_ESCAPE: input->key_lock_escape = down; break;
            case SDLK_RETURN:
            case SDLK_KP_ENTER:
                input->key_lock_return = down;
                break;
            case SDLK_LEFT: input->key_lock_left = down; break;
            case SDLK_RIGHT: input->key_lock_right = down; break;
            case SDLK_UP: input->key_lock_up = down; break;
            case SDLK_DOWN: input->key_lock_down = down; break;
            case SDLK_LSHIFT: input->key_lock_lshift = down; break;
            case SDLK_HOME: input->key_lock_home = down; break;
            case SDLK_END: input->key_lock_end = down; break;
            case SDLK_BACKSPACE: input->key_lock_backspace = down; break;
            case SDLK_DELETE: input->key_lock_delete = down; break;
            case SDLK_COMMA: input->key_lock_comma = down; break;
            case SDLK_PERIOD: input->key_lock_period = down; break;
            case SDLK_F1: input->key_lock_f1 = down; break;
            case SDLK_F2: input->key_lock_f2 = down; break;
            case SDLK_F3: input->key_lock_f3 = down; break;
            case SDLK_F4: input->key_lock_f4 = down; break;
            case SDLK_F5: input->key_lock_f5 = down; break;
            case SDLK_F6: input->key_lock_f6 = down; break;
            case SDLK_F7: input->key_lock_f7 = down; break;
            case SDLK_F8: input->key_lock_f8 = down; break;
            case SDLK_F9: input->key_lock_f9 = down; break;
            case SDLK_F10: input->key_lock_f10 = down; break;
            case SDLK_F11: input->key_lock_f11 = down; break;
            case SDLK_F12: input->key_lock_f12 = down; break;
        }
    }
}

static void sdl_key_mapping(SDL_Keysym* keysym, bool down) {
    
    cInputResetKeys(input);
    
    switch(keysym->sym) {
        case SDLK_0: input->key_0 = down; break;
        case SDLK_1: input->key_1 = down; break;
        case SDLK_2: input->key_2 = down; break;
        case SDLK_3: input->key_3 = down; break;
        case SDLK_4: input->key_4 = down; break;
        case SDLK_5: input->key_5 = down; break;
        case SDLK_6: input->key_6 = down; break;
        case SDLK_7: input->key_7 = down; break;
        case SDLK_8: input->key_8 = down; break;
        case SDLK_9: input->key_9 = down; break;
            
        case SDLK_a: input->key_a = down; break;
        case SDLK_b: input->key_b = down; break;
        case SDLK_c: input->key_c = down; break;
        case SDLK_d: input->key_d = down; break;
        case SDLK_e: input->key_e = down; break;
        case SDLK_f: input->key_f = down; break;
        case SDLK_g: input->key_g = down; break;
        case SDLK_h: input->key_h = down; break;
        case SDLK_i: input->key_i = down; break;
        case SDLK_j: input->key_j = down; break;
        case SDLK_k: input->key_k = down; break;
        case SDLK_l: input->key_l = down; break;
        case SDLK_m: input->key_m = down; break;
        case SDLK_n: input->key_n = down; break;
        case SDLK_o: input->key_o = down; break;
        case SDLK_p: input->key_p = down; break;
        case SDLK_q: input->key_q = down; break;
        case SDLK_r: input->key_r = down; break;
        case SDLK_s: input->key_s = down; break;
        case SDLK_t: input->key_t = down; break;
        case SDLK_u: input->key_u = down; break;
        case SDLK_v: input->key_v = down; break;
        case SDLK_w: input->key_w = down; break;
        case SDLK_x: input->key_x = down; break;
        case SDLK_y: input->key_y = down; break;
        case SDLK_z: input->key_z = down; break;
          
        case SDLK_SPACE: input->key_space = down; break;
        case SDLK_PLUS:
        case SDLK_KP_PLUS:
            input->key_plus = down;
            break;
        case SDLK_MINUS:
        case SDLK_KP_MINUS:
            input->key_minus = down;
            break;
        case SDLK_TAB: input->key_tab = down; break;
        case SDLK_LGUI: input->key_lgui = down; break;
        case SDLK_LCTRL: input->key_lctrl = down; break;
        case SDLK_ESCAPE: input->key_escape = down; break;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
            input->key_return = down;
        break;
        case SDLK_LEFT: input->key_left = down; break;
        case SDLK_RIGHT: input->key_right = down; break;
        case SDLK_UP: input->key_up = down; break;
        case SDLK_DOWN: input->key_down = down; break;
        case SDLK_LSHIFT: input->key_lshift = down; break;
        case SDLK_HOME: input->key_home = down; break;
        case SDLK_END: input->key_end = down; break;
        case SDLK_BACKSPACE: input->key_backspace = down; break;
        case SDLK_DELETE: input->key_delete = down; break;
        case SDLK_COMMA: input->key_comma = down; break;
        case SDLK_PERIOD: input->key_period = down; break;
        case SDLK_F1: input->key_f1 = down; break;
        case SDLK_F2: input->key_f2 = down; break;
        case SDLK_F3: input->key_f3 = down; break;
        case SDLK_F4: input->key_f4 = down; break;
        case SDLK_F5: input->key_f5 = down; break;
        case SDLK_F6: input->key_f6 = down; break;
        case SDLK_F7: input->key_f7 = down; break;
        case SDLK_F8: input->key_f8 = down; break;
        case SDLK_F9: input->key_f9 = down; break;
        case SDLK_F10: input->key_f10 = down; break;
        case SDLK_F11: input->key_f11 = down; break;
        case SDLK_F12: input->key_f12 = down; break;
            
    }
}

static void handle_reset_confirmation_keys(void) {
    if(input->key_return) {
        set_info_timer("reset project");
        reset_project();
        cSynthReset(synth);
        reset_confirmation = false;
        input->key_lock_return = true;
    } else {
        // reset cancelled.
        reset_confirmation = false;
    }
}

static void handle_tempo_keys(void) {
    
    bool zero = false;
    bool move_cursor_down = false;
    int cursor_x = tempo_selection_x;
    int cursor_y = tempo_selection_y;
    char value = -1;

    if(input->key_plus) {
        if(cursor_y == 0) {
            synth->tempo_map[cursor_x][cursor_y]->bpm++;
            if(synth->tempo_map[cursor_x][cursor_y]->bpm > 999) {
                synth->tempo_map[cursor_x][cursor_y]->bpm = 999;
            }
        }
    } else if(input->key_minus) {
        if(cursor_y == 0) {
            synth->tempo_map[cursor_x][cursor_y]->bpm--;
            if(synth->tempo_map[cursor_x][cursor_y]->bpm < 0) {
                synth->tempo_map[cursor_x][cursor_y]->bpm = 0;
            }
        }
    } else if(input->key_backspace || input->key_delete) {
        zero = true;
        if(cursor_y > 0) {
            move_cursor_down = true;
        }
    } else if(input->key_x) {
        if (!modifier) {
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
        }
    } else if(input->key_0) {
        value = 0;
    } else if(input->key_1) {
        value = 1;
    } else if(input->key_2) {
        value = 2;
    } else if(input->key_3) {
        value = 3;
    } else if(input->key_4) {
        value = 4;
    } else if(input->key_5) {
        value = 5;
    } else if(input->key_6) {
        value = 6;
    } else if(input->key_7) {
        value = 7;
    } else if(input->key_8) {
        value = 8;
    } else if(input->key_9) {
        value = 9;
    } else if(input->key_a) {
        value = 10;
    } else if(input->key_b) {
        value = 11;
    } else if(input->key_c) {
        value = 12;
    } else if(input->key_d) {
        value = 13;
    } else if(input->key_e) {
        value = 14;
    } else if(input->key_f) {
        value = 15;
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

static void handle_wavetable_keys(void) {
    
    bool zero = false;
    bool move_cursor_down = false;
    int cursor_x = wavetable_selection_x;
    int cursor_y = wavetable_selection_y;
    char value = -1;
    
    bool h_switch = false;
    if(cursor_x % 2 == 0) {
        h_switch = true;
    }
    
    if(input->key_plus) {
            if(cursor_y == 0 && h_switch) {
                synth->wavetable_map[cursor_x][cursor_y]->speed++;
                if(synth->wavetable_map[cursor_x][cursor_y]->speed > 999) {
                    synth->wavetable_map[cursor_x][cursor_y]->speed = 999;
                }
            }
    } else if(input->key_minus) {
        if(cursor_y == 0 && h_switch) {
            synth->wavetable_map[cursor_x][cursor_y]->speed--;
            if(synth->wavetable_map[cursor_x][cursor_y]->speed < 0) {
                synth->wavetable_map[cursor_x][cursor_y]->speed = 0;
            }
        }
    } else if(input->key_backspace || input->key_delete) {
        zero = true;
        if(cursor_y > 0) {
            move_cursor_down = true;
        }
    } else if(input->key_x) {
        if(cursor_y > 0 && cursor_y != 2) {
            if(synth->wavetable_map[cursor_x][cursor_y]->active) {
                synth->wavetable_map[cursor_x][cursor_y]->active = false;
                move_cursor_down = true;
            } else {
                synth->wavetable_map[cursor_x][cursor_y]->active = true;
                move_cursor_down = true;
            }
        }
        
        if(cursor_y == 2) {
            move_cursor_down = true;
        }
    } else if(input->key_0) {
        value = 0;
    } else if(input->key_1) {
        value = 1;
    } else if(input->key_2) {
        value = 2;
    } else if(input->key_3) {
        value = 3;
    } else if(input->key_4) {
        value = 4;
    } else if(input->key_5) {
        value = 5;
    } else if(input->key_6) {
        value = 6;
    } else if(input->key_7) {
        value = 7;
    } else if(input->key_8) {
        value = 8;
    } else if(input->key_9) {
        value = 9;
    } else if(input->key_a) {
        value = 10;
    } else if(input->key_b) {
        value = 11;
    } else if(input->key_c) {
        value = 12;
    } else if(input->key_d) {
        value = 13;
    } else if(input->key_e) {
        value = 14;
    } else if(input->key_f) {
        value = 15;
    }
    
    if(cursor_y > 1) {
        if(zero) {
            synth->wavetable_map[cursor_x][cursor_y]->value = 0;
            if(!h_switch) {
                synth->wavetable_map[cursor_x][cursor_y]->value = 1;
            }
            move_cursor_down = true;
        } else if(value > -1) {
            if(h_switch) {
                value = value % 6;
                synth->wavetable_map[cursor_x][cursor_y]->value = value;
            }
            if(!h_switch && value == 0) {
                synth->wavetable_map[cursor_x][cursor_y]->value = 1;
            } else if(!h_switch) {
                synth->wavetable_map[cursor_x][cursor_y]->value = value;
            }
            move_cursor_down = true;
        }
        
    } else if(cursor_y == 0) {
        // bpm
        if(zero) {
            synth->wavetable_map[cursor_x][cursor_y]->speed = 0;
        } else if(value > -1) {
            int old_bpm = synth->wavetable_map[cursor_x][cursor_y]->speed;
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
            synth->wavetable_map[cursor_x][cursor_y]->speed = number;
        }
    }
    
    if(move_cursor_down) {
        wavetable_selection_y++;
        if(wavetable_selection_y >= synth->wavetable_height) {
            wavetable_selection_y = 2;
        }
    }
}


static void handle_note_keys(void) {
    
    int cursor_y = visual_cursor_y;
    
    if(input->key_z) {
        add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 12);
    } else if(input->key_s) {
        add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 13);
    } else if(input->key_x) {
        add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 14);
    } else if(input->key_d) {
        add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 15);
    } else if(input->key_c) {
        add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 16);
    } else if(input->key_v) {
        add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 17);
    } else if(input->key_g) {
        add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 18);
    } else if(input->key_b) {
        add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 19);
    } else if(input->key_h) {
        add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 20);
    } else if(input->key_n) {
        add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 21);
    } else if(input->key_j) {
        add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 22);
    } else if(input->key_m) {
        add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 23);
    } else if(input->key_comma) {
        add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 24);
    } else if(input->key_l) {
        add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 25);
    } else if(input->key_period) {
        add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 26);
    } else if(input->key_q) {
        add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 24);
    } else if(input->key_2) {
        add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 25);
    } else if(input->key_w) {
        add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 26);
    } else if(input->key_3) {
        add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 27);
    } else if(input->key_e) {
        add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 28);
    } else if(input->key_r) {
        add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 29);
    } else if(input->key_5) {
        add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 30);
    } else if(input->key_t) {
        add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 31);
    } else if(input->key_6) {
        add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 32);
    } else if(input->key_y) {
        add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 33);
    } else if(input->key_7) {
        add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 34);
    } else if(input->key_u) {
        add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 35);
    } else if(input->key_i) {
        add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 36);
    } else if(input->key_9) {
        add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 37);
    } else if(input->key_o) {
        add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 38);
    } else if(input->key_0) {
        add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 39);
    } else if(input->key_p) {
        add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 40);
    }
}

static void handle_pattern_keys(void) {
    
    bool zero = false;
    int number = 0;
    
    if(input->key_backspace || input->key_delete) {
        zero = true;
    } else if(input->key_0) {
        number = 0;
    } else if(input->key_1) {
        number = 1;
    } else if(input->key_2) {
        number = 2;
    } else if(input->key_3) {
        number = 3;
    } else if(input->key_4) {
        number = 4;
    } else if(input->key_5) {
        number = 5;
    } else if(input->key_6) {
        number = 6;
    } else if(input->key_7) {
        number = 7;
    } else if(input->key_8) {
        number = 8;
    } else if(input->key_9) {
        number = 9;
    } else {
        return;
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

void handle_param_value(void) {
    
    int cursor_y = visual_cursor_y;
    
    if(input->key_0) {
       add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 0);
    } else if(input->key_1) {
        add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 1);
    } else if(input->key_2) {
        add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 2);
    } else if(input->key_3) {
        add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 3);
    } else if(input->key_4) {
        add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 4);
    } else if(input->key_5) {
        add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 5);
    } else if(input->key_6) {
        add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 6);
    } else if(input->key_7) {
        add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 7);
    } else if(input->key_8) {
        add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 8);
    } else if(input->key_9) {
        add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 9);
    } else if(input->key_a) {
        add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 10);
    } else if(input->key_b) {
        add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 11);
    } else if(input->key_c) {
        add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 12);
    } else if(input->key_d) {
        add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 13);
    } else if(input->key_e) {
        add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 14);
    } else if(input->key_f) {
        add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 15);
    } else if(input->key_g) {
        add_track_node_with_octave(synth->track_cursor_x, cursor_y, editing, 16);
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
        
        // TODO this is ugly, find a better way
        if(node->effect_value == 16) {
            // remove bitcrush
            synth->bitcrush_active = false;
        }
        
        node->effect = '-';
        node->effect_value = -1;
        // remove params as well
        node->effect_param1 = '-';
        node->effect_param1_value = -1;
        node->effect_param2 = '-';
        node->effect_param2_value = -1;
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

static void handle_instrument_effect_keys(void) {
    
    int instrument = selected_instrument_id;
    char value = -1;
    
    if(input->key_0) {
        value = 0;
    } else if(input->key_1) {
        value = 1;
    } else if(input->key_2) {
        value = 2;
    } else if(input->key_3) {
        value = 3;
    } else if(input->key_4) {
        value = 4;
    } else if(input->key_5) {
        value = 5;
    } else if(input->key_6) {
        value = 6;
    } else if(input->key_7) {
        value = 7;
    } else if(input->key_8) {
        value = 8;
    } else if(input->key_9) {
        value = 9;
    } else if(input->key_a) {
        value = 10;
    } else if(input->key_b) {
        value = 11;
    } else if(input->key_c) {
        value = 12;
    } else if(input->key_d) {
        value = 13;
    } else if(input->key_e) {
        value = 14;
    } else if(input->key_f) {
        value = 15;
    } else if(input->key_g) {
        value = 16;
    }

    if(value > -1) {
        struct CTrackNode *node = synth->instrument_effects[instrument][instrument_editor_effects_y];
        if(node == NULL) {
            node = cSynthNewTrackNode();
            synth->instrument_effects[instrument][instrument_editor_effects_y] = node;
        }
        
        // only effects are allowed to go over F.
        if(instrument_editor_effects_x != 0 && value > 15) {
            return;
        }
        
        if(instrument_editor_effects_x == 0) {
            
            // TODO this is ugly, find a better way
            if(node->effect_value == 16 && value != 16) {
                // remove bitcrush
                synth->bitcrush_active = false;
            }
            
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
        }
    }
}

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
    if(visualiser) {
        prepare_visualiser(s_byteStream, byteStreamLength);
    }
}

static void clear_clipping_color_from_visualiser(void) {
    
    for(int v_x = 0; v_x < s_width; v_x++) {
        for(int v_y = 0; v_y < s_height; v_y++) {
            visualiser2d[v_x][v_y] = color_bg;
        }
    }
}

static void prepare_visualiser(Sint16 *s_byteStream, int byteStreamLength) {
    
    for(int v_x = 0; v_x < s_width; v_x++) {
        for(int v_y = 0; v_y < s_height; v_y++) {
            if(visualiser2d[v_x][v_y] != color_visualiser_clipping) {
                visualiser2d[v_x][v_y] = color_bg;
            }
        }
    }
    
    int x_counter = 0;
    for(int v_x = 0; v_x < s_width*2; v_x+=2) {
        int scaled_x = v_x;
        if(scaled_x < byteStreamLength-1 && x_counter < s_width) {
            Sint16 sample_left = s_byteStream[scaled_x];
            Sint16 sample_right = s_byteStream[scaled_x+1];
            sample_left *= 0.00219733268227;
            sample_right *= 0.00219733268227;
            int fourth = 72;
            int visual_left = sample_left + fourth;
            int visual_right = sample_right + (fourth*3);
            int middle = s_height/2;
            visualiser2d[x_counter][middle] = 0xFFFFFFFF;
            unsigned int color = color_visualiser;
            if(visual_left < 2) {
                visual_left = 0;
                color = color_visualiser_clipping;
            } else if (visual_left >= (fourth*2)-1) {
                visual_left = (fourth*2)-1;
                color = color_visualiser_clipping;
            }
            if(visualiser2d[x_counter][visual_left] != color_visualiser_clipping) {
                visualiser2d[x_counter][visual_left] = color;
            }
            color = color_visualiser;
            if(visual_right < (fourth*2)+2) {
                visual_right = (fourth*2)+1;
                color = color_visualiser_clipping;
            } else if (visual_right >= s_height-1) {
                visual_right = s_height-1;
                color = color_visualiser_clipping;
            }
            if(visualiser2d[x_counter][visual_right] != color_visualiser_clipping) {
                visualiser2d[x_counter][visual_right] = color;
            }
            x_counter++;
        }
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
        current_waveform = 5;
    } else if(current_waveform > 5) {
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
    if(current_waveform == 5) {
        synth->voices[pattern_cursor_x]->waveform = synth->custom_table;
    }
    synth->patterns_and_voices[pattern_cursor_x][pattern_cursor_y] = current_waveform;
}

static void change_param(bool plus) {
    
    int x = pattern_cursor_x;
    int y = pattern_cursor_y;
    
    if(y == 0) {
        change_waveform(plus);
    } else if(y == 17 || y == 18) {
        // nothing
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
    } else if(y == 19 && x == 1) {
        // nothing
    } else if(y == 19 && x == 2) {
        // nothing
    }
    else if(y == 20 && x == 2) {
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
        // nothing
    } else if(y == 20) {
        // nothing
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
            adsr_invert_y_render(i_pos_x, i_pos_y, color_envelope);
        }
        i++;
    }
}

static void render_custom_table(double dt) {
    
    struct CInstrument *ins = synth->custom_instrument;
    int max_nodes = ins->adsr_nodes;
    int inset_x = 50;
    int inset_y_node = 50;
    double speed = 0.0008*dt;
    if(modifier) {
        speed *= 0.1;
    }
    
    if(selected_custom_table_node_index > 0 && selected_custom_table_node_index < max_nodes-1) {
    
        if (pressed_left) {
            double pos1 = ins->adsr[selected_custom_table_node_index-1]->pos;
            double pos2 = ins->adsr[selected_custom_table_node_index]->pos;
            if(pos2 > pos1) {
                ins->adsr[selected_custom_table_node_index]->pos -= speed;
            }
            if(pos2 < pos1) {
                ins->adsr[selected_custom_table_node_index]->pos = pos1;
            }
        } else if(pressed_right) {
            if(selected_instrument_node_index < max_nodes-1) {
                double pos1 = ins->adsr[selected_custom_table_node_index]->pos;
                double pos2 = ins->adsr[selected_custom_table_node_index+1]->pos;
                if(pos1 < pos2) {
                    ins->adsr[selected_custom_table_node_index]->pos += speed;
                }
                if(ins->adsr[selected_custom_table_node_index]->pos > pos2) {
                    ins->adsr[selected_custom_table_node_index]->pos = pos2;
                }
            } else if(selected_custom_table_node_index == max_nodes-1) {
                // last node
                ins->adsr[selected_custom_table_node_index]->pos += speed;
            }
        }
    }
    if(pressed_down) {
        ins->adsr[selected_custom_table_node_index]->amp -= speed;
        if(ins->adsr[selected_custom_table_node_index]->amp < -1) {
            ins->adsr[selected_custom_table_node_index]->amp = -1;
        }
    } else if(pressed_up) {
        ins->adsr[selected_custom_table_node_index]->amp += speed;
        if(ins->adsr[selected_custom_table_node_index]->amp > 1) {
            ins->adsr[selected_custom_table_node_index]->amp = 1;
        }
    }

    double amp_factor = 100;
    double pos_factor = 400;
    int i;
    
    for(i = 0; i < 1000; i++) {
        double g_pos = (i*(pos_factor*0.001)) + inset_x;
        int top_line_y = 150;
        int bottom_line_y = -50;
        int middle_line = 50;
        adsr_invert_y_render(g_pos, top_line_y, color_text);
        adsr_invert_y_render(g_pos, middle_line, color_inactive_text);
        adsr_invert_y_render(g_pos, bottom_line_y, color_text);
    }
    
    for(i = 20; i < 220; i++) {
        raster2d[50][i] = color_text;
        raster2d[450][i] = color_text;
    }
    
    for(i = 0; i < max_nodes-1; i++) {
        struct CadsrNode *node = ins->adsr[i];
        struct CadsrNode *node2 = ins->adsr[i+1];
        double g_amp = (node->amp*amp_factor) + inset_y_node;
        double g_pos = (node->pos*pos_factor) + inset_x - envelope_node_camera_offset;
        double g_amp2 = (node2->amp*amp_factor) + inset_y_node;
        double g_pos2 = (node2->pos*pos_factor) + inset_x - envelope_node_camera_offset;
        draw_line((int)g_pos, (int)g_amp, (int)g_pos2, (int)g_amp2);
    }
    
    // render dots for nodes
    for(i = 0; i < max_nodes; i++) {
        struct CadsrNode *node = ins->adsr[i];
        double g_amp = (node->amp*amp_factor) + inset_y_node;
        double g_pos = (node->pos*pos_factor) + inset_x;
        
        envelope_node_camera_offset = 0;
        
        for(int x = -2; x < 2; x++) {
            for(int y = -2; y < 2; y++) {
                int color = color_inactive_instrument_node;
                if(i == selected_custom_table_node_index) {
                    color = color_active_instrument_node;
                }
                adsr_invert_y_render(g_pos+x-envelope_node_camera_offset, g_amp+y, color);
            }
        }
    }
    
    cSynthWriteCustomTableFromNodes(synth);
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
        double g_pos = (node->pos*pos_factor) + inset_x - envelope_node_camera_offset;
        double g_amp2 = (node2->amp*amp_factor) + inset_y;
        double g_pos2 = (node2->pos*pos_factor) + inset_x - envelope_node_camera_offset;
        draw_line((int)g_pos, (int)g_amp, (int)g_pos2, (int)g_amp2);
    }
    
    // render dots for nodes
    for(i = 0; i < max_nodes; i++) {
        struct CadsrNode *node = ins->adsr[i];
        double g_amp = (node->amp*amp_factor) + inset_y;
        double g_pos = (node->pos*pos_factor) + inset_x;
        if(i == selected_instrument_node_index) {
            envelope_node_camera_offset = (int)(g_pos) - (s_width/2);
            if(envelope_node_camera_offset < 0) {
                envelope_node_camera_offset = 0;
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
                adsr_invert_y_render(g_pos+x-envelope_node_camera_offset, g_amp+y, color);
            }
        }
    }
    
    char cval[64];
    char c = cSynthGetCharFromParam((char)selected_instrument_id);
    snprintf(cval, 63, "instrument %c", c);
    cEngineRenderLabelWithParams(raster2d, cval, 1, 2, color_text, cengine_color_transparent);
    
    // render preset instrument effects.
    int offset_y = 13;
    for (int x = 0; x < 3; x++) {
        for (int y = 0; y < visual_instrument_effects; y++) {
            
            //effect type
            char cval[20];
            struct CTrackNode *t = synth->instrument_effects[selected_instrument_id][y];
            
            int color = color_text;
            int bg_color = cengine_color_transparent;
            if(x == instrument_editor_effects_x && y == instrument_editor_effects_y && instrument_editor_effects) {
                color = color_edit_text;
                bg_color = color_edit_marker;
            } else if(!instrument_editor_effects){
                color = color_inactive_text;
            }
            
            if(t != NULL) {
                if(x == 0) {
                    snprintf(cval, 19, "%c", t->effect);
                    cEngineRenderLabelWithParams(raster2d, cval, x+1, y+offset_y, color, bg_color);
                }
                if(x == 1) {
                    snprintf(cval, 19, "%c", t->effect_param1);
                    cEngineRenderLabelWithParams(raster2d, cval, x+1, y+offset_y, color, bg_color);
                }
                if(x == 2) {
                    snprintf(cval, 19, "%c", t->effect_param2);
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
            
            int bg_color = cengine_color_transparent;
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
                char cval[4];
                snprintf(cval, 3, "%d", val);
                cEngineRenderLabelWithParams(raster2d, get_wave_type_as_char(val), x*10+inset_x, y+inset_y, wave_color, bg_color);
            } else if(y == 17) {
                char cval[10];
                int ins_nr = x;
                char c = cSynthGetCharFromParam((char)ins_nr);
                snprintf(cval, 9, "ins %c", c);
                cEngineRenderLabelWithParams(raster2d, cval, x*10+inset_x, y+inset_y, color, bg_color);
            } else if(y == 18) {
                char cval[10];
                int ins_nr = x;
                ins_nr += 6;
                char c = cSynthGetCharFromParam((char)ins_nr);
                snprintf(cval, 9, "ins %c", c);
                cEngineRenderLabelWithParams(raster2d, cval, x*10+inset_x, y+inset_y, color, bg_color);
            } else if(y == 19 && x < 4) {
                char cval[10];
                int ins_nr = x;
                ins_nr += 12;
                char c = cSynthGetCharFromParam((char)ins_nr);
                snprintf(cval, 9, "ins %c", c);
                cEngineRenderLabelWithParams(raster2d, cval, x*10+inset_x, y+inset_y, color, bg_color);
            } else if(y == 20 && x == 0) {
                cEngineRenderLabelWithParams(raster2d, "wavetable", x*10+inset_x, y+inset_y, color, bg_color);
            } else if(y == 21 && x == 2) {
                cEngineRenderLabelWithParams(raster2d, "cust edit", x*10+inset_x, y+inset_y, color, bg_color);
            } else if(y == 20 && x == 1) {
                char cval[10];
                snprintf(cval, 9, "amp %d%%", synth->master_amp_percent);
                if(synth->audio_clips) {
                    bg_color = color_file_name_text;
                    synth->audio_clips = false;
                }
                cEngineRenderLabelWithParams(raster2d, cval, x*10+inset_x, y+inset_y, color, bg_color);
            } else if(y == 20 && x == 2) {
                char cval[20];
                snprintf(cval, 19, "rows %d", synth->track_height);
                cEngineRenderLabelWithParams(raster2d, cval, x*10+inset_x, y+inset_y, color, bg_color);
            } else if(y == 20 && x == 3) {
                char cval[20];
                snprintf(cval, 19, "arp %d", synth->arpeggio_speed);
                cEngineRenderLabelWithParams(raster2d, cval, x*10+inset_x, y+inset_y, color, bg_color);
            } else if(y == 20 && x == 4) {
                cEngineRenderLabelWithParams(raster2d, "tempo", x*10+inset_x, y+inset_y, color, bg_color);
            } else if(y == 20 && x == 5) {
                cEngineRenderLabelWithParams(raster2d, "visual", x*10+inset_x, y+inset_y, color, bg_color);
                
            } else if(y == 21 && x == 0) {
                if(synth->preview_enabled) {
                    cEngineRenderLabelWithParams(raster2d, "preview 1", x*10+inset_x, y+inset_y, color, bg_color);
                } else {
                    cEngineRenderLabelWithParams(raster2d, "preview 0", x*10+inset_x, y+inset_y, color, bg_color);
                }
            } else if(y == 21 && x == 1) {
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
                char cval[4];
                snprintf(cval, 3, "%d", pattern);
                cEngineRenderLabelWithParams(raster2d, cval, x*10+inset_x, y+inset_y, color, bg_color);
                
                if(x == 0) {
                    // print track numbers
                    int track_nr = y-1+visual_pattern_offset;
                    char cval[4];
                    snprintf(cval, 3, "%d", track_nr);
                    int x_offset = 1;
                    if(track_nr < 10) {
                        x_offset = 2;
                    }
                    cEngineRenderLabelWithParams(raster2d, cval, x_offset, y+1, color_text, cengine_color_transparent);
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
        snprintf(cval, 19, "p:%d t:%d", pattern_at_cursor, track_at_cursor);
        cEngineRenderLabelWithParams(raster2d, cval, 55, 23, cengine_color_white, cengine_color_transparent);
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
        return "sqr";
    }
    if(type == 3) {
        return "tri";
    }
    if(type == 4) {
        return "nois";
    }
    if(type == 5) {
        return "cust";
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
        cEngineRenderLabelWithParams(raster2d, get_wave_type_as_char(val), 2+x*10, -visual_cursor_y+5, wave_color, cengine_color_transparent);
    }
}

static void render_visualiser(void) {
    
    for(int x = 0; x < s_width; x++) {
        for(int y = 0; y < s_height; y++) {
            raster2d[x][y] = visualiser2d[x][y];
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
                char cval[4];
                snprintf(cval, 3, "%d", track_nr);
                int x_offset = 1;
                if(track_nr < 10) {
                    x_offset = 2;
                }
                cEngineRenderLabelWithParams(raster2d, cval, x_offset, y+1, color_text, cengine_color_transparent);
            }

            int color = color_inactive_text;
            int bg_color = cengine_color_transparent;
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
                snprintf(cval, 9, "BPM %d", bpm);
            } else {
                char node_value = t->ticks;
                char c = cSynthGetCharFromParam((char)node_value);
                snprintf(cval, 9, "%c", c);
            }
            cEngineRenderLabelWithParams(raster2d, cval, x*10+inset_x, y+inset_y, color, bg_color);
        }
    }
}

static void render_reset_confirmation(double dt) {
    
    for(int x = 0; x < s_width; x++) {
        for(int y = 0; y < s_height; y++) {
            raster2d[x][y] = color_bg;
        }
    }
    cEngineRenderLabelWithParams(raster2d, "Reset project? Press [RETURN] to reset or [ESCAPE].", 2, 3, color_text, cengine_color_transparent);
}

static void render_wavetable_editor(double dt) {
    
    int inset_x = 5;
    int inset_y = 1;
    
    bool h_switch = true;
    
    for(int x = 0; x < synth->wavetable_width; x++) {
        for(int y = 0; y < synth->wavetable_height; y++) {
            
            if(x == 0 && y > 1) {
                // print track numbers
                int track_nr = y-2;
                char cval[4];
                snprintf(cval, 3, "%d", track_nr);
                int x_offset = 1;
                if(track_nr < 10) {
                    x_offset = 2;
                }
                cEngineRenderLabelWithParams(raster2d, cval, x_offset, y+1, color_text, cengine_color_transparent);
            }
            
            int color = color_inactive_text;
            int bg_color = cengine_color_transparent;
            
            char cval[20];
            struct CWavetableNode *t = synth->wavetable_map[x][y];
            if(t->active || (y == 0 && h_switch)) {
                color = color_text;
            }
            
            if(wavetable_selection_x == x && wavetable_selection_y == y) {
                bg_color = color_edit_marker;
                color = color_edit_text;
            }
            
            if(h_switch) {
                
                if (y == 0) {
                    snprintf(cval, 19, "speed %d", t->speed);
                    cEngineRenderLabelWithParams(raster2d, cval, x*5+inset_x, y+inset_y, color, bg_color);
                } else if (y == 1) {
                    
                    cEngineRenderLabelWithParams(raster2d, "loop", x*5+inset_x, y+inset_y, color, bg_color);
                } else {
                    char node_value = t->value;
                    char c = cSynthGetCharFromParam((char)node_value);
                    snprintf(cval, 19, "%c", c);
                    cEngineRenderLabelWithParams(raster2d, get_wave_type_as_char(node_value), x*5+inset_x, y+inset_y, color, bg_color);
                }
                
            } else {
                if (y > 1) {
                    struct CWavetableNode *p_t = synth->wavetable_map[x-1][y];
                    if(p_t->active) {
                        color = color_text;
                    } else {
                        color = color_inactive_text;
                    }
                    char node_value = t->value;
                    char c = cSynthGetCharFromParam((char)node_value);
                    snprintf(cval, 19, "%c", c);
                    cEngineRenderLabelWithParams(raster2d, cval, x*5+inset_x, y+inset_y, color, bg_color);
                } else {
                    //cEngineRenderLabelWithParams(raster2d, "-", x*5+inset_x+4, y+inset_y, color_inactive_text, bg_color);
                }
            }
        }
        
        if(h_switch) {
            inset_x = 5;
        } else {
            inset_x = 5;
        }
        h_switch = !h_switch;
    }
}

static void render_track(double dt) {
    
    if(help) {
        render_help();
        return;
    } else if(credits) {
        render_credits();
        return;
    } else if(reset_confirmation) {
        render_reset_confirmation(dt);
        return;
    } else if(visualiser && !instrument_editor && !file_editor) {
        render_visualiser();
        return;
    } else if(instrument_editor && !file_editor && !tempo_editor && !visualiser) {
        render_instrument_editor(dt);
        return;
    } else if(custom_table && !file_editor && !tempo_editor && !visualiser) {
        render_custom_table(dt);
        return;
    } else if(tempo_editor && !file_editor) {
        render_tempo_editor(dt);
        return;
    } else if(wavetable_editor && !file_editor) {
        render_wavetable_editor(dt);
        return;
    } else if(pattern_editor && !file_editor) {
        render_pattern_mapping();
        return;
    } else if(file_editor) {
        render_files();
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
            
            int bg_color = 0;
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
                            snprintf(cval, 19, "%c", c);
                            cEngineRenderLabelWithParams(raster2d, cval, inset_x+x+offset_x, pos_y, color, bg_color);
                        } else {
                            cEngineRenderLabelWithParams(raster2d, "-", inset_x+x+offset_x, pos_y, color, bg_color);
                        }
                    } else if(x_count == 2) {
                        //effect type
                        char cval[20];
                        snprintf(cval, 19, "%c", t->effect);
                        cEngineRenderLabelWithParams(raster2d, cval, inset_x+x+offset_x, pos_y, color, bg_color);
                    } else if(x_count == 3) {
                        //effect type
                        char cval[20];
                        snprintf(cval, 19, "%c", t->effect_param1);
                        cEngineRenderLabelWithParams(raster2d, cval, inset_x+x+offset_x, pos_y, color, bg_color);
                    } else if(x_count == 4) {
                        //effect type
                        char cval[20];
                        snprintf(cval, 19, "%c", t->effect_param2);
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
        snprintf(cval, 19, "p:%d t:%d r:%d", current_pattern, current_track, visual_cursor_y);
        cEngineRenderLabelWithParams(raster2d, cval, 50, 23, color_text, color_text_bg);
    }
}

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
	
    if(fullscreen) {
        window = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP);
    } else {
        window = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL);
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
            snprintf(title_string, 255, "%s", title);
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
    
    // contains an integer for every color/pixel on the sheet.
    raw_sheet = cAllocatorAlloc((sheet_width*sheet_height) * sizeof(unsigned int), "main.c raw_sheet 1");
    for(int r = 0; r < sheet_width*sheet_height; r++) {
        raw_sheet[r] = 0;
    }
    
    // load gfx from source.
    for (int i = 0; i < 1024*16; i++) {
        raw_sheet[i] = chars_gfx[i];
    }
    cEngineWritePixelData(raw_sheet);
    
    raw_sheet = cAllocatorFree(raw_sheet);
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
    c->max_touches = 8;
    c->level_width = 64;
    c->level_height = 64;
    c->max_buttons = 10;
    c->color_mode_argb = true;
    c->color_mode_rgba = false;
    c->show_fps = false;
    c->ground_render_enabled = false;
    cEngineInit(c);
    input = c->input;
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
    }
    
    if(fps_print_interval >= print_interval_limit) {
        fps_print_interval = 0;
    }
    fps_print_interval++;
    
    return delta;
}

static void main_loop(void) {
    
    int delay_ms = active_render_delay_ms;
    if(passive_rendering == false) {
        delay_ms = passive_render_delay_ms;
    }
    
    check_sdl_events(event);
    cInputApplyPendingLocks(input);
    
    if(instrument_editor) {
        synth->needs_redraw = true;
    }
    
    if(custom_table) {
        synth->needs_redraw = true;
    }
    
    if(credits) {
        synth->needs_redraw = true;
    }
    
	if(tempo_editor) {
		synth->needs_redraw = true; 
	}
    
    if(visualiser) {
        synth->needs_redraw = true;
    }
    
    if(any_key_pressed && !synth->preview_locked) {
        synth->sustain_active = true;
        synth->preview_locked = true;
    } else if(!any_key_pressed && synth->preview_locked) {
        synth->preview_locked = false;
        synth->preview_started = false;
        cSynthTurnOffSustain(synth);
    }
    
    update_info(last_dt);
    
    if(redraw_screen || synth->needs_redraw || !passive_rendering) {
       
        render_track(last_dt);
        render_info();
        
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
    
    int dt = get_delta();
    last_dt = dt;
    int wait_time = delay_ms-dt;
    if(dt < 10) {
        SDL_Delay(wait_time);
    }
}

static void debug_log(char *str) {
    // TODO need to use write safe location before using this.
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

static void copy_project_win(const char *name) {
    char *read_path = cAllocatorAlloc((1024 * sizeof(char*)), "win path 1");
    snprintf(read_path, 1023, "%s%s", "demos\\", name);
	char *write_path = cAllocatorAlloc((1024 * sizeof(char*)), "win path 2");
    snprintf(write_path, 1023, "%s%s", conf_default_dir, name);
    char *b = load_file(read_path);
    if(b != NULL) {
        FILE * fp;
        fp = fopen(write_path, "w+");
        if(fp != NULL) {
			if(debuglog) { printf("writing file at path %s", write_path); }
            fprintf(fp, "%s", b);
            fclose(fp);
        }
    } else {
        if(debuglog) { printf("could not load file %s", read_path); }
    }
    cAllocatorFree(read_path);
	cAllocatorFree(write_path);
}

static void load_config(void) {
    // for windows
    bool success = false;
    char *path = cAllocatorAlloc((1024 * sizeof(char*)), "config path");
    snprintf(path, 1023, "%sconfig.txt", conf_default_dir);
    char *b = load_file(path);
    if(b != NULL) {
        success = parse_config(b);
        cAllocatorFree(b);
    }
    if(!success) {
        if(debuglog) { printf("could not find config file. Writing config.txt and copying demo projects to %s\n", path); }
        bufferSize = 8192;
        FILE * fp;
        fp = fopen(path, "w+");
        if(fp != NULL) {
            fprintf(fp, "%s", "{\r\n\"buffer_size\":4096,\r\n\"buffer_size_info\":\"Must be a power of two, for example 256, 512, 1024, 2048, 4096, 8192, 16384\",\r\n\"fullscreen\":false,\r\n\"preview\":true,\r\n\"color_info_text_bg\" : {\"r\" : 0, \"g\" : 0, \"b\" : 0},\r\n\"color_file_name_text\" : {\"r\" : 250, \"g\" : 100, \"b\" : 100},\r\n\"color_inactive_instrument_node\" : {\"r\" : 100, \"g\" : 100, \"b\" : 0},\r\n\"color_active_instrument_node\" : {\"r\" : 255, \"g\" : 100, \"b\" : 0},\r\n\"color_envelope\" : {\"r\" : 0, \"g\" : 100, \"b\" : 100},\r\n\"color_visualiser\" : {\"r\" : 100, \"g\" : 250, \"b\" : 100},\r\n\"color_visualiser_clipping\" : {\"r\" : 250, \"g\" : 100, \"b\" : 100},\r\n\"color_inactive_text\" : {\"r\" : 100, \"g\" : 100, \"b\" : 100},\r\n\"color_text\" : {\"r\" : 240, \"g\" : 240, \"b\" : 240},\r\n\"color_text_bg\" : {\"r\" : 0, \"g\" : 0, \"b\" : 0},\r\n\"color_marker\" : {\"r\" : 100, \"g\" : 100, \"b\" : 0},\r\n\"color_solo\" : {\"r\" : 0, \"g\" : 0, \"b\" : 100},\r\n\"color_solo_text\" : {\"r\" : 255, \"g\" : 255, \"b\" : 255},\r\n\"color_mute\" : {\"r\" : 200, \"g\" : 0, \"b\" : 0},\r\n\"color_mute_text\" : {\"r\" : 255, \"g\" : 255, \"b\" : 255},\r\n\"color_active_row\" : {\"r\" : 0, \"g\" : 100, \"b\" : 100},\r\n\"color_active_row_text\" : {\"r\" : 0, \"g\" : 0, \"b\" : 0},\r\n\"color_playing_row\" : {\"r\" : 100, \"g\" : 250, \"b\" : 100},\r\n\"color_playing_row_text\" : {\"r\" : 0, \"g\" : 0, \"b\" : 0},\r\n\"color_edit_marker\" : {\"r\" : 255, \"g\" : 100, \"b\" : 0},\r\n\"color_edit_text\" : {\"r\" : 0, \"g\" : 0, \"b\" : 0},\r\n\"color_selection_marker\" : {\"r\" : 0, \"g\" : 255, \"b\" : 255},\r\n\"color_selection_text\" : {\"r\" : 0, \"g\" : 0, \"b\" : 0},\r\n\"color_bg\" : {\"r\" : 20, \"g\" : 20, \"b\" : 20},\r\n\"color_bg1\" : {\"r\" : 60, \"g\" : 60, \"b\" : 60},\r\n\"color_bg2\" : {\"r\" : 60, \"g\" : 60, \"b\" : 60},\r\n\"color_bg3\" : {\"r\" : 60, \"g\" : 60, \"b\" : 60},\r\n\"color_bg4\" : {\"r\" : 60, \"g\" : 60, \"b\" : 60},\r\n\"color_bg5\" : {\"r\" : 60, \"g\" : 60, \"b\" : 60},\r\n\"color_bg6\" : {\"r\" : 60, \"g\" : 60, \"b\" : 60},\r\n\"color_bg1_highlight\" : {\"r\" : 40, \"g\" : 40, \"b\" : 40},\r\n\"color_bg2_highlight\" : {\"r\" : 40, \"g\" : 40, \"b\" : 40},\r\n\"color_bg3_highlight\" : {\"r\" : 40, \"g\" : 40, \"b\" : 40},\r\n\"color_bg4_highlight\" : {\"r\" : 40, \"g\" : 40, \"b\" : 40},\r\n\"color_bg5_highlight\" : {\"r\" : 40, \"g\" : 40, \"b\" : 40},\r\n\"color_bg6_highlight\" : {\"r\" : 40, \"g\" : 40, \"b\" : 40}\r\n}\r\n");
            fclose(fp);
        }
        b = load_file(path);
        if(b != NULL) {
            success = parse_config(b);
            cAllocatorFree(b);
            copy_project_win("catslayer.snibb");
            copy_project_win("dunsa2.snibb");
            copy_project_win("fiskbolja.snibb");
            copy_project_win("horizon.snibb");
            copy_project_win("kissemisse.snibb");
            copy_project_win("korvhastig.snibb");
            copy_project_win("websnacks.snibb");
            copy_project_win("wrestchest.snibb");
			copy_project_win("projectcart.snibb");
        } else {
            if(debuglog) { printf("could not find config file after writing. path:%s\n", path); }
        }
    }
    path = cAllocatorFree(path);
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
    char *param_fullscreen = "fullscreen";
    char *param_preview = "preview";
    char *param_color_info_text_bg = "color_info_text_bg";
    char *param_color_file_name_text = "color_file_name_text";
    char *param_color_inactive_instrument_node = "color_inactive_instrument_node";
    char *param_color_active_instrument_node = "color_active_instrument_node";
    char *param_color_visualiser = "color_visualiser";
    char *param_color_visualiser_clipping = "color_visualiser_clipping";
    char *param_color_envelope = "color_envelope";
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
        
        object = cJSON_GetObjectItem(root, param_color_envelope);
        if(object != NULL) { color_envelope = get_color_from_json_config(object); }
        
        object = cJSON_GetObjectItem(root, param_color_visualiser);
        if(object != NULL) { color_visualiser = get_color_from_json_config(object); }
        
        object = cJSON_GetObjectItem(root, param_color_visualiser_clipping);
        if(object != NULL) { color_visualiser_clipping = get_color_from_json_config(object); }
        
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
    
    // override to use exe dir as default_dir
    bool path_defined = false;
    #if defined(platform_windows)
        char *b = load_file("exe_dir_as_workspace.txt");
        if(b != NULL) {
            path_defined = true;
            conf_default_dir = cAllocatorAlloc((1024 * sizeof(char*)), "conf default dir exe dir");
            snprintf(conf_default_dir, 1023, "%s", "");
            printf("exe dir override found. default dir:%s\n", conf_default_dir);
            cAllocatorFree(b);
        }
    #endif
    // get path from SDL2
    if(path_defined == false) {
        char *pref_path = SDL_GetPrefPath("lundstroem", "snibbetracker");
        if (pref_path != NULL) {
            conf_default_dir = cAllocatorAlloc((1024 * sizeof(char*)), "conf default dir 2");
            snprintf(conf_default_dir, 1023, "%s", pref_path);
            printf("default dir:%s\n", conf_default_dir);
        } else {
            if(debuglog) { printf("SDL_GetPrefPath returned NULL\n"); }
        }
    }
    
    #if defined(platform_windows)
        load_config();
        st_log("started executing.");
    #elif defined(platform_osx)
        //osx, load from bundle
        char *settings = get_settings_json();
        parse_config(settings);
        if(conf_default_dir != NULL) {
            copy_demo_songs(conf_default_dir);
        }
        free(settings);
    #else
        //linux
    #endif
    
    init_data();
    st_log("init data successful.");
    
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
    synth->tempo_skip_step = true;
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
            synth->looped = false;
            break;
        }
        buffer_size += chunk_size*2;
    }
    
    if(debuglog) { printf("export buffer size:%ld\n", buffer_size); }
    synth->current_track = starting_track;
    cSynthResetTrackProgress(synth, starting_track, 0);
    cSynthResetTempoIndex(synth);
    synth->tempo_skip_step = true;
    
    cSynthResetOnLoopBack(synth);
    exporting = true;
    synth->looped = false;
    synth->bitcrush_active = false;
    
    for (int i = 0; i < synth->track_width; i++) {
        struct CVoice *v = synth->voices[i];
        v->adsr_cursor = 0;
        v->note_on = false;
        for(int i = 0; i < v->delay_buffer_size; i++) {
            v->delay_buffer[i] = 0;
        }
    }
    
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
        
        /* write RIFF header */
        fwrite("RIFF", 1, 4, wav_file);
        write_little_endian((unsigned int)(36 + num_samples), 4, wav_file);
        
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
        //write_little_endian((unsigned int)(bytes_per_sample * num_samples * num_channels), 4, wav_file);
        write_little_endian((unsigned int)(num_samples), 4, wav_file);
        
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

static void handle_credits_keys(void) {
    
    credits_left = false;
    credits_right = false;
    credits_up = false;
    credits_down = false;
    
    if(input->key_return || input->key_escape) {
        credits = false;
    }
    if(input->key_left) {
        credits_left = true;
    }
    if(input->key_right) {
        credits_right = true;
    }
    if(input->key_up) {
        credits_up = true;
    }
    if(input->key_down) {
        credits_down = true;
    }
    if(input->key_space) {
        toggle_playback();
    }
}

static void handle_help_keys() {
    
    if(input->key_return || input->key_escape) {
        help = false;
    }
    if(input->key_up) {
        help_index--;
        if(help_index < 0) {
            help_index = help_index_max-1;
        }
    }
    if(input->key_down) {
        help_index++;
        if(help_index >= help_index_max) {
            help_index = 0;
        }
    }
    if(input->key_space) {
       toggle_playback();
    }
}

static void render_help(void) {
    
    int x = 0;
    int offset_x = 1;
    int inset_x = 1;
    int y = 1;
    int color = color_text;
    int bg_color = cengine_color_transparent;
    int page = help_index;
    
    if(page == 0) {
        cEngineRenderLabelWithParams(raster2d, "track view", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "----------------", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "- return: toggle editing on/off.", inset_x+x+offset_x, y, color, bg_color);
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
            cEngineRenderLabelWithParams(raster2d, "- ctrl+c/v/x: copy paste or cut note (or selection).", inset_x+x+offset_x, y, color, bg_color);
            y++;
            cEngineRenderLabelWithParams(raster2d, "- ctrl+f: toggle play cursor follow.", inset_x+x+offset_x, y, color, bg_color);
            y++;
            cEngineRenderLabelWithParams(raster2d, "- ctrl+plus/minus: transpose octave in selection.", inset_x+x+offset_x, y, color, bg_color);
            y++;
            cEngineRenderLabelWithParams(raster2d, "- ctrl+1-9: set stepsize.", inset_x+x+offset_x, y, color, bg_color);
            y++;
        #elif defined(platform_osx)
            cEngineRenderLabelWithParams(raster2d, "- cmd+left/right: change octave up/down.", inset_x+x+offset_x, y, color, bg_color);
            y++;
            cEngineRenderLabelWithParams(raster2d, "- cmd+up/down: move notes below cursor.", inset_x+x+offset_x, y, color, bg_color);
            y++;
            cEngineRenderLabelWithParams(raster2d, "- cmd+c/v/x: copy paste or cut note (or selection).", inset_x+x+offset_x, y, color, bg_color);
            y++;
            cEngineRenderLabelWithParams(raster2d, "- cmd+f: toggle play cursor follow.", inset_x+x+offset_x, y, color, bg_color);
            y++;
            cEngineRenderLabelWithParams(raster2d, "- cmd+plus/minus: transpose octave in selection.", inset_x+x+offset_x, y, color, bg_color);
            y++;
            cEngineRenderLabelWithParams(raster2d, "- cmd+1-9: set stepsize.", inset_x+x+offset_x, y, color, bg_color);
            y++;
        #endif
        cEngineRenderLabelWithParams(raster2d, "- plus/minus: transpose halfnote", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "- shift+arrow keys: make selection. (if edit is on)", inset_x+x+offset_x, y, color, bg_color);
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
        cEngineRenderLabelWithParams(raster2d, "pattern view", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "----------------", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "- arrow keys: move around grid.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "- plus/minus: cycle waveform, pattern numbers, rows etc.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "- return: go to instrument view (when cursor is at Ins 0-F).", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "- tab: go to track view.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "- e: jump to trackview with current position.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "- m: mute track (or channel if cursor is at the top).", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "- x: activate/inactivate track.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "- s: solo track (or channel if cursor is at the top).", inset_x+x+offset_x, y, color, bg_color);
        y++;
        #if defined(platform_windows)
            cEngineRenderLabelWithParams(raster2d, "- shift+up/down: paginate tracks (0-63).", inset_x+x+offset_x, y, color, bg_color);
            y++;
            cEngineRenderLabelWithParams(raster2d, "- ctrl+c/v: copy paste track data.", inset_x+x+offset_x, y, color, bg_color);
            y++;
            cEngineRenderLabelWithParams(raster2d, "- ctrl+up/down: move pattern rows.", inset_x+x+offset_x, y, color, bg_color);
            y++;
        #elif defined(platform_osx)
            cEngineRenderLabelWithParams(raster2d, "- shift+up/down: paginate tracks (0-63).", inset_x+x+offset_x, y, color, bg_color);
            y++;
            cEngineRenderLabelWithParams(raster2d, "- cmd+c/v: copy paste track data.", inset_x+x+offset_x, y, color, bg_color);
            y++;
            cEngineRenderLabelWithParams(raster2d, "- cmd+up/down: move pattern rows.", inset_x+x+offset_x, y, color, bg_color);
            y++;
        #endif
        cEngineRenderLabelWithParams(raster2d, "- home/end: set cursor to top / bottom.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "amp - master amplitude.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "rows - number of active rows in patterns.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "arp - general arpeggio speed.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "preview - 1 or 0. if notes are audiable when editing.", inset_x+x+offset_x, y, color, bg_color);
        y++;
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
            cEngineRenderLabelWithParams(raster2d, "- ctrl+arrow keys: move node slowly.", inset_x+x+offset_x, y, color, bg_color);
            y++;
        #elif defined(platform_osx)
            cEngineRenderLabelWithParams(raster2d, "- cmd+arrow keys: move node slowly.", inset_x+x+offset_x, y, color, bg_color);
            y++;
        #endif
        cEngineRenderLabelWithParams(raster2d, "- tab: cycle nodes.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "- return: go to pattern view.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "- shift: toggle editing of envelope or effects.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "- home/end: cycle instruments.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        
        cEngineRenderLabelWithParams(raster2d, "3 / 7", 1, 22, color, bg_color);
    }
    
    if(page == 3) {
        cEngineRenderLabelWithParams(raster2d, "custom wave", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "----------------", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "- arrow keys: move node.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "- tab: cycle nodes.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        y++;
        cEngineRenderLabelWithParams(raster2d, "wavetable view", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "----------------", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "- x: activate/inactivate row. first row is always", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "  active. toggle loop active/inactive.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "- 1-F: change overall speed on top row, or speed per row.", inset_x+x+offset_x, y, color, bg_color);
        y++;
    }
    
    if(page == 4) {
        cEngineRenderLabelWithParams(raster2d, "tempo view", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "----------------", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "- x: activate/inactivate row. each column", inset_x+x+offset_x, y, color, bg_color);
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
    
    if(page == 5) {
        
        cEngineRenderLabelWithParams(raster2d, "global controls", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "----------------", inset_x+x+offset_x, y, color, bg_color);
        y++;
        #if defined(platform_windows)
            cEngineRenderLabelWithParams(raster2d, "- ctrl+s: go to save view.", inset_x+x+offset_x, y, color, bg_color);
            y++;
            cEngineRenderLabelWithParams(raster2d, "- ctrl+o: go to load view.", inset_x+x+offset_x, y, color, bg_color);
            y++;
            cEngineRenderLabelWithParams(raster2d, "- ctrl+e: export wav.", inset_x+x+offset_x, y, color, bg_color);
            y++;
            cEngineRenderLabelWithParams(raster2d, "- ctrl+n: reset project.", inset_x+x+offset_x, y, color, bg_color);
            y++;
            cEngineRenderLabelWithParams(raster2d, "- ctrl+i: go to instrument view.", inset_x+x+offset_x, y, color, bg_color);
            y++;
            cEngineRenderLabelWithParams(raster2d, "- ctrl+t: go to tempo view.", inset_x+x+offset_x, y, color, bg_color);
            y++;
            cEngineRenderLabelWithParams(raster2d, "- ctrl+p: go to visualiser.", inset_x+x+offset_x, y, color, bg_color);
            y++;
            cEngineRenderLabelWithParams(raster2d, "- ctrl+r: go to wavetable view.", inset_x+x+offset_x, y, color, bg_color);
            y++;
            cEngineRenderLabelWithParams(raster2d, "- ctrl+j: go to custom wave view.", inset_x+x+offset_x, y, color, bg_color);
            y++;
            cEngineRenderLabelWithParams(raster2d, "- escape: exit view.", inset_x+x+offset_x, y, color, bg_color);
            y++;
        #elif defined(platform_osx)
            cEngineRenderLabelWithParams(raster2d, "- cmd+s: go to save view.", inset_x+x+offset_x, y, color, bg_color);
            y++;
            cEngineRenderLabelWithParams(raster2d, "- cmd+o: go to load view.", inset_x+x+offset_x, y, color, bg_color);
            y++;
            cEngineRenderLabelWithParams(raster2d, "- cmd+e: export wav.", inset_x+x+offset_x, y, color, bg_color);
            y++;
            cEngineRenderLabelWithParams(raster2d, "- cmd+n: reset project.", inset_x+x+offset_x, y, color, bg_color);
            y++;
            cEngineRenderLabelWithParams(raster2d, "- cmd+i: go to instrument view.", inset_x+x+offset_x, y, color, bg_color);
            y++;
            cEngineRenderLabelWithParams(raster2d, "- cmd+t: go to tempo view.", inset_x+x+offset_x, y, color, bg_color);
            y++;
            cEngineRenderLabelWithParams(raster2d, "- cmd+p: go to visualiser.", inset_x+x+offset_x, y, color, bg_color);
            y++;
            cEngineRenderLabelWithParams(raster2d, "- cmd+r: go to wavetable view.", inset_x+x+offset_x, y, color, bg_color);
            y++;
            cEngineRenderLabelWithParams(raster2d, "- cmd+j: go to custom wave view.", inset_x+x+offset_x, y, color, bg_color);
            y++;
            cEngineRenderLabelWithParams(raster2d, "- escape: exit view.", inset_x+x+offset_x, y, color, bg_color);
            y++;
        #endif
        cEngineRenderLabelWithParams(raster2d, "- f1-f9: switch views", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "save/load view", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "----------------", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "- character keys: enter filename.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "- backspace: remove character.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "- return: save/load file.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "- escape: exit view.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "5 / 7", 1, 22, color, bg_color);
    }
    
    if(page == 6) {
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
        cEngineRenderLabelWithParams(raster2d, "2xx - delay (speed, feedback).", inset_x+x+offset_x, y, color, bg_color);
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
        cEngineRenderLabelWithParams(raster2d, "6xx - FM (depth, speed).", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "7xx - detune (amount, amount) 88 is middle.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "8xx - PWM (linear position/oscillation depth, oscillation", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "      speed) on squarewave. if param2 is present, param1", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "      will be used for osc depth.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "6 / 7", 1, 22, color, bg_color);
    }
    
    if(page == 7) {
        cEngineRenderLabelWithParams(raster2d, "effects 2(2)", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "----------------", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "9xx - set wavetable/waveform for current channel.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "      param1: set wavetable lane 0-5 or param2:", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "      change waveform 0-5.", inset_x+x+offset_x, y, color, bg_color);
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
        cEngineRenderLabelWithParams(raster2d, "Gxx - bitcrush, params are multiplied to represent", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "      a bit depth. Affects all channels.", inset_x+x+offset_x, y, color, bg_color);
        y++;
        cEngineRenderLabelWithParams(raster2d, "7 / 7", 1, 22, color, bg_color);
    }
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
    
    if(credits_y > s_height-215) {
        credits_y_inc = -credits_y_inc;
        credits_y = s_height-216;
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
            credits_bg_color = cengine_color_transparent;
        }
        
        credits_higher_contrast = false;
        credits_scanlines_x = false;
        credits_scanlines_y = false;
    }
    
    cEngineRenderLabelByPixelPos(credits2d, "_code_and_design_", int_x+inset_x, int_y+inset_y, color, bg_color);
    inset_y+=inc;
    cEngineRenderLabelByPixelPos(credits2d, "   lundstroem", int_x+inset_x, int_y+inset_y, color, bg_color);
    inset_y+=inc;
    inset_y+=inc;
    cEngineRenderLabelByPixelPos(credits2d, "_design_and_testing_", int_x+inset_x, int_y+inset_y, color, bg_color);
    inset_y+=inc;
    cEngineRenderLabelByPixelPos(credits2d, "   salkinitzor", int_x+inset_x, int_y+inset_y, color, bg_color);
    inset_y+=inc;
    inset_y+=inc;
    cEngineRenderLabelByPixelPos(credits2d, "_feedback_and_testing_", int_x+inset_x, int_y+inset_y, color, bg_color);
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
    cEngineRenderLabelByPixelPos(credits2d, "   (C) lundstroem", int_x+inset_x, int_y+inset_y, color, bg_color);
    
    int hue_x = int_x;
    int hue_y = int_y;
    int hue_width = 240;
    int hue_height = 215;
   
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

static void resetColorValues(void) {
    
    c_hue_a = 0;
    c_hue_r = 0;
    c_hue_g = 0;
    c_hue_b = 0;
    
    c_new_hue_r = 0;
    c_new_hue_g = 0;
    c_new_hue_b = 0;
}

static void transformHSV(float H, float S, float V) {
    
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
                transformHSV(rotation, 1.0f, 0.99f);
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
