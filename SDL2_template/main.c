#include <SDL2/SDL.h>
#include <SDL2_image/SDL_image.h>
#include "Game.h"
#include "CInput.h"
#include "CTouch.h"
#include "CSynth.h"

int screen_width = 1280;
int screen_height = 720;
int fps_print_interval = 0;
int old_time = 0;

#define MAX_TOUCHES 8
#define sheet_width 1024
#define sheet_height 1024
#define fullscreen 0

struct CInput *input = NULL;


unsigned int *raster = NULL;
unsigned int **raster2d = NULL;
unsigned int **sheet = NULL;
unsigned int *raw_sheet = NULL;
int width = 256*4;
int height = 144*4;
int s_width = 256*4;
int s_height = 144*4;

static void setup_data()
{
    // contains an integer for every color/pixel on the screen.
    raster = (unsigned int *) malloc((s_width*s_height) * sizeof(unsigned int));
    int r = 0;
    for(r = 0; r < s_width*s_height; r++)
    {
        raster[r] = 0;
    }
    
    raster2d = malloc(s_width * sizeof(unsigned int *));
    if(raster2d == NULL)
    {
        fprintf(stderr, "out of memory\n");
    }
    for(int i = 0; i < s_width; i++)
    {
        raster2d[i] = malloc(s_height * sizeof(unsigned int));
        if(raster2d[i] == NULL)
        {
            fprintf(stderr, "out of memory\n");
        }
    }
    
    input = cInputNew();
    
    input->touches = malloc(MAX_TOUCHES * sizeof(struct CTouch*));
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
    
    input->ended_touches = malloc(MAX_TOUCHES * sizeof(struct CTouch*));
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
}

void convertInput(int x, int y)
{
    input->mouse_x = x/4;
    input->mouse_y = y/4;
}


int quit = 0;

static void quitGame( int code )
{
    free(raster);
    
    for(int i = 0; i < s_width; i++)
    {
        free(raster2d[i]);
    }
    free(raster2d);
    
    cEngineCleanup();
    free(input);
    
    printf("quit game");
}


int sine_scroll = 0;

void handle_key_down( SDL_Keysym* keysym )
{
    struct CSynthContext *synth = cSynthGetContext();
    switch( keysym->sym ) {
        case SDLK_ESCAPE:
            quit = 1;
            break;
        case SDLK_SPACE:
            break;
            
        case SDLK_LEFT:
            for(int i = 0; i < synth->max_instruments; i++) {
                if(synth->instruments[i] != NULL) {
                    synth->instruments[i]->tone -= 1;
                    
                }
            }
            sine_scroll-=50;
            if(sine_scroll < 0) {
                sine_scroll = 0;
            }
            break;
        case SDLK_RIGHT:
            for(int i = 0; i < synth->max_instruments; i++) {
                if(synth->instruments[i] != NULL) {
                    synth->instruments[i]->tone += 1;
                    
                }
            }
            sine_scroll+=50;
            if(sine_scroll < 0) {
                sine_scroll = 0;
            }
            break;
        case SDLK_1:
            if(synth->voices[0] != NULL) {
                if(synth->voices[0]->active == 0) {
                    synth->voices[0]->active = 1;
                    printf("voice 0 active");
                } else {
                    synth->voices[0]->active = 0;
                    printf("voice 0 inactive");
                }
            }
            break;
        case SDLK_2:
            if(synth->voices[1] != NULL) {
                if(synth->voices[1]->active == 0) {
                    synth->voices[1]->active = 1;
                    printf("voice 1 active");
                } else {
                    synth->voices[1]->active = 0;
                    printf("voice 1 inactive");
                }
            }
            break;
        case SDLK_3:
            if(synth->voices[2] != NULL) {
                if(synth->voices[2]->active == 0) {
                    synth->voices[2]->active = 1;
                    printf("voice 1 active");
                } else {
                    synth->voices[2]->active = 0;
                    printf("voice 1 inactive");
                }
            }
            break;
        case SDLK_4:
            if(synth->voices[3] != NULL) {
                if(synth->voices[3]->active == 0) {
                    synth->voices[3]->active = 1;
                    printf("voice 1 active");
                } else {
                    synth->voices[3]->active = 0;
                    printf("voice 1 inactive");
                }
            }
            break;
        case SDLK_a:
            if(synth->instruments[0] != NULL) {
                /*
                if(synth->instruments[0]->note_on == 0) {
                    synth->instruments[0]->note_on = 1;
                    printf("ins 1 note_on 1");
                } else {
                    synth->instruments[0]->note_on = 0;
                    printf("ins 1 note_on 0");
                }
                 */
                cSynthInstrumentNoteOn(synth->instruments[0]);
            }
            break;
            
        default:
            break;
    }
}

