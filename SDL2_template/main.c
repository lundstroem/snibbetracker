#include <SDL2/SDL.h>
#include <SDL2_image/SDL_image.h>
#include "Game.h"
#include "CInput.h"
#include "CTouch.h"

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
int width = 144*4;
int height = 256*4;
int s_width = 144;
int s_height = 256;

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
    
    //magic number
    /*
     int offset_compensation = 6;
     
     float h_ = screen_width*0.5625f;
     int h = (int)h_;
     double f_screen_width = screen_width;
     double f_screen_height = screen_height;
     double f_v_screen_height = h;
     double offset = ((f_screen_height-h)/2)*(s_height/f_screen_height);
     double factor_x = x / f_screen_width;
     double factor_y = y / f_v_screen_height;
     double d_r_x = factor_x * s_width;
     double d_r_y = factor_y * s_height;
     int r_x = (int)d_r_x;
     int r_y = (int)d_r_y-offset-offset_compensation;
     int index = r_x+r_y*s_width;
     
     
     if(index < (s_width*s_height) && index > -1)
     {
     raster[r_x+r_y*s_width] = 0xffff00ff;
     input->mouse_x = r_x;
     input->mouse_y = r_y;
     }
     */
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

static void handle_key_down( SDL_Keysym* keysym )
{
    switch( keysym->sym ) {
        case SDLK_ESCAPE:
            quit = 1;
            break;
        case SDLK_SPACE:
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
const double ChromaticRatio = 1.059463094359295264562;
const double Tao = 6.283185307179586476925;

Uint32 sampleRate = 48000;
Uint32  frameRate =    60;
Uint32 floatStreamLength = 8;// must be a power of two, decrease to allow for a lower syncCompensationFactor to allow for lower latency, increase to reduce risk of underrun
Uint32 samplesPerFrame; // = sampleRate/frameRate;
Uint32 msPerFrame; // = 1000/frameRate;
double practicallySilent = 0.001;

Uint32 audioBufferLength = 48000;// must be a multiple of samplesPerFrame (auto adjusted upwards if not)
float *audioBuffer;

SDL_atomic_t audioCallbackLeftOff;
Sint32 audioMainLeftOff;
Uint8 audioMainAccumulator;

SDL_AudioDeviceID AudioDevice;
SDL_AudioSpec audioSpec;

SDL_Event event;
SDL_bool running = SDL_TRUE;

typedef struct {
    float *waveform;
    Uint32 waveformLength;
    double volume;        // multiplied
    double pan;           // 0 to 1: all the way left to all the way right
    double frequency;     // Hz
    double phase;         // 0 to 1
} voice;

/********************/


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
}

double getFrequency(double pitch) {
    return pow(ChromaticRatio, pitch-57)*440;
}
int getWaveformLength(double pitch) {
    return sampleRate / getFrequency(pitch)+0.5f;
}

void buildSineWave(float *data, Uint32 length) {
    Uint32 i;
    for (i=0; i < length; i++)
        data[i] = sin( i*(Tao/length) );
}

void logSpec(SDL_AudioSpec *as) {
    printf(
           " freq______%5d\n"
           " format____%5d\n"
           " channels__%5d\n"
           " silence___%5d\n"
           " samples___%5d\n"
           " size______%5d\n\n",
           (int) as->freq,
           (int) as->format,
           (int) as->channels,
           (int) as->silence,
           (int) as->samples,
           (int) as->size
           );
}

void logVoice(voice *v) {
    printf(
           " waveformLength__%d\n"
           " volume__________%f\n"
           " pan_____________%f\n"
           " frequency_______%f\n"
           " phase___________%f\n",
           v->waveformLength,
           v->volume,
           v->pan,
           v->frequency,
           v->phase
           );
}

void logWavedata(float *floatStream, Uint32 floatStreamLength, Uint32 increment) {
    printf("\n\nwaveform data:\n\n");
    Uint32 i=0;
    for (i=0; i<floatStreamLength; i+=increment)
        printf("%4d:%2.16f\n", i, floatStream[i]);
    printf("\n\n");
}

void audioCallback(void *unused, Uint8 *byteStream, int byteStreamLength) {
    float* floatStream = (float*) byteStream;
    
    Sint32 localAudioCallbackLeftOff = SDL_AtomicGet(&audioCallbackLeftOff);
    
    Uint32 i;
    for (i=0; i<floatStreamLength; i++) {
        
        floatStream[i] = audioBuffer[localAudioCallbackLeftOff];
        
        localAudioCallbackLeftOff++;
        if ( localAudioCallbackLeftOff == audioBufferLength )
            localAudioCallbackLeftOff = 0;
    }
    //printf("localAudioCallbackLeftOff__%5d\n", localAudioCallbackLeftOff);
    
    SDL_AtomicSet(&audioCallbackLeftOff, localAudioCallbackLeftOff);
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
                c->width = 144;
                c->height = 256;
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
                
                
                /*** audio init **/
                float  syncCompensationFactor = 0.0016;// decrease to reduce risk of collision, increase to lower latency
                Sint32 mainAudioLead;
                Uint32 i;
                
                voice testVoiceA;
                voice testVoiceB;
                voice testVoiceC;
                testVoiceA.volume = 0.5;
                testVoiceB.volume = 0.5;
                testVoiceC.volume = 0.5;
                testVoiceA.pan = 0.5;
                testVoiceB.pan = 0;
                testVoiceC.pan = 1;
                testVoiceA.phase = 0;
                testVoiceB.phase = 0;
                testVoiceC.phase = 0;
                testVoiceA.frequency = getFrequency(48);// A3
                testVoiceB.frequency = getFrequency(52);// C#4
                testVoiceC.frequency = getFrequency(55);// E4
                Uint16 C0waveformLength = getWaveformLength(0);
                testVoiceA.waveformLength = C0waveformLength;
                testVoiceB.waveformLength = C0waveformLength;
                testVoiceC.waveformLength = C0waveformLength;
                float sineWave[C0waveformLength];
                buildSineWave(sineWave, C0waveformLength);
                testVoiceA.waveform = sineWave;
                testVoiceB.waveform = sineWave;
                testVoiceC.waveform = sineWave;
                
                logVoice(&testVoiceA);
                logWavedata(testVoiceA.waveform, testVoiceA.waveformLength, 10);
                
                //if ( init() ) return 1;
                SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER);
                SDL_AudioSpec want;
                SDL_zero(want);// btw, I have no idea what this is...
                
                
                want.freq = sampleRate;
                want.format = AUDIO_F32;
                want.channels = 2;
                want.samples = floatStreamLength;
                want.callback = audioCallback;
                
                AudioDevice = SDL_OpenAudioDevice(NULL, 0, &want, &audioSpec, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
                
                if (AudioDevice == 0) {
                    printf("\nFailed to open audio: %s\n", SDL_GetError());
                    return 1;
                }
                
                printf("want:\n");
                logSpec(&want);
                printf("audioSpec:\n");
                logSpec(&audioSpec);
                
                if (audioSpec.format != want.format) {
                    printf("\nCouldn't get Float32 audio format.\n");
                    return 2;
                }
                
                sampleRate = audioSpec.freq;
                floatStreamLength = audioSpec.size/4;
                samplesPerFrame = sampleRate/frameRate;
                msPerFrame = 1000/frameRate;
                audioMainLeftOff = samplesPerFrame*8;
                SDL_AtomicSet(&audioCallbackLeftOff, 0);
                
                if (audioBufferLength % samplesPerFrame) {
                    audioBufferLength += samplesPerFrame-(audioBufferLength % samplesPerFrame);
                }
                audioBuffer = malloc( sizeof(float)*audioBufferLength );
                
                
                
                
                
                SDL_Delay(42);// let the tubes warm up
                
                SDL_PauseAudioDevice(AudioDevice, 0);// unpause audio.
                /***********************************************/
                
                if (texture != NULL) {
                    while (!quit) {
                        //printf("checkSDLEvents\n");
                        
                        
                        
                        /**** audio update ****/
                        
                        for (int i=0; i<samplesPerFrame; i++) {
                            audioBuffer[audioMainLeftOff+i] = 0;
                        }
                        
                        
                        //printf("audioMainLeftOff___________%5d\n", audioMainLeftOff);
                        
                        speak(&testVoiceA);
                        speak(&testVoiceB);
                        speak(&testVoiceC);
                        
                        if (audioMainAccumulator > 1) {
                            for (i=0; i<samplesPerFrame; i++) {
                                audioBuffer[audioMainLeftOff+i] /= audioMainAccumulator;
                            }
                        }
                        audioMainAccumulator = 0;
                        
                        audioMainLeftOff += samplesPerFrame;
                        if (audioMainLeftOff == audioBufferLength) {
                            audioMainLeftOff = 0;
                        }
                        
                        mainAudioLead = audioMainLeftOff - SDL_AtomicGet(&audioCallbackLeftOff);
                        
                        if (mainAudioLead < 0) {
                            mainAudioLead += audioBufferLength;
                        }
                        
                        //printf("mainAudioLead:%5d\n", mainAudioLead);
                        if (mainAudioLead < floatStreamLength) {
                            printf("An audio collision may have occured!\n");
                        }
                        
                        // SDL_Delay( mainAudioLead*syncCompensationFactor );
                        
                        /*********/
                        
                        
                        
                        
                        
                        checkSDLEvents(event);
                        
                        for (int r_x = 0; r_x < s_width; r_x++) {
                            for (int r_y = 0; r_y < s_height; r_y++) {
                                raster[r_x+r_y*s_width] = raster2d[r_x][r_y];
                            }
                        }
                        
                        cEngineGameloop(getDelta(), raster2d, input);
                        
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



