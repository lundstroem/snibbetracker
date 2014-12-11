#include <SDL2/SDL.h>
#include <SDL2_image/SDL_image.h>
#include <stdbool.h>
#include "Game.h"
#include "CInput.h"
#include "CTouch.h"
#include "CSynth.h"
#include "CAllocator.h"
#include "CTimer.h"

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
int selected_instrument_node_index = 0;

bool pressed_left = false;
bool pressed_right = false;
bool pressed_up = false;
bool pressed_down = false;

struct CTimer *infoTimer = NULL;
char *infoString = NULL;

static void setInfoTimer(char *string) {
    if(string != NULL) {
        int max_size = 20;
        int len = (int)strlen(string);
        if(len < max_size) {
            char *info = (char *)malloc(max_size * sizeof(char));
            sprintf(info, "%s", string);
            if(infoString != NULL) {
                free(infoString);
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
        int max_size = 20;
        int len = (int)strlen(string);
        if(len < max_size) {
            char *info = (char *)malloc(max_size * sizeof(char));
            sprintf(info, "%s:%d", string, data);
            if(infoString != NULL) {
                free(infoString);
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

static void setup_data()
{
    // contains an integer for every color/pixel on the screen.
    raster = (unsigned int *) cAllocatorAlloc((s_width*s_height) * sizeof(unsigned int), "main.c raster 1");
    int r = 0;
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
    
    infoTimer = cTimerNew(1000);
    cTimerReset(infoTimer);
    
}


void convertInput(int x, int y)
{
    input->mouse_x = x/4;
    input->mouse_y = y/4;
}


int quit = 0;

static void quitGame( int code )
{
    raster = cAllocatorFree(raster);
    for(int i = 0; i < s_width; i++) {
        raster2d[i] = cAllocatorFree(raster2d[i]);
    }
    raster2d = cAllocatorFree(raster2d);
    cEngineCleanup();
    input = cInputCleanup(input);
    infoTimer = cAllocatorFree(infoTimer);
    
    printf("quit game\n");
}


int sine_scroll = 0;
static void addTrackNodeWithOctave(int x, int y, int editing, int tone);
static void checkVisualCursorBounds();
static void checkPatternCursorBounds();
static bool checkScreenBounds(int x, int y);
static char *getWaveTypeAsChar(int type);
static void changeParam(bool plus);
static void ADSRInvertYRender(double x, double y, int color);

static void addTrackNodeWithOctave(int x, int y, int editing, int tone) {
    
    if(instrument_editor || pattern_editor) {
        // only allow preview of notes in editor
        cSynthAddTrackNode(x, y, false, 1, tone+(octave*12));
    } else {
        int base = 5;
        int diff = 0;
        int node_x = floor(visual_cursor_x/5);
        int x_count = visual_cursor_x%5;
        if(x_count == 0) {
            cSynthAddTrackNode(x, y, editing, 1, tone+(octave*12));
            if(editing) {
                visual_cursor_y++;
                checkVisualCursorBounds();
            }
        } else if(!editing) {
            cSynthAddTrackNode(x, y, false, 1, tone+(octave*12));
        }
        
        if(x_count == 1 && editing) {
            // change instrument
            printf("change instrument\n");
        }
        
        if(x_count == 2 && editing) {
            // change param1
            printf("change param1\n");
        }
        
        if(x_count == 3 && editing) {
            // change param2
            printf("change param2\n");
        }
        
        if(x_count == 4 && editing) {
            // change param1
            printf("change param3\n");
        }
    }
}

static void addParamToTrackNode() {
    
    
    
}


static void checkVisualCursorBounds() {
    
    struct CSynthContext *synth = cSynthGetContext();
    
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
        
        if(synth->current_track < synth->active_patterns-1) {
            synth->current_track++;
        } else {
            //rewind
            synth->current_track = 0;
        }
        
        visual_cursor_y = visual_track_height-1;
    }
}

static void checkPatternCursorBounds() {
    
    struct CSynthContext *synth = cSynthGetContext();
    
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


void handle_key_down( SDL_Keysym* keysym )
{
    struct CSynthContext *synth = cSynthGetContext();
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

        case SDLK_TAB:
            if(instrument_editor) {
                struct CInstrument *ins = synth->instruments[selected_instrument_id];
                selected_instrument_node_index++;
                if(selected_instrument_node_index >= ins->adsr_nodes) {
                    selected_instrument_node_index = 0;
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
            quit = true;
            break;
        case SDLK_RETURN:
            if(playing == 0) {
                playing = true;
                cSynthResetTrackProgress();
            } else {
                playing = 0;
            }
            break;
            
        case SDLK_LEFT:
            pressed_left = true;
            if(modifier == 1) {
                octave--;
                if(octave < 0) {
                    octave = 0;
                }
                setInfoTimerWithInt("octave", octave);
            } if(pattern_editor) {
                pattern_cursor_x--;
                checkPatternCursorBounds();
            } else {
                visual_cursor_x--;
                checkVisualCursorBounds();
            }
            break;
        case SDLK_RIGHT:
            pressed_right = true;
            if(modifier == 1) {
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
                if(pattern_editor == 1) {
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
                cSynthRemoveTrackNode(synth->track_cursor_x, synth->track_cursor_y);
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

static int getDelta() {
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

void audioCallback(void *unused, Uint8 *byteStream, int byteStreamLength) {
   
    memset(byteStream, 0, byteStreamLength);
    
    Sint16 *s_byteStream = (Sint16 *)byteStream;
    int remain = byteStreamLength / 2;
    
    struct CSynthContext *synth = cSynthGetContext();
    if(synth == NULL) {
        printf("audioCallback: synthContext is null, returning.");
        return;
    }
    
    
    
    for(int ins_i = 0; ins_i < synth->max_instruments; ins_i++) {
        struct CInstrument *ins = synth->instruments[ins_i];
        if(ins != NULL && ins->note_on == 1 && ins->voice != NULL) {
            Uint32 i;
            double d_sampleRate = synth->sample_rate;
            double d_waveformLength = ins->voice->waveform_length;
            double delta_phi = (double) (cSynthGetFrequency((double)ins->tone) / d_sampleRate * (double)d_waveformLength);
            for (i = 0; i < remain; i+=2) {
                if(ins->note_on == 1) {
                    
                    double amp = 0;
                    if(ins->noteoff_slope == 1) {
                        cSynthIncPhase(ins->voice, delta_phi);
                        amp = ins->noteoff_slope_value*ins->volume_scalar;
                        ins->noteoff_slope_value -= 0.001;
                        //printf("slope: %f\n", ins->noteoff_slope_value);
                        if(ins->noteoff_slope_value < 0) {
                            ins->noteoff_slope = false;
                            ins->noteoff_slope_value = 0;
                        }
                    } else {
                        cSynthIncPhase(ins->voice, delta_phi);
                        amp = cSynthInstrumentVolume(ins)*ins->volume_scalar;
                        ins->adsr_cursor += 0.00001;
                    }
                    s_byteStream[i] += ins->voice->waveform[ins->voice->phase_int]*amp;
                    s_byteStream[i+1] += ins->voice->waveform[ins->voice->phase_int]*amp;
                    
                    
                    /*
                    if(ins_i == 4) {
                        int16_t sample = ((rand()%INT16_MAX*2)-INT16_MAX)*amp;
                        s_byteStream[i] += sample;
                        s_byteStream[i+1] += sample;
                    } else {
                     */
                     
                    
                        //double cutoff = 0.099;
                        
                        /*
                        Sint16 lo_pass_output = s_byteStream[i] + (cutoff*(ins->voice->waveform[ins->voice->phase_int] - s_byteStream[i]));
                        s_byteStream[i] += lo_pass_output*amp;
                        
                        lo_pass_output = s_byteStream[i+1] + (cutoff*(ins->voice->waveform[ins->voice->phase_int] - s_byteStream[i+1]));
                        s_byteStream[i+1] += lo_pass_output*amp;
                        */
                        
                        /*
                        Sint16 hi_pass_output = ins->voice->waveform[ins->voice->phase_int] - (s_byteStream[i] + cutoff*(ins->voice->waveform[ins->voice->phase_int] - s_byteStream[i]));
                        s_byteStream[i] += hi_pass_output*amp;
                        
                        hi_pass_output = ins->voice->waveform[ins->voice->phase_int] - (s_byteStream[i+1] + cutoff*(ins->voice->waveform[ins->voice->phase_int] - s_byteStream[i+1]));
                        s_byteStream[i+1] += hi_pass_output*amp;
                        */
                    //}
                }
            }
        } 
    }
    
    if(playing == 1) {
        cSynthAdvanceTrack(remain);
    }
}

static void changeWaveform(int plus) {
    struct CSynthContext *synth = cSynthGetContext();
    
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
    
    struct CSynthContext *synth = cSynthGetContext();
    
    int x = pattern_cursor_x;
    int y = pattern_cursor_y;
    
    if(y == 0) {
        changeWaveform(plus);
    } else if(y == 17 || y == 18) {
        int ins_nr = x;
        // instruments
        if(y == 18) {
            ins_nr += 6;
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
        
    } else if(y == 19) {
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

static void renderInstrumentEditor() {
    struct CSynthContext *synth = cSynthGetContext();
    struct CInstrument *ins = synth->instruments[selected_instrument_id];
    int max_nodes = ins->adsr_nodes;
    int inset_x = 10;
    int inset_y = 50;
    double speed = 0.01;
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

static void renderPatternMapping() {
    struct CSynthContext *synth = cSynthGetContext();

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
                sprintf(cval, "Active patterns %d", synth->active_patterns);
                cEngineRenderLabelWithParams(raster2d, cval, x*10+inset_x, y+inset_y, cengine_color_white, bg_color);
            } else if(y == 19) {
                //nothing
            } else {
                if(y <= synth->active_patterns) {
                    bg_color = cengine_color_green;
                    if(x == pattern_cursor_x && y == pattern_cursor_y) {
                        bg_color = cengine_color_red;
                    }
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

static void drawWaveTypes() {
    struct CSynthContext *synth = cSynthGetContext();
    
    for (int x = 0; x < synth->patterns_and_voices_width; x++) {
        int val = synth->patterns_and_voices[x][0];
        cEngineRenderLabelWithParams(raster2d, getWaveTypeAsChar(val), 1+x*10, -visual_cursor_y+5, cengine_color_white, cengine_color_black);
    }
}

void renderTrack() {
    
    if(instrument_editor) {
        renderInstrumentEditor();
        return;
    }
    
    if(pattern_editor) {
        renderPatternMapping();
        return;
    }
    
    drawWaveTypes();
    
    struct CSynthContext *synth = cSynthGetContext();
    
    int x_count = 0;
    int offset_x = 0;
    int inset_x = 1;
    int inset_y = 6;
    
    
    int cursor_x = visual_cursor_x/5;
    cSynthUpdateTrackCursor(cursor_x, visual_cursor_y);
    int track_progress_int = synth->track_progress_int;
    
    if(playing == 0) {
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
            
            
            if(x == 0 || x == 5 || x == 10 || x == 15 || x == 20 || x == 25) {
                node_x = floor(x/5);
                int pattern = synth->patterns_and_voices[node_x][synth->current_track+1];
                
                if(synth->track[pattern][node_x][y] != NULL) {
                    int tone = synth->track[pattern][node_x][y]->tone;
                    char *ctone = cSynthToneToChar(tone);
                    cEngineRenderLabelWithParams(raster2d, ctone, inset_x+x+offset_x, inset_y+y-track_progress_int, cengine_color_white, bg_color);
                } else {
                    cEngineRenderLabelWithParams(raster2d, " - ", inset_x+x+offset_x, inset_y+y-track_progress_int, cengine_color_white, bg_color);
                }
                offset_x += 3;
            } else {
                
                if(x_count == 1) {
                    int pattern = synth->patterns_and_voices[node_x][synth->current_track+1];
                    if(synth->track[pattern][node_x][y] != NULL) {
                        char cval[20];
                        sprintf(cval, "%d", synth->track[pattern][node_x][y]->instrument_nr);
                        cEngineRenderLabelWithParams(raster2d, cval, inset_x+x+offset_x, inset_y+y-track_progress_int, cengine_color_white, bg_color);
                    } else {
                        cEngineRenderLabelWithParams(raster2d, "-", inset_x+x+offset_x, inset_y+y-track_progress_int, cengine_color_white, bg_color);
                    }
                } else {
                    cEngineRenderLabelWithParams(raster2d, "-", inset_x+x+offset_x, inset_y+y-track_progress_int, cengine_color_white, bg_color);
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

int main(int argc, char ** argv)
{
    SDL_Event event;
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window * window = SDL_CreateWindow("dtracker", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, 0);
    
    if (window != NULL) {
        SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, 0);
        if (renderer != NULL) {
            char * filename = "groundtiles.png";
            SDL_Surface * image = IMG_Load(filename);
            raw_sheet = image->pixels;
            setup_data();
            //SDL_FreeSurface(image);
            
            if (image != NULL) {
                
                SDL_Texture * texture = SDL_CreateTexture(renderer,
                                                          SDL_PIXELFORMAT_ARGB8888,
                                                          SDL_TEXTUREACCESS_STREAMING,
                                                          s_width, s_height);
                
                SDL_GL_SetSwapInterval(1);
                
                
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
                
                cEngineWritePixelData(raw_sheet);
                free(raw_sheet);
                
                struct CSynthContext *synth = cSynthContextNew();
                cSynthInit(synth);
                
                //if ( init() ) return 1;
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
                /***********************************************/
                
                if (texture != NULL) {
                    while (!quit) {
                        
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
                    }
                    
                    SDL_DestroyTexture(texture);
                }
            }
            printf("allocs before cleanup:\n");
            cAllocatorPrintAllocationCount();
            
            quitGame(0);
            
            printf("allocs after cleanup:\n");
            cAllocatorPrintAllocations();
            cAllocatorPrintAllocationCount();
            
            // If allocation tracking is on, clean up the last stuff.
            cAllocatorCleanup();
            
            SDL_DestroyWindow(window);
            SDL_CloseAudioDevice(AudioDevice);
            SDL_Quit();
        }
    }
    
    return 0;
}