static void checkSDLEvents(SDL_Event event) {
    
    while (SDL_PollEvent(&event)) {
        switch(event.type) {
            case SDL_QUIT:
                quit = 1;
                break;
            case SDL_KEYDOWN:
                handle_key_down( &event.key.keysym );
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
                    input->mouse1 = 1;
                    input->touches[0]->active = 1;
                    input->touches[0]->x = event.motion.x/4;
                    input->touches[0]->y = event.motion.y/4;
                    printf("mouse 1 x:%i y:%i\n", input->touches[0]->x, input->touches[0]->y);
                    
                }
                
                if(event.button.button == SDL_BUTTON_RIGHT) {
                    input->mouse2 = 1;
                    input->touches[1]->active = 1;
                    input->touches[1]->x = event.motion.x;
                    input->touches[1]->y = event.motion.y;
                }
                
                break;
            case SDL_MOUSEBUTTONUP:
                //printf("Mouse button %d pressed at (%d,%d)\n",
                //           event.button.button, event.button.x, event.button.y);
                
                if(event.button.button == SDL_BUTTON_LEFT) {
                    input->mouse1 = 0;
                    input->touches[0]->active = 0;
                    input->ended_touches[0]->active = 1;
                    input->ended_touches[0]->x = event.motion.x;
                    input->ended_touches[0]->y = event.motion.y;
                }
                
                if(event.button.button == SDL_BUTTON_RIGHT) {
                    input->mouse2 = 0;
                    input->touches[1]->active = 0;
                    input->ended_touches[1]->active = 1;
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
    int fps = 0;
    
    if(old_time == 0) {
        old_time = currentTime;
        delta = 16;
    } else {
        delta = currentTime-old_time;
        old_time = currentTime;
        if(delta <= 0) {
            delta = 1;
        }
        fps = 1000/delta;
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

//Uint32 sampleRate = 22050;
//Uint32 frameRate =    60;

Uint32 floatStreamLength = 8;// must be a power of two, decrease to allow for a lower syncCompensationFactor to allow for lower latency, increase to reduce risk of underrun
Uint32 samplesPerFrame; // = sampleRate/frameRate;
Uint32 msPerFrame; // = 1000/frameRate;
double practicallySilent = 0.001;

Uint32 audioBufferLength = 2940;// must be a multiple of samplesPerFrame (auto adjusted upwards if not)

Sint8 *audioBuffer;

SDL_atomic_t audioCallbackLeftOff;
Sint32 audioMainLeftOff;
Sint8 audioMainAccumulator;

SDL_AudioDeviceID AudioDevice;
SDL_AudioSpec audioSpec;

SDL_Event event;
SDL_bool running = SDL_TRUE;

//Uint32 waveLength = 256;
//Uint32 sineWaveIndex = 0;
//double sineWaveIndexDouble = 0;



/********************/

/*
char *sineWave;
char *sawtoothWave;
char *squareWave;
char *triangleWave;
char *noise;
 */

/*
void speak(voice *v) {
    float sample;
    Uint32 sourceIndex;
    double phaseIncrement = v->frequency/sampleRate;
    Uint32 i;
    if (v->volume > practicallySilent) {
        for (i=0; (i+1)<samplesPerFrame; i+=2) {
            
            v->phase += phaseIncrement;
            if (v->phase > 1) v->phase -= 1;
            
            sourceIndex = v->phase*v->waveformLength;
            sample = v->waveform[sourceIndex]*v->volume;
            
            audioBuffer[audioMainLeftOff+i] += sample*(1-v->pan); //left channel
            audioBuffer[audioMainLeftOff+i+1] += sample*v->pan;   //right channel
        }
    }
    else {
        for (i=0; i<samplesPerFrame; i+=1)
            audioBuffer[audioMainLeftOff+i] = 0;
    }
    audioMainAccumulator++;
}*/

/*
double getFrequency(double pitch) {
    return pow(ChromaticRatio, pitch-57)*440;
}
int getWaveformLength(double pitch) {
    return sampleRate / getFrequency(pitch)+0.5f;
}
*/
 
//void buildSineWave(char *data, Uint32 length) {
//
//    float phaseIncrement = (2.0f * M_PI)/(float)waveLength;
//    float currentPhase = 0.0;
//    for (int i = 0; i < waveLength; i++) {
//        int sample = (int)(sin(currentPhase)*128);
//        data[i] = sample;
//        currentPhase += phaseIncrement;
//        printf("data %i:%i\n", i, data[i]);
//    }
//}
//
//void buildSawtoothWave(char *data, Uint32 length) {
//    float phaseIncrement = 256/(float)waveLength;
//    float currentPhase = 0.0;
//    for (int i = 0; i < waveLength; i++) {
//        Sint8 sample = 127-(int)currentPhase;
//        int i_s = sample;
//        data[i] = i_s;
//        currentPhase += phaseIncrement;
//        printf("sawtooth data %i:%i i_s:%i\n", i, data[i], i_s);
//    }
//}
//
//void buildSquareWave(char *data, Uint32 length) {
//    for (int i = 0; i < waveLength; i++) {
//        Sint8 sample = 127;
//        if(i > waveLength/2) {
//            sample = -127;
//        }
//        int i_s = sample;
//        data[i] = i_s;
//        printf("data %i:%i i_s:%i\n", i, data[i], i_s);
//    }
//}
//
//void buildTriangleWave(char *data, Uint32 length) {
//    
//    float phaseIncrement = (256/(float)waveLength)*2;
//    float currentPhase = -127.0;
//    
//    for (int i = 0; i < waveLength; i++) {
//        Sint8 sample = (int)currentPhase;
//        if(currentPhase > 127) {
//            sample = 127;
//        }
//        int i_s = sample;
//        data[i] = i_s;
//        if(i < waveLength/2) {
//            currentPhase += phaseIncrement;
//        } else {
//            currentPhase -= phaseIncrement;
//        }
//        printf("triangle data %i:%i i_s:%i\n", i, data[i], i_s);
//    }
//}
//
//void buildNoise(char *data, Uint32 length) {
//    for (int i = 0; i < waveLength; i++) {
//        int sample = (rand()%255)-127;
//        data[i] = sample;
//        printf("data %i:%i\n", i, data[i]);
//    }
//}

void logWavedata(float *floatStream, Uint32 floatStreamLength, Uint32 increment) {
    printf("\n\nwaveform data:\n\n");
    Uint32 i=0;
    for (i=0; i<floatStreamLength; i+=increment)
        printf("%4d:%2.16f\n", i, floatStream[i]);
    printf("\n\n");
}

//static void incPhase(struct Voice *v, double inc);

int testSchedule = 0;
int testScheduleSwitch = 0;

void audioCallback(void *unused, Uint8 *byteStream, int byteStreamLength) {
   
    for (int i = 0; i < byteStreamLength; i++) {
        byteStream[i] = 0;
    }
    
    Sint8 *s_byteStream = (Sint8*)byteStream;
    
    struct CSynthContext *synth = cSynthGetContext();
    
    for(int ins_i = 0; ins_i < synth->max_instruments; ins_i++) {
        struct CInstrument *ins = synth->instruments[ins_i];
        if(ins != NULL && ins->note_on == 1 && ins->voice != NULL) {
            Uint32 i;
            double d_sampleRate = synth->sample_rate;
            double d_waveformLength = ins->voice->waveform_length;
            double delta_phi = (double) (cSynthGetFrequency((double)ins->tone) / d_sampleRate * (double)d_waveformLength);
            for (i = 0; i < byteStreamLength; i++) {
                cSynthIncPhase(ins->voice, delta_phi);
                
                s_byteStream[i] += ins->voice->waveform[ins->voice->phase_int]*((double)ins->amplitude[ins->amplitude_index]/256.0)*0.2;
                //s_byteStream[i] += ins->voice->waveform[ins->voice->phase_int]*0.6;
                
                //ins->voice->volume;
                ins->amplitude_index_double += (double)byteStreamLength*0.001;
                ins->amplitude_index = (int) ins->amplitude_index_double;
                if(ins->amplitude_index >= ins->amplitude_length) {
                    ins->amplitude_index = 0;
                    ins->amplitude_index_double = 0;
                    ins->note_on = 0;
                }
            }
        } 
    }
}



int onExit() {
    SDL_CloseAudioDevice(AudioDevice);
    free(audioBuffer);//not necessary?
    //SDL_Quit();
    return 0;
}


int main(int argc, char ** argv)
{
    SDL_Event event;
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window * window = SDL_CreateWindow("synth", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, 0);
    
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
                c->width = 256;
                c->height = 144;
                c->sprite_size = 16;
                c->sheet_size = 1024;
                c->max_touches = 8;
                c->level_width = 64;
                c->level_height = 64;
                c->max_buttons = 10;
                c->show_fps = 0;
                c->ground_render_enabled = 0;
                
                cEngineInit(c);
                
                cEngineWritePixelData(raw_sheet);
                free(raw_sheet);
                
                struct CSynthContext *synth = cSynthContextNew();
                cSynthInit(synth);
                
                //if ( init() ) return 1;
                SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER);
                SDL_AudioSpec want;
                SDL_zero(want);// btw, I have no idea what this is...
                
                
                want.freq = synth->sample_rate;
                want.format = AUDIO_S8;
                want.channels = 1;
                want.samples = floatStreamLength;
                want.callback = audioCallback;
                
                AudioDevice = SDL_OpenAudioDevice(NULL, 0, &want, &audioSpec, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
                
                if (AudioDevice == 0) {
                    printf("\nFailed to open audio: %s\n", SDL_GetError());
                    return 1;
                }
                
                
                if (audioSpec.format != want.format) {
                    printf("\nCouldn't get Float32 audio format.\n");
                    return 2;
                }
                
                int sampleRate = synth->sample_rate;
                int frameRate = synth->frame_rate;
                
                sampleRate = audioSpec.freq;
                floatStreamLength = audioSpec.size/4;
                samplesPerFrame = sampleRate/frameRate;
                msPerFrame = 1000/frameRate;
                audioMainLeftOff = samplesPerFrame*8;
                SDL_AtomicSet(&audioCallbackLeftOff, 0);
                
                /*
                if (audioBufferLength % samplesPerFrame) {
                    audioBufferLength += samplesPerFrame-(audioBufferLength % samplesPerFrame);
                }
                */
                
                audioBuffer = malloc( sizeof(Sint8)*audioBufferLength );
                
                
                SDL_Delay(42);// let the tubes warm up
                
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
                        
                        cEngineGameloop(getDelta(), raster2d, input);
                        //renderStuff
                        
                        for(int x = 0; x < s_width; x++) {
                            for(int y = 0; y < s_height; y++) {
                                raster2d[x][y] = 0;
                            }
                        }
                        
                        /*
                        for(int i = sine_scroll; i < waveLength; i++) {
                            int pos = sineWave[i]+150;
                            if(pos > -1 && pos < s_height
                               && i-sine_scroll < s_width) {
                                raster2d[i-sine_scroll][pos] = 0xffffffff;
                            }
                        }
                        
                        for(int i = sine_scroll; i < waveLength; i++) {
                            int pos = squareWave[i]+150;
                            if(pos > -1 && pos < s_height
                               && i-sine_scroll < s_width) {
                                raster2d[i-sine_scroll][pos] = 0xff00ffff;
                            }
                        }
                        
                        for(int i = sine_scroll; i < waveLength; i++) {
                            int pos = triangleWave[i]+150;
                            if(pos > -1 && pos < s_height
                               && i-sine_scroll < s_width) {
                                raster2d[i-sine_scroll][pos] = 0xffff00ff;
                            }
                        }
                        
                        for(int i = sine_scroll; i < waveLength; i++) {
                            int pos = sawtoothWave[i]+150;
                            if(pos > -1 && pos < s_height
                               && i-sine_scroll < s_width) {
                                raster2d[i-sine_scroll][pos] = 0xffffff00;
                            }
                        }*/
                        
                        SDL_UpdateTexture(texture, NULL, raster, s_width * sizeof (Uint32));
                        SDL_RenderClear(renderer);
                        
                        
                        SDL_RenderCopy(renderer, texture, NULL, NULL);
                        SDL_RenderPresent(renderer);
                    }
                    
                    SDL_DestroyTexture(texture);
                }
            }
            
            quitGame(0);
            SDL_DestroyWindow(window);
            onExit();
            SDL_Quit();
        }
    }
    
    return 0;
}



