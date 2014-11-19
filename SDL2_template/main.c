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

double toneFrequency[116] = {
                     16.35,
                     17.32,
                     18.35,
                     19.45,
                     20.60,
                     21.83,
                     23.12,
                     24.50,
                     25.96,
                     27.50,
                     29.14,
                     30.87,
                     32.70,
                     34.65,
                     36.71,
                     38.89,
                     41.20,
                     43.65,
                     46.25,
                     49.00,
                     51.91,
                     55.00,
                     58.27,
                     61.74,
                     65.41,
                     69.30,
                     73.42,
                     77.78,
                     82.41,
                     87.31,
                     92.50,
                     98.00,
                     103.83,
                     110.00,
                     116.54,
                     123.47,
                     130.81,
                     138.59,
                     146.83,
                     155.56,
                     164.81,
                     174.61,
                     185.00,
                     196.00,
                     207.65,
                     220.00,
                     233.08,
                     246.94,
                     261.63,
                     277.18,
                     293.66,
                     311.13,
                     329.63,
                     349.23,
                     369.99,
                     392.00,
                     415.30,
                     440.00,
                     466.16,
                     493.88,
                     523.25,
                     554.37,
                     587.33,
                     622.25,
                     659.26,
                     698.46,
                     739.99,
                     783.99,
                     830.61,
                     880.00,
                     932.33,
                     987.77,
                     1046.50,
                     1108.73,
                     1174.66,
                     1244.51,
                     1318.51,
                     1396.91,
                     1479.98,
                     1567.98,
                     1661.22,
                     1760.00,
                     1864.66,
                     1975.53,
                     2093.00,
                     2217.46,
                     2349.32,
                     2489.02,
                     2637.02,
                     2793.83,
                     2959.96,
                     3135.96,
                     3322.44,
                     3520.00,
                     3729.31,
                     3951.07,
                     4186.01,
                     4434.92,
                     4698.64,
                     4978.03,
                     5274.04,
                     5587.65,
                     5919.91,
                     6271.93,
                     6644.88,
                     7040.00,
                     7458.62,
                     7902.13,
                     8372.01,
                     8869.84,
                     9397.27,
                     9956.06,
                     10548.08,
                     11175.30,
                     11839.82,
                     12543.85
                     };


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

static double sineWaveIndexInc = 7.001;
static int playingFreq = 20;

static void handle_key_down( SDL_Keysym* keysym )
{
    switch( keysym->sym ) {
        case SDLK_ESCAPE:
            quit = 1;
            break;
        case SDLK_SPACE:
            break;
        case SDLK_LEFT:
            playingFreq -= 1;
            printf("%i\n", playingFreq);
            break;
        case SDLK_RIGHT:
            playingFreq += 1;
            if(playingFreq > 115) {
                playingFreq = 115;
            }
            printf("%i\n", playingFreq);
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

Uint32 sampleRate = 22050;
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

//Uint8 *sineWave = NULL;
Uint32 sineWaveLength = 2048;
Uint32 sineWaveIndex = 0;
double sineWaveIndexDouble = 0;

struct Voice {
    Uint8 *waveform;
    Uint32 waveformLength;
    double volume;        // multiplied
    //double pan;           // 0 to 1: all the way left to all the way right
    double frequency;     // Hz
    Uint32 tone;     // tone
    double phase;         // 0 to 1
};

struct Voice *voices = NULL;
int MAX_VOICES = 8;

/********************/
/*
Uint8 sineWave [256] = {
    128,131,134,137,140,143,146,149,152,156,159,162,165,168,171,174,
    176,179,182,185,188,191,193,196,199,201,204,206,209,211,213,216,
    218,220,222,224,226,228,230,232,234,236,237,239,240,242,243,245,
    246,247,248,249,250,251,252,252,253,254,254,255,255,255,255,255,
    255,255,255,255,255,255,254,254,253,252,252,251,250,249,248,247,
    246,245,243,242,240,239,237,236,234,232,230,228,226,224,222,220,
    218,216,213,211,209,206,204,201,199,196,193,191,188,185,182,179,
    176,174,171,168,165,162,159,156,152,149,146,143,140,137,134,131,
    128,124,121,118,115,112,109,106,103,99, 96, 93, 90, 87, 84, 81,
    79, 76, 73, 70, 67, 64, 62, 59, 56, 54, 51, 49, 46, 44, 42, 39,
    37, 35, 33, 31, 29, 27, 25, 23, 21, 19, 18, 16, 15, 13, 12, 10,
    9,  8,  7,  6,  5,  4,  3,  3,  2,  1,  1,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  1,  1,  2,  3,  3,  4,  5,  6,  7,  8,
    9,  10, 12, 13, 15, 16, 18, 19, 21, 23, 25, 27, 29, 31, 33, 35,
    37, 39, 42, 44, 46, 49, 51, 54, 56, 59, 62, 64, 67, 70, 73, 76,
    79, 81, 84, 87, 90, 93, 96, 99, 103,106,109,112,115,118,121,124
};
*/

Uint8 *sineWave;

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

double getFrequency(double pitch) {
    return pow(ChromaticRatio, pitch-57)*440;
}
int getWaveformLength(double pitch) {
    return sampleRate / getFrequency(pitch)+0.5f;
}

void buildSineWave(Uint8 *data, Uint32 length) {
    /*
    Uint32 i;
    for (i=0; i < length; i++) {
        data[i] = sin( i*(Tao/length) )*256;
        printf("%i\n", data[i]);
    }*/
    for (int i = 0; i < sineWaveLength; ++i)
    {
        Uint8 sample = (Uint8)roundf(255 * sinf(2.0f * M_PI * (float)i / sineWaveLength));
        data[i] = sample;
        printf("data %i:%i\n", i, data[i]);
    }
}



void logWavedata(float *floatStream, Uint32 floatStreamLength, Uint32 increment) {
    printf("\n\nwaveform data:\n\n");
    Uint32 i=0;
    for (i=0; i<floatStreamLength; i+=increment)
        printf("%4d:%2.16f\n", i, floatStream[i]);
    printf("\n\n");
}

void audioCallback(void *unused, Uint8 *byteStream, int byteStreamLength) {
    
    double delta_phi = (double) toneFrequency[playingFreq] / sampleRate * sineWaveLength;
    sineWaveIndexInc = delta_phi;
    
    // phase increment
    
    Sint32 localAudioCallbackLeftOff = SDL_AtomicGet(&audioCallbackLeftOff);
    
    Uint32 i;
    for (i=0; i<byteStreamLength; i++) {
        
        byteStream[i] = sineWave[sineWaveIndex]/4;
        sineWaveIndexDouble += sineWaveIndexInc;
        sineWaveIndex = (Uint32)sineWaveIndexDouble;
        if(sineWaveIndex >= sineWaveLength) {
            sineWaveIndex = 0;
            sineWaveIndexDouble = 0;
        }
        
    }
    
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
                
                
                
                //if ( init() ) return 1;
                SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER);
                SDL_AudioSpec want;
                SDL_zero(want);// btw, I have no idea what this is...
                
                
                want.freq = sampleRate;
                want.format = AUDIO_U8;
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
                
                
                sineWave = malloc( sizeof(Uint8)*sineWaveLength );
                buildSineWave(sineWave, sineWaveLength);
                
                /*
                voices = malloc(sizeof(struct Voice*)*MAX_VOICES);
                
                struct Voice *voice = malloc(sizeof(struct Voice*));
                voice->tone = 40;
                voice->waveform = sineWave;
                voice->waveformLength = sineWaveLength;
                voice->phase = 0;
                 */
                
                //voices[0]
                
                SDL_Delay(42);// let the tubes warm up
                
                SDL_PauseAudioDevice(AudioDevice, 0);// unpause audio.
                /***********************************************/
                
                if (texture != NULL) {
                    while (!quit) {
                        //printf("checkSDLEvents\n");
                        
                        
                        
          
                        
                        
                        
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



