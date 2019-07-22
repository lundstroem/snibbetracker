/*
    CSynth.c
    synth
 
    Created by Harry Lundstrom on 20/11/14.
    Copyright (c) 2014 D. All rights reserved.
*/

#include "CSynth.h"
#include "CAllocator.h"
#include "CEngine.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include "CNoiseTable.h"

const int build_number = 24;
const int file_version = 4;
const double ChromaticRatio = 1.059463094359295264562;
static double cSynthInstrumentVolumeByBaseNode(struct CInstrument *i, int base_node, double cursor);
static void cSynthInitData(struct CSynthContext *synth);
static void cSynthBuildSineWave(int16_t *data, int wave_length);
static void cSynthBuildSawtoothWave(int16_t *data, int wave_length);
static void cSynthBuildSquareWave(int16_t *data, int wave_length);
static void cSynthBuildTriangleWave(int16_t *data, int wave_length);
static void cSynthBuildNoise(int16_t *data, int wave_length);
static void cSynthLookAheadForActivatingSlope(struct CSynthContext *synth);
static void cSynthVoiceApplyVibrato(struct CSynthContext *synth, struct CVoice *v);
static void cSynthVoiceApplyPitchUpDown(struct CSynthContext *synth, struct CVoice *v);
static void cSynthVoiceApplyDetune(struct CSynthContext *synth, struct CVoice *v);
static void cSynthVoiceApplyArpeggio(struct CSynthContext *synth, struct CVoice *v);
static void cSynthVoiceApplyPortamento(struct CSynthContext *synth, struct CVoice *v);
static void cSynthUpdateWavetable(struct CSynthContext *synth, struct CVoice *v);
void cSynthResetAllEffects(struct CVoice *v);
static void cSynthBitcrush(struct CSynthContext *synth, int16_t *s_byteStream, long begin, long length);
static struct CVoice *cSynthNewVoice(struct CSynthContext *synth, int16_t *waveform);
static void cSynthResetVoice(struct CSynthContext *synth, int16_t *waveform, struct CVoice *v);
void cSynthInstrumentResetEnvelope(struct CInstrument *i);
static struct CInstrument *cSynthNewInstrument(struct CSynthContext *synth);
static void cSynthLoadFailed(char *reason, cJSON *json);
static struct CTempoNode *cSynthNewTempoNode(void);
static struct CWavetableNode *cSynthNewWavetableNode(void);
static void cSynthSetWaveformsFromPattern(struct CSynthContext *synth);
static void cSynthChangeWaveformForChannel(struct CSynthContext *synth, int channel, int waveform);
static void cSynthPostEffectFix(struct CSynthContext *synth, struct CVoice *v, struct CTrackNode *t);
static void cSynthVoiceSetEffect(struct CSynthContext *synth, struct CVoice *v, struct CTrackNode *t, int previous_tone);
static void cSynthUpdateTicksModifier(struct CSynthContext *synth);

void cSynthInit(struct CSynthContext *synth) {
    
    cSynthInitData(synth);
}

struct CSynthContext *cSynthContextNew() {

    struct CSynthContext *s = cAllocatorAlloc(sizeof(struct CSynthContext), "CSynthContext");
    s->debuglog = false;
    s->errorlog = true;
    s->file_version = file_version;
    s->build_number = build_number;
    s->str_title = NULL;
    s->str_author = NULL;
    s->str_info = NULL;
    s->sample_rate = 44100;
    s->frame_rate = 60;
    s->chunk_size = 0;
    s->master_amp = 0.5;
    s->master_amp_percent = 50;
    s->default_bpm = 120;
    s->audio_clips = false;
    s->preview_enabled = true;
    s->sustain_active = false;
    s->preview_locked = false;
    s->preview_started = false;
    s->interleaved = true;
    s->voices = NULL;
    s->instruments = NULL;
    s->custom_instrument = NULL;
    s->max_voices = 6;
    s->max_instruments = 16;
    s->current_instrument = 0;
    s->active_tracks = NULL;
    s->solo_track = -1;
    s->solo_voice = -1;
    s->wave_length = 1024;
    s->noise_length = 44100;
    s->looped = false;
    s->track = NULL;
    s->swap_track = NULL;
    s->copy_track = NULL;
    s->instrument_effects = NULL;
    s->max_instrument_effects = 16;
    s->max_tracks_and_patterns = 32;
    s->track_width = 6;
    s->track_height = 16;
    s->track_max_height = 256;
    s->track_progress = 0;
    s->track_progress_int = 0;
    s->track_cursor_x = 0;
    s->track_cursor_y = 0;
    s->track_advance = true;
    s->current_track = 0;
    s->patterns_and_voices = NULL;
    s->patterns_and_voices_width = 6;
    s->patterns_and_voices_height = 22;
    s->patterns = NULL;
    s->patterns_width = 6;
    s->patterns_height = 64;
    s->max_adsr_nodes = 5;
    s->arpeggio_speed = 10;
    s->temp_mixdown_buffer = NULL;
    s->temp_mixdown_size = 64;
    s->sine_wave_table = NULL;
    s->sawtooth_wave_table = NULL;
    s->square_wave_table = NULL;
    s->triangle_wave_table = NULL;
    s->noise_table = NULL;
    s->custom_table = NULL;
    s->needs_redraw = true;
    s->base_mod_adsr_cursor = 0.441000;
    s->base_mod_noteoff_slope = 2.2;
    s->base_mod_advance_amp_targets = 2.2;
    s->base_mod_vibrato_speed = 0.689063;
    s->base_mod_vibrato_depth = 0.003445;
    s->base_mod_pitch_1 = 13.781250;
    s->base_mod_pitch_2 = 0.17813;
    s->base_mod_arp_speed = 0.013877;
    s->base_mod_portamento_speed = 0.037223;
    s->base_mod_pwm_speed = 441.000000;
    s->base_mod_detune = 630.781250;
    s->mod_noteoff_slope = 0;
    s->mod_adsr_cursor = 0;
    s->mod_advance_amp_targets = 0;
    s->mod_vibrato_speed = 0;
    s->mod_vibrato_depth = 0;
    s->mod_pitch_1 = 0;
    s->mod_pitch_2 = 0;
    s->mod_arp_speed = 0;
    s->mod_portamento_speed = 0;
    s->mod_pwm_speed = 0;
    s->track_highlight_interval = 4;
    s->tempo_width = 6;
    s->tempo_height = 17;
    s->tempo_map = NULL;
    s->wavetable_width = 12;
    s->wavetable_height = 18;
    s->wavetable_map = NULL;
    s->current_tempo_column = 0;
    s->tempo_index = 1;
    s->pending_tempo_column = -1;
    s->pending_tempo_blink_counter = 0;
    s->ticks_modifier = -1;
    s->pending_tempo_blink_counter_toggle = false;
    s->tempo_skip_step = false;
    s->bitcrush_active = false;
    s->bitcrush_depth = 0;
    return s;
}

cJSON* cSynthSaveProject(struct CSynthContext *synth) {
    
    char *node_build_number = NULL;
    char *node_file_version = NULL;
    char *node_author = NULL;
    char *node_title = NULL;
    char *node_info = NULL;
    char *node_master_amp_percent = NULL;
    char *node_solo_voice = NULL;
    char *node_solo_track = NULL;
    char *node_active_tracks = NULL;
    char *node_muted_voices = NULL;
    char *node_track_height = NULL;
    char *node_arp = NULL;
    char *node_pattern = NULL;
    char *node_column = NULL;
    char *node_row = NULL;
    char *node_tone = NULL;
    char *node_tone_active = NULL;
    char *node_instrument = NULL;
    char *node_effect = NULL;
    char *node_effect_value = NULL;
    char *node_effect_param_1 = NULL;
    char *node_effect_param_1_value = NULL;
    char *node_effect_param_2 = NULL;
    char *node_effect_param_2_value = NULL;
    char *node_nodes = NULL;
    char *node_patterns = NULL;
    char *node_patterns_and_voices = NULL;
    char *node_instruments_adsr = NULL;
    char *node_amp = NULL;
    char *node_pos = NULL;
    char *node_instrument_effects = NULL;
    char *node_instrument_link = NULL;
    char *node_track_highlight_interval = NULL;
    char *node_tempo = NULL;
    char *node_tempo_active_column = NULL;
    char *node_tempo_bpm = NULL;
    char *node_tempo_ticks = NULL;
    char *node_tempo_active = NULL;
    char *node_tempo_x = NULL;
    char *node_tempo_y = NULL;
    char *node_wavetable = NULL;
    char *node_wavetable_speed = NULL;
    char *node_wavetable_value = NULL;
    char *node_wavetable_active = NULL;
    char *node_wavetable_x = NULL;
    char *node_wavetable_y = NULL;
    char *node_custom_table = NULL;
    char *node_custom_instrument = NULL;
    
    cJSON *root = NULL;
    cJSON *node = NULL;
    cJSON *instrument = NULL;
    cJSON *adsr_node = NULL;
    cJSON *array = NULL;
    cJSON *tempo_node = NULL;
    cJSON *wavetable_node = NULL;
    cJSON *custom_table_node = NULL;
    
    int a;
    int s_x;
    int s_y;
    int i;
    
    root = cJSON_CreateObject();

    node_build_number = "build";
    node_file_version = "file_version";
    node_author = "author";
    node_title = "title";
    node_info = "info";
    node_master_amp_percent = "master_amp_percent";
    node_solo_voice = "solo_voice";
    node_solo_track = "solo_track";
    node_active_tracks = "active_tracks";
    node_muted_voices = "muted_voices";
    node_track_height = "track_height";
    node_arp = "arp";
    node_pattern = "p";
    node_column = "c";
    node_row = "r";
    node_tone = "n";
    node_tone_active = "t";
    node_instrument = "i";
    node_effect = "e";
    node_effect_value = "f";
    node_effect_param_1 = "g";
    node_effect_param_1_value = "h";
    node_effect_param_2 = "j";
    node_effect_param_2_value = "k";
    node_nodes = "nodes";
    node_patterns = "patterns";
    node_patterns_and_voices = "patterns_and_voices";
    node_instruments_adsr = "instruments_adsr";
    node_amp = "a";
    node_pos = "p";
    node_instrument_effects = "instrument_effects";
    node_instrument_link = "i_l";
    node_track_highlight_interval = "track_highlight_interval";
    node_tempo = "tempo";
    node_tempo_active_column = "active_tempo_column";
    node_tempo_bpm = "b";
    node_tempo_ticks = "t";
    node_tempo_active = "a";
    node_tempo_x = "x";
    node_tempo_y = "y";
    node_wavetable = "wavetable";
    node_wavetable_speed = "s";
    node_wavetable_value = "v";
    node_wavetable_active = "a";
    node_wavetable_x = "x";
    node_wavetable_y = "y";
    node_custom_table = "custom_table";
    node_custom_instrument = "i";
    
    cJSON_AddNumberToObject(root, node_build_number, synth->build_number);
    cJSON_AddNumberToObject(root, node_file_version, synth->file_version);
    
    if (synth->str_author != NULL) {
        cJSON_AddStringToObject(root, node_author, synth->str_author->chars);
    }
    if (synth->str_title != NULL) {
        cJSON_AddStringToObject(root, node_title, synth->str_title->chars);
    }
    if (synth->str_info != NULL) {
        cJSON_AddStringToObject(root, node_info, synth->str_info->chars);
    }
    
    cJSON_AddNumberToObject(root, node_master_amp_percent, synth->master_amp_percent);
    cJSON_AddNumberToObject(root, node_solo_voice, synth->solo_voice);
    cJSON_AddNumberToObject(root, node_solo_track, synth->solo_track);
    cJSON_AddNumberToObject(root, node_track_height, synth->track_height);
    cJSON_AddNumberToObject(root, node_arp, synth->arpeggio_speed);
    cJSON_AddNumberToObject(root, node_track_highlight_interval, synth->track_highlight_interval);
    cJSON_AddNumberToObject(root, node_tempo_active_column, synth->current_tempo_column);
    
    /* loop through all existing nodes. Add them to an array. */
    array = cJSON_CreateArray();
    cJSON_AddItemToObject(root, node_nodes, array);
    
    for(a = 0; a < synth->patterns_height; a++) {
        for(s_x = 0; s_x < synth->track_width; s_x++) {
            for(s_y = 0; s_y < synth->track_max_height; s_y++) {
                if(synth->track[a] != NULL && synth->track[a][s_x] != NULL && synth->track[a][s_x][s_y] != NULL) {
                    struct CTrackNode *track_node = synth->track[a][s_x][s_y];
                    
                    // Only add node if it has any relevant data.
                    
                    if(track_node->tone_active ||
                       track_node->effect != '-' ||
                       track_node->effect_param1 != '-' ||
                       track_node->effect_param2 != '-' ||
                       track_node->effect_value != -1 ||
                       track_node->effect_param1_value != -1 ||
                       track_node->effect_param2_value != -1)
                    {
                        node = cJSON_CreateObject();
                        cJSON_AddNumberToObject(node, node_pattern, a);
                        cJSON_AddNumberToObject(node, node_column,	s_x);
                        cJSON_AddNumberToObject(node, node_row,	s_y);
                        cJSON_AddNumberToObject(node, node_tone, track_node->tone);
                        cJSON_AddNumberToObject(node, node_tone_active,	track_node->tone_active);
                        cJSON_AddNumberToObject(node, node_instrument, track_node->instrument_nr);
                        cJSON_AddNumberToObject(node, node_effect, track_node->effect);
                        cJSON_AddNumberToObject(node, node_effect_value, track_node->effect_value);
                        cJSON_AddNumberToObject(node, node_effect_param_1, track_node->effect_param1);
                        cJSON_AddNumberToObject(node, node_effect_param_1_value, track_node->effect_param1_value);
                        cJSON_AddNumberToObject(node, node_effect_param_2, track_node->effect_param2);
                        cJSON_AddNumberToObject(node, node_effect_param_2_value, track_node->effect_param2_value);
                        cJSON_AddItemToArray(array, node);
                    }
                }
            }
        }
    }
    
    array = cJSON_CreateArray();
    cJSON_AddItemToObject(root, node_instrument_effects, array);
    // instrument effects
    for(s_x = 0; s_x < synth->max_instrument_effects; s_x++) {
        for(s_y = 0; s_y < synth->max_instrument_effects; s_y++) {
            if(synth->instrument_effects != NULL && synth->instrument_effects[s_x] != NULL) {
                
                struct CTrackNode *track_node = synth->instrument_effects[s_x][s_y];
                if(track_node != NULL) {
                    node = cJSON_CreateObject();
                    cJSON_AddNumberToObject(node, node_instrument_link,	s_x);
                    cJSON_AddNumberToObject(node, node_row,	s_y);
                    cJSON_AddNumberToObject(node, node_effect, track_node->effect);
                    cJSON_AddNumberToObject(node, node_effect_value, track_node->effect_value);
                    cJSON_AddNumberToObject(node, node_effect_param_1, track_node->effect_param1);
                    cJSON_AddNumberToObject(node, node_effect_param_1_value, track_node->effect_param1_value);
                    cJSON_AddNumberToObject(node, node_effect_param_2, track_node->effect_param2);
                    cJSON_AddNumberToObject(node, node_effect_param_2_value, track_node->effect_param2_value);
                    cJSON_AddItemToArray(array, node);
                }
            }
        }
    }

    // new save
    int size = synth->patterns_and_voices_width*synth->patterns_and_voices_height;
    int int_array[size];
    int pos = 0;
    for(s_x = 0; s_x < synth->patterns_and_voices_width; s_x++) {
        for(s_y = 0; s_y < synth->patterns_and_voices_height; s_y++) {
            if(synth->patterns_and_voices != NULL && synth->patterns_and_voices[s_x] != NULL) {
                int_array[pos] = synth->patterns_and_voices[s_x][s_y];
                pos++;
            }
        }
    }
    cJSON *json_int_array = cJSON_CreateIntArray(int_array, size);
    cJSON_AddItemToObject(root, node_patterns_and_voices, json_int_array);
    
    size = synth->patterns_width*synth->patterns_height;
    int int_array_patterns[size];
    pos = 0;
    for(s_x = 0; s_x < synth->patterns_width; s_x++) {
        for(s_y = 0; s_y < synth->patterns_height; s_y++) {
            if(synth->patterns != NULL && synth->patterns[s_x] != NULL) {
                int_array_patterns[pos] = synth->patterns[s_x][s_y];
                pos++;
            }
        }
    }
    json_int_array = cJSON_CreateIntArray(int_array_patterns, size);
    cJSON_AddItemToObject(root, node_patterns, json_int_array);
    
    // active tracks
    size = synth->patterns_height;
    int int_array_tracks[64];
    for(i = 0; i < synth->patterns_height; i++) {
        int_array_tracks[i] = synth->active_tracks[i];
    }
    json_int_array = cJSON_CreateIntArray(int_array_tracks, size);
    cJSON_AddItemToObject(root, node_active_tracks, json_int_array);
    
    // muted voices
    size = synth->max_voices;
    int int_array_voices[size];
    for(i = 0; i < size; i++) {
        int_array_voices[i] = synth->voices[i]->muted;
    }
    json_int_array = cJSON_CreateIntArray(int_array_voices, size);
    cJSON_AddItemToObject(root, node_muted_voices, json_int_array);
 
    /* loop through all instruments, add to array. */
    array = cJSON_CreateArray();
    cJSON_AddItemToObject(root, node_instruments_adsr, array);
    for(i = 0; i < synth->max_instruments; i++) {
        instrument = cJSON_CreateArray();
        /* add the 5 nodes to array. */
        for(s_x = 0; s_x < synth->max_adsr_nodes; s_x++) {
            adsr_node = cJSON_CreateObject();
            cJSON_AddNumberToObject(adsr_node, node_amp, synth->instruments[i]->adsr[s_x]->amp);
            cJSON_AddNumberToObject(adsr_node, node_pos, synth->instruments[i]->adsr[s_x]->pos);
            cJSON_AddItemToArray(instrument, adsr_node);
        }
        cJSON_AddItemToArray(array, instrument);
    }
    
    /* custom table instrument */
    custom_table_node = cJSON_CreateObject();
    cJSON_AddItemToObject(root, node_custom_table, custom_table_node);
    instrument = cJSON_CreateArray();
    /* add the 5 nodes to array. */
    for(s_x = 0; s_x < synth->max_adsr_nodes; s_x++) {
        adsr_node = cJSON_CreateObject();
        cJSON_AddNumberToObject(adsr_node, node_amp, synth->custom_instrument->adsr[s_x]->amp);
        cJSON_AddNumberToObject(adsr_node, node_pos, synth->custom_instrument->adsr[s_x]->pos);
        cJSON_AddItemToArray(instrument, adsr_node);
    }
    cJSON_AddItemToObject(custom_table_node, node_custom_instrument, instrument);
    
    /* save tempo */
    array = cJSON_CreateArray();
    for(s_x = 0; s_x < synth->tempo_width; s_x++) {
        for(s_y = 0; s_y < synth->tempo_height; s_y++) {
            if(synth->tempo_map != NULL && synth->tempo_map[s_x] != NULL) {
                struct CTempoNode *t = synth->tempo_map[s_x][s_y];
                tempo_node = cJSON_CreateObject();
                cJSON_AddNumberToObject(tempo_node, node_tempo_x, s_x);
                cJSON_AddNumberToObject(tempo_node, node_tempo_y, s_y);
                if (s_y == 0) {
                    cJSON_AddNumberToObject(tempo_node, node_tempo_bpm, t->bpm);
                } else {
                    cJSON_AddNumberToObject(tempo_node, node_tempo_ticks, t->ticks);
                    cJSON_AddNumberToObject(tempo_node, node_tempo_active, t->active);
                }
                cJSON_AddItemToArray(array, tempo_node);
            }
        }
    }
    cJSON_AddItemToObject(root, node_tempo, array);
    
    /* save wavetable */
    array = cJSON_CreateArray();
    for(s_x = 0; s_x < synth->wavetable_width; s_x++) {
        for(s_y = 0; s_y < synth->wavetable_height; s_y++) {
            if(synth->wavetable_map != NULL && synth->wavetable_map[s_x] != NULL) {
                struct CWavetableNode *t = synth->wavetable_map[s_x][s_y];
                wavetable_node = cJSON_CreateObject();
                cJSON_AddNumberToObject(wavetable_node, node_wavetable_x, s_x);
                cJSON_AddNumberToObject(wavetable_node, node_wavetable_y, s_y);
                if (s_y == 0) {
                    cJSON_AddNumberToObject(wavetable_node, node_wavetable_speed, t->speed);
                } else {
                    cJSON_AddNumberToObject(wavetable_node, node_wavetable_value, t->value);
                    cJSON_AddNumberToObject(wavetable_node, node_wavetable_active, t->active);
                }
                cJSON_AddItemToArray(array, wavetable_node);
            }
        }
    }
    cJSON_AddItemToObject(root, node_wavetable, array);
    return root;
}


int cSynthLoadProject(struct CSynthContext *synth, const char* json) {
    
    cSynthResetTrackProgress(synth, 0, 0);
    
    /* reset voices */
    for (int i = 0; i < synth->max_voices; i++) {
        struct CVoice *v = synth->voices[i];
        cSynthResetVoice(synth, synth->sine_wave_table, v);
    }
    
    cJSON *object = NULL;
    cJSON *array = NULL;
    cJSON *node = NULL;
    cJSON *instrument = NULL;
    char *node_author = NULL;
    char *node_title = NULL;
    char *node_info = NULL;
    char *node_master_amp_percent = NULL;
    char *node_solo_voice = NULL;
    char *node_solo_track = NULL;
    char *node_active_tracks = NULL;
    char *node_muted_voices = NULL;
    char *node_track_height = NULL;
	char *node_arp = NULL;
    char *node_pattern = NULL;
    char *node_column = NULL;
    char *node_row = NULL;
    char *node_tone = NULL;
    char *node_tone_active = NULL;
    char *node_instrument = NULL;
    char *node_effect = NULL;
    char *node_effect_value = NULL;
    char *node_effect_param_1 = NULL;
    char *node_effect_param_1_value = NULL;
    char *node_effect_param_2 = NULL;
    char *node_effect_param_2_value = NULL;
    char *node_nodes = NULL;
    char *node_patterns_and_voices = NULL;
    char *node_patterns = NULL;
    char *node_instruments_adsr = NULL;
    char *node_amp = NULL;
    char *node_pos = NULL;
    char *node_instrument_effects = NULL;
    char *node_instrument_link = NULL;
    char *node_track_highlight_interval = NULL;
    char *node_tempo = NULL;
    char *node_tempo_active_column = NULL;
    char *node_tempo_bpm = NULL;
    char *node_tempo_ticks = NULL;
    char *node_tempo_active = NULL;
    char *node_tempo_x = NULL;
    char *node_tempo_y = NULL;
    char *node_wavetable = NULL;
    char *node_wavetable_speed = NULL;
    char *node_wavetable_value = NULL;
    char *node_wavetable_active = NULL;
    char *node_wavetable_x = NULL;
    char *node_wavetable_y = NULL;
    char *node_custom_table = NULL;
    char *node_custom_instrument = NULL;
    
    char *node_file_version = "file_version";
    int i, s_x, s_y, nodes_size, pattern_count;
    
    cJSON *root = cJSON_Parse(json);
    
    if(root == NULL) { cSynthLoadFailed("root missing", root); return 0; }
    
    node_author = "author";
    node_title = "title";
    node_info = "info";
    node_master_amp_percent = "master_amp_percent";
    node_solo_voice = "solo_voice";
    node_solo_track = "solo_track";
    node_muted_voices = "muted_voices";
    node_active_tracks = "active_tracks";
    node_track_height = "track_height";
    node_arp = "arp";
	node_pattern = "p";
    node_column = "c";
    node_row = "r";
    node_tone = "n";
    node_tone_active = "t";
    node_instrument = "i";
    node_effect = "e";
    node_effect_value = "f";
    node_effect_param_1 = "g";
    node_effect_param_1_value = "h";
    node_effect_param_2 = "j";
    node_effect_param_2_value = "k";
    node_nodes = "nodes";
    node_patterns_and_voices = "patterns_and_voices";
    node_patterns = "patterns";
    node_instruments_adsr = "instruments_adsr";
    node_amp = "a";
    node_pos = "p";
    node_instrument_effects = "instrument_effects";
    node_instrument_link = "i_l";
    node_track_highlight_interval = "track_highlight_interval";
    node_tempo = "tempo";
    node_tempo_active_column = "active_tempo_column";
    node_tempo_bpm = "b";
    node_tempo_ticks = "t";
    node_tempo_active = "a";
    node_tempo_x = "x";
    node_tempo_y = "y";
    node_wavetable = "wavetable";
    node_wavetable_speed = "s";
    node_wavetable_value = "v";
    node_wavetable_active = "a";
    node_wavetable_x = "x";
    node_wavetable_y = "y";
    node_custom_table = "custom_table";
    node_custom_instrument = "i";
    
    object = cJSON_GetObjectItem(root, node_file_version);
    if(object == NULL) { cSynthLoadFailed("file version missing", root); return 0; }
    
    object = cJSON_GetObjectItem(root, node_author);
    if(object != NULL) {
        synth->str_author = cStrPrint(synth->str_author, object->valuestring);
    }
    object = cJSON_GetObjectItem(root, node_title);
    if(object != NULL) {
        synth->str_title = cStrPrint(synth->str_title, object->valuestring);
    }
    object = cJSON_GetObjectItem(root, node_info);
    if(object != NULL) {
        synth->str_info = cStrPrint(synth->str_info, object->valuestring);
    }
    
    object = cJSON_GetObjectItem(root, node_master_amp_percent);
    if(object != NULL) {
        synth->master_amp_percent = object->valueint;
        synth->master_amp = synth->master_amp_percent * 0.01;
    }
    
    object = cJSON_GetObjectItem(root, node_solo_voice);
    if(object != NULL) {
        synth->solo_voice = object->valueint;
    }
    
    object = cJSON_GetObjectItem(root, node_solo_track);
    if(object != NULL) {
        synth->solo_track = object->valueint;
    }
    
    object = cJSON_GetObjectItem(root, node_track_height);
    if(object == NULL) { cSynthLoadFailed("track height missing", root); return 0; }
    synth->track_height = object->valueint;
    
	object = cJSON_GetObjectItem(root, node_arp);
    if(object == NULL) { cSynthLoadFailed("arp missing", root); return 0; }
    synth->arpeggio_speed = object->valueint;
	
    object = cJSON_GetObjectItem(root, node_track_highlight_interval);
    if(object != NULL) {
        synth->track_highlight_interval = object->valueint;
    }
    
    object = cJSON_GetObjectItem(root, node_tempo_active_column);
    if(object != NULL) {
        synth->current_tempo_column = object->valueint;
    }
    
    object = cJSON_GetObjectItem(root, node_custom_table);
    if(object != NULL) {
        array = cJSON_GetObjectItem(object, node_custom_instrument);
        if(array != NULL) {
            /* add the 5 nodes to array. */
            for(s_x = 0; s_x < synth->max_adsr_nodes; s_x++) {
                node = cJSON_GetArrayItem(array, s_x);
                synth->custom_instrument->adsr[s_x]->amp = cJSON_GetObjectItem(node, node_amp)->valuedouble;
                synth->custom_instrument->adsr[s_x]->pos = cJSON_GetObjectItem(node, node_pos)->valuedouble;
                if(s_x == 0) {
                    // force first node to 0.
                    synth->custom_instrument->adsr[s_x]->pos = 0.0;
                }
            }
            cSynthWriteCustomTableFromNodes(synth);
        }
    }
    
    /* nodes */
    array = cJSON_GetObjectItem(root, node_nodes);
    nodes_size = cJSON_GetArraySize(array);
    for (i = 0; i < nodes_size; i++) {
        struct CTrackNode *track_node = cSynthNewTrackNode();
        
        int value_node_pattern = 0;
        int value_node_column = 0;
        int value_node_row = 0;
        int value_node_tone = 0;
        int value_node_tone_active = 0;
        int value_node_instrument = 0;
        int value_node_effect = 0;
        int value_node_effect_value = 0;
        int value_node_effect_param_1 = 0;
        int value_node_effect_param_1_value = 0;
        int value_node_effect_param_2 = 0;
        int value_node_effect_param_2_value = 0;
        
        node = cJSON_GetArrayItem(array, i);
        
        object = cJSON_GetObjectItem(node, node_pattern);
        if(object == NULL) { cSynthLoadFailed("node_pattern missing", root); return 0; }
        value_node_pattern = object->valueint;
        
        object = cJSON_GetObjectItem(node, node_column);
        if(object == NULL) { cSynthLoadFailed("node_column missing", root); return 0; }
        value_node_column = object->valueint;
        
        object = cJSON_GetObjectItem(node, node_row);
        if(object == NULL) { cSynthLoadFailed("node_row missing", root); return 0; }
        value_node_row = object->valueint;
        
        object = cJSON_GetObjectItem(node, node_tone);
        if(object == NULL) { cSynthLoadFailed("node_note missing", root); return 0; }
        value_node_tone = object->valueint;
        
        object = cJSON_GetObjectItem(node, node_tone_active);
        if(object == NULL) { cSynthLoadFailed("node_tone_active missing", root); return 0; }
        value_node_tone_active = object->valueint;
        
        object = cJSON_GetObjectItem(node, node_instrument);
        if(object == NULL) { cSynthLoadFailed("node_instrument missing", root); return 0; }
        value_node_instrument = object->valueint;
        
        object = cJSON_GetObjectItem(node, node_effect);
        if(object == NULL) { cSynthLoadFailed("node_effect missing", root); return 0; }
        value_node_effect = object->valueint;
        
        object = cJSON_GetObjectItem(node, node_effect_value);
        if(object == NULL) { cSynthLoadFailed("node_effect_value missing", root); return 0; }
        value_node_effect_value = object->valueint;
        
        object = cJSON_GetObjectItem(node, node_effect_param_1);
        if(object == NULL) { cSynthLoadFailed("node_effect_param_1 missing", root); return 0; }
        value_node_effect_param_1 = object->valueint;
        
        object = cJSON_GetObjectItem(node, node_effect_param_1_value);
        if(object == NULL) { cSynthLoadFailed("node_effect_param_1_value missing", root); return 0; }
        value_node_effect_param_1_value = object->valueint;
        
        object = cJSON_GetObjectItem(node, node_effect_param_2);
        if(object == NULL) { cSynthLoadFailed("node_effect_param_2 missing", root); return 0; }
        value_node_effect_param_2 = object->valueint;
        
        object = cJSON_GetObjectItem(node, node_effect_param_2_value);
        if(object == NULL) { cSynthLoadFailed("node_effect_param_2_value missing", root); return 0; }
        value_node_effect_param_2_value = object->valueint;
        
        track_node->tone = value_node_tone;
        track_node->tone_active = value_node_tone_active;
        track_node->instrument_nr = value_node_instrument;
        track_node->instrument = synth->instruments[value_node_instrument];
        track_node->effect = (char)value_node_effect;
        track_node->effect_value = (char)value_node_effect_value;
        track_node->effect_param1 = (char)value_node_effect_param_1;
        track_node->effect_param1_value = (char)value_node_effect_param_1_value;
        track_node->effect_param2 = (char)value_node_effect_param_2;
        track_node->effect_param2_value = (char)value_node_effect_param_2_value;
        
        synth->track[value_node_pattern][value_node_column][value_node_row] = track_node;
    }
    
    /* instrument effects */
    for(s_x = 0; s_x < synth->max_instrument_effects; s_x++) {
        for(s_y = 0; s_y < synth->max_instrument_effects; s_y++) {
            if(synth->instrument_effects != NULL && synth->instrument_effects[s_x] != NULL) {
                if(synth->instrument_effects[s_x][s_y] != NULL) {
                    synth->instrument_effects[s_x][s_y] = cAllocatorFree(synth->instrument_effects[s_x][s_y]);
                }
            }
        }
    }

    array = cJSON_GetObjectItem(root, node_instrument_effects);
    nodes_size = cJSON_GetArraySize(array);
    for (i = 0; i < nodes_size; i++) {
        struct CTrackNode *track_node = cSynthNewTrackNode();
        
        int value_instrument_link = 0;
        int value_node_row = 0;
        int value_node_effect = 0;
        int value_node_effect_value = 0;
        int value_node_effect_param_1 = 0;
        int value_node_effect_param_1_value = 0;
        int value_node_effect_param_2 = 0;
        int value_node_effect_param_2_value = 0;
        
        node = cJSON_GetArrayItem(array, i);
        
        object = cJSON_GetObjectItem(node, node_instrument_link);
        if(object == NULL) { cSynthLoadFailed("node_pattern missing", root); return 0; }
        value_instrument_link = object->valueint;
        
        object = cJSON_GetObjectItem(node, node_row);
        if(object == NULL) { cSynthLoadFailed("node_row missing", root); return 0; }
        value_node_row = object->valueint;
        
        object = cJSON_GetObjectItem(node, node_effect);
        if(object == NULL) { cSynthLoadFailed("node_effect missing", root); return 0; }
        value_node_effect = object->valueint;
        
        object = cJSON_GetObjectItem(node, node_effect_value);
        if(object == NULL) { cSynthLoadFailed("node_effect_value missing", root); return 0; }
        value_node_effect_value = object->valueint;
        
        object = cJSON_GetObjectItem(node, node_effect_param_1);
        if(object == NULL) { cSynthLoadFailed("node_effect_param_1 missing", root); return 0; }
        value_node_effect_param_1 = object->valueint;
        
        object = cJSON_GetObjectItem(node, node_effect_param_1_value);
        if(object == NULL) { cSynthLoadFailed("node_effect_param_1_value missing", root); return 0; }
        value_node_effect_param_1_value = object->valueint;
        
        object = cJSON_GetObjectItem(node, node_effect_param_2);
        if(object == NULL) { cSynthLoadFailed("node_effect_param_2 missing", root); return 0; }
        value_node_effect_param_2 = object->valueint;
        
        object = cJSON_GetObjectItem(node, node_effect_param_2_value);
        if(object == NULL) { cSynthLoadFailed("node_effect_param_2_value missing", root); return 0; }
        value_node_effect_param_2_value = object->valueint;
        
        track_node->effect = (char)value_node_effect;
        track_node->effect_value = (char)value_node_effect_value;
        track_node->effect_param1 = (char)value_node_effect_param_1;
        track_node->effect_param1_value = (char)value_node_effect_param_1_value;
        track_node->effect_param2 = (char)value_node_effect_param_2;
        track_node->effect_param2_value = (char)value_node_effect_param_2_value;
        
        synth->instrument_effects[value_instrument_link][value_node_row] = track_node;
    }

    array = cJSON_GetObjectItem(root, node_patterns_and_voices);
    pattern_count = 0;
    /* loop through all patterns and voices */
    for(s_x = 0; s_x < synth->patterns_and_voices_width; s_x++) {
        for(s_y = 0; s_y < synth->patterns_and_voices_height; s_y++) {
            if(synth->patterns_and_voices != NULL && synth->patterns_and_voices[s_x] != NULL) {
                node = cJSON_GetArrayItem(array, pattern_count);
                synth->patterns_and_voices[s_x][s_y] = node->valueint;
                pattern_count++;
            }
        }
    }
    
    array = cJSON_GetObjectItem(root, node_patterns);
    pattern_count = 0;
    /* loop through all patterns */
    for(s_x = 0; s_x < synth->patterns_width; s_x++) {
        for(s_y = 0; s_y < synth->patterns_height; s_y++) {
            if(synth->patterns != NULL && synth->patterns[s_x] != NULL) {
                node = cJSON_GetArrayItem(array, pattern_count);
                synth->patterns[s_x][s_y] = node->valueint;
                pattern_count++;
            }
        }
    }
    
    // active tracks
    array = cJSON_GetObjectItem(root, node_active_tracks);
    pattern_count = 0;
    /* loop through all patterns */
    for(i = 0; i < synth->patterns_height; i++) {
        node = cJSON_GetArrayItem(array, i);
        synth->active_tracks[i] = node->valueint;
        pattern_count++;
    }

    // muted voices
    array = cJSON_GetObjectItem(root, node_muted_voices);
    pattern_count = 0;
    /* loop through all voices */
    for(i = 0; i < synth->max_voices; i++) {
        node = cJSON_GetArrayItem(array, i);
        synth->voices[i]->muted = node->valueint;
        pattern_count++;
    }
    
    array = cJSON_GetObjectItem(root, node_instruments_adsr);
    /* loop through all instruments */
    for(i = 0; i < synth->max_instruments; i++) {
        instrument = cJSON_GetArrayItem(array, i);
            
        /* add the 5 nodes to array. */
        for(s_x = 0; s_x < synth->max_adsr_nodes; s_x++) {
            node = cJSON_GetArrayItem(instrument, s_x);
            synth->instruments[i]->adsr[s_x]->amp = cJSON_GetObjectItem(node, node_amp)->valuedouble;
            synth->instruments[i]->adsr[s_x]->pos = cJSON_GetObjectItem(node, node_pos)->valuedouble;
        }
    }
    
    // reset tempo
    for(s_x = 0; s_x < synth->tempo_width; s_x++) {
        for(s_y = 0; s_y < synth->tempo_height; s_y++) {
            if(synth->tempo_map != NULL && synth->tempo_map[s_x] != NULL) {
                if (s_y == 0) {
                    synth->tempo_map[s_x][s_y]->bpm = synth->default_bpm;
                } else if(s_y == 1 || (s_x == 0 && s_y < 5 && s_y > 0)) {
                    synth->tempo_map[s_x][s_y]->active = true;
                    synth->tempo_map[s_x][s_y]->ticks = 4;
                }
            }
        }
    }
    
    // tempo
    int value_node_tempo_x = 0;
    int value_node_tempo_y = 0;
    int value_node_tempo_ticks = 0;
    int value_node_tempo_bpm = 0;
    int value_node_tempo_active = 0;
    array = cJSON_GetObjectItem(root, node_tempo);
    if(array != NULL) {
        nodes_size = cJSON_GetArraySize(array);
        for(i = 0; i < nodes_size; i++) {
            node = cJSON_GetArrayItem(array, i);
            object = cJSON_GetObjectItem(node, node_tempo_y);
            if(object != NULL) {
                value_node_tempo_y = object->valueint;
                if(value_node_tempo_y == 0) {
                    object = cJSON_GetObjectItem(node, node_tempo_x);
                    if(object != NULL) {
                        value_node_tempo_x = object->valueint;
                    } else {
                        if(synth->errorlog) { printf("error: node_tempo_x is null\n"); }
                    }
                    object = cJSON_GetObjectItem(node, node_tempo_bpm);
                    if(object != NULL) {
                        value_node_tempo_bpm = object->valueint;
                    } else {
                        if(synth->errorlog) { printf("error: node_tempo_bpm is null\n"); }
                    }
                    synth->tempo_map[value_node_tempo_x][value_node_tempo_y]->bpm = value_node_tempo_bpm;
                } else {
                    object = cJSON_GetObjectItem(node, node_tempo_x);
                    if(object != NULL) {
                        value_node_tempo_x = object->valueint;
                    } else {
                        if(synth->errorlog) { printf("error: node_tempo_x is null\n"); }
                    }
                    object = cJSON_GetObjectItem(node, node_tempo_active);
                    if(object != NULL) {
                        value_node_tempo_active = object->valueint;
                    } else {
                        if(synth->errorlog) { printf("error: node_tempo_active is null\n"); }
                    }
                    object = cJSON_GetObjectItem(node, node_tempo_ticks);
                    if(object != NULL) {
                        value_node_tempo_ticks = object->valueint;
                    } else {
                        if(synth->errorlog) { printf("error: node_tempo_ticks is null\n"); }
                    }
                    synth->tempo_map[value_node_tempo_x][value_node_tempo_y]->active = value_node_tempo_active;
                    synth->tempo_map[value_node_tempo_x][value_node_tempo_y]->ticks = (char)value_node_tempo_ticks;
                }
            } else {
                if(synth->errorlog) { printf("error: node_tempo_y is null\n"); }
            }
        }
    }
    
    int value_node_wavetable_x = 0;
    int value_node_wavetable_y = 0;
    int value_node_wavetable_value = 0;
    int value_node_wavetable_speed = 0;
    int value_node_wavetable_active = 0;
    array = cJSON_GetObjectItem(root, node_wavetable);
    if(array != NULL) {
        nodes_size = cJSON_GetArraySize(array);
        for(i = 0; i < nodes_size; i++) {
            node = cJSON_GetArrayItem(array, i);
            object = cJSON_GetObjectItem(node, node_wavetable_y);
            if(object != NULL) {
                value_node_wavetable_y = object->valueint;
                if(value_node_wavetable_y == 0) {
                    object = cJSON_GetObjectItem(node, node_wavetable_x);
                    if(object != NULL) {
                        value_node_wavetable_x = object->valueint;
                    } else {
                        if(synth->errorlog) { printf("error: node_wavetable_x is null\n"); }
                    }
                    object = cJSON_GetObjectItem(node, node_wavetable_speed);
                    if(object != NULL) {
                        value_node_wavetable_speed = object->valueint;
                    } else {
                        if(synth->errorlog) { printf("error: node_wavetable_bpm is null\n"); }
                    }
                    synth->wavetable_map[value_node_wavetable_x][value_node_wavetable_y]->speed = value_node_wavetable_speed;
                } else {
                    object = cJSON_GetObjectItem(node, node_wavetable_x);
                    if(object != NULL) {
                        value_node_wavetable_x = object->valueint;
                    } else {
                        if(synth->errorlog) { printf("error: node_wavetable_x is null\n"); }
                    }
                    object = cJSON_GetObjectItem(node, node_wavetable_active);
                    if(object != NULL) {
                        value_node_wavetable_active = object->valueint;
                    } else {
                        if(synth->errorlog) { printf("error: node_wavetable_active is null\n"); }
                    }
                    object = cJSON_GetObjectItem(node, node_wavetable_value);
                    if(object != NULL) {
                        value_node_wavetable_value = object->valueint;
                    } else {
                        if(synth->errorlog) { printf("error: node_wavetable_ticks is null\n"); }
                    }
                    synth->wavetable_map[value_node_wavetable_x][value_node_wavetable_y]->active = value_node_wavetable_active;
                    synth->wavetable_map[value_node_wavetable_x][value_node_wavetable_y]->value = (char)value_node_wavetable_value;
                }
            } else {
                if(synth->errorlog) { printf("error: node_wavetable_y is null\n"); }
            }
        }
    }
    
    cSynthSetWaveformsFromPattern(synth);
    cJSON_Delete(root);
    
    return 1;
}
       
static void cSynthLoadFailed(char *reason, cJSON *json) {
    
    if(json != NULL) {
        printf("load failed:%s json:%s\n", reason, cJSON_Print(json));
    } else {
        printf("load failed:%s\n", reason);
    }
}

void cSynthReset(struct CSynthContext *synth) {
    
    synth->str_author = cStrCleanup(synth->str_author);
    synth->str_title = cStrCleanup(synth->str_title);
    synth->str_info = cStrCleanup(synth->str_info);
    
    synth->track_cursor_x = 0;
    synth->track_cursor_y = 0;
    
    synth->bitcrush_active = false;
    
    cSynthResetTrackProgress(synth, 0, 0);
    
    /* reset voices */
    for (int i = 0; i < synth->max_voices; i++) {
        struct CVoice *v = synth->voices[i];
        cSynthResetVoice(synth, synth->sine_wave_table, v);
    }
    
    synth->voices[0]->waveform = synth->sine_wave_table;
    synth->voices[1]->waveform = synth->sawtooth_wave_table;
    synth->voices[2]->waveform = synth->square_wave_table;
    synth->voices[3]->waveform = synth->triangle_wave_table;
    synth->voices[4]->waveform = synth->noise_table;
    synth->voices[5]->waveform = synth->noise_table;
    
    /* clear all track nodes before loading */
    int i = 0;
    int a = 0;
    int s_x = 0;
    int s_y = 0;
    
    for(a = 0; a < synth->max_tracks_and_patterns; a++) {
        for(s_x = 0; s_x < synth->track_width; s_x++) {
            for(s_y = 0; s_y < synth->track_max_height; s_y++) {
                if(synth->track[a] != NULL && synth->track[a][s_x] != NULL) {
                    synth->track[a][s_x][s_y] = cAllocatorFree(synth->track[a][s_x][s_y]);
                }
            }
        }
    }
    
    for(s_x = 0; s_x < synth->patterns_and_voices_width; s_x++) {
        for(s_y = 0; s_y < synth->patterns_and_voices_height; s_y++) {
            if(synth->patterns_and_voices != NULL && synth->patterns_and_voices[s_x] != NULL) {
                synth->patterns_and_voices[s_x][s_y] = 0;
            }
        }
    }
    
    for(s_x = 0; s_x < synth->patterns_and_voices_width; s_x++) {
        for(s_y = 0; s_y < synth->patterns_and_voices_height; s_y++) {
            if(synth->patterns_and_voices != NULL && synth->patterns_and_voices[s_x] != NULL) {
                if(s_x == 0 && s_y == 0) {
                    synth->patterns_and_voices[s_x][s_y] = 0;
                } else if(s_x == 1 && s_y == 0) {
                    synth->patterns_and_voices[s_x][s_y] = 1;
                } else if(s_x == 2 && s_y == 0) {
                    synth->patterns_and_voices[s_x][s_y] = 2;
                } else if(s_x == 3 && s_y == 0) {
                    synth->patterns_and_voices[s_x][s_y] = 3;
                } else if(s_x == 4 && s_y == 0) {
                    synth->patterns_and_voices[s_x][s_y] = 4;
                } else if(s_x == 5 && s_y == 0) {
                    synth->patterns_and_voices[s_x][s_y] = 4;
                }
            }
        }
    }
    
    synth->master_amp = 0.5;
    synth->master_amp_percent = 50;
    synth->default_bpm = 120;
    synth->current_instrument = 0;
    synth->track_height = 16;
    synth->current_track = 0;
    synth->arpeggio_speed = 10;
    
    //patterns
    for(s_x = 0; s_x < synth->patterns_width; s_x++) {
        for(s_y = 0; s_y < synth->patterns_height; s_y++) {
            if(synth->patterns != NULL && synth->patterns[s_x] != NULL) {
                synth->patterns[s_x][s_y] = 0;
            }
        }
    }
    
    /* active tracks */
    for (i = 0; i < synth->patterns_height; i++) {
        synth->active_tracks[i] = 0;
    }
    // activate first track
    synth->active_tracks[0] = 1;
    
    // no track or voice is solo to begin with.
    synth->solo_track = -1;
    synth->solo_voice = -1;
    
    
    /* tempo */
    for(s_x = 0; s_x < synth->tempo_width; s_x++) {
        for(s_y = 0; s_y < synth->tempo_height; s_y++) {
            if(synth->tempo_map != NULL && synth->tempo_map[s_x] != NULL) {
                synth->tempo_map[s_x][s_y]->active = false;
                synth->tempo_map[s_x][s_y]->ticks = 1;
                if (s_y == 0) {
                    synth->tempo_map[s_x][s_y]->bpm = synth->default_bpm;
                } else if(s_y == 1 || (s_x == 0 && s_y < 5 && s_y > 0)) {
                    synth->tempo_map[s_x][s_y]->active = true;
                    synth->tempo_map[s_x][s_y]->ticks = 4;
                }
            } else {
                fprintf(stderr, "error allocating tempo_node, setting null.");
            }
        }
    }
    synth->current_tempo_column = 0;
    
    
    /* envelopes */
    for(i = 0; i < synth->max_instruments; i++) {
        cSynthInstrumentResetEnvelope(synth->instruments[i]);
    }
    
    /* instrument effects */
    for(s_x = 0; s_x < synth->max_instrument_effects; s_x++) {
        for(s_y = 0; s_y < synth->max_instrument_effects; s_y++) {
            if(synth->instrument_effects != NULL && synth->instrument_effects[s_x] != NULL) {
                if(synth->instrument_effects[s_x][s_y] != NULL) {
                    synth->instrument_effects[s_x][s_y] = cAllocatorFree(synth->instrument_effects[s_x][s_y]);
                }
            }
        }
    }
    
    // swap track
    for(s_x = 0; s_x < synth->track_width; s_x++) {
        for(s_y = 0; s_y < synth->track_max_height; s_y++) {
            if(synth->swap_track != NULL && synth->swap_track[s_x] != NULL && synth->swap_track[s_x][s_y] != NULL) {
                synth->swap_track[s_x][s_y] = cAllocatorFree(synth->swap_track[s_x][s_y]);
            }
        }
    }
    
    // copy track
    for(s_x = 0; s_x < synth->track_width; s_x++) {
        for(s_y = 0; s_y < synth->track_max_height; s_y++) {
            if(synth->copy_track != NULL && synth->copy_track[s_x] != NULL && synth->copy_track[s_x][s_y] != NULL) {
                synth->copy_track[s_x][s_y] = cAllocatorFree(synth->copy_track[s_x][s_y]);
            }
        }
    }
    
    // custom instrument
    synth->custom_instrument->adsr[0]->amp = 0.0;
    synth->custom_instrument->adsr[0]->pos = 0.0;
    synth->custom_instrument->adsr[1]->amp = 1.0;
    synth->custom_instrument->adsr[1]->pos = 0.4;
    synth->custom_instrument->adsr[2]->amp = 0.0;
    synth->custom_instrument->adsr[2]->pos = 0.6;
    synth->custom_instrument->adsr[3]->amp = -1.0;
    synth->custom_instrument->adsr[3]->pos = 0.8;
    synth->custom_instrument->adsr[4]->amp = 0.0;
    synth->custom_instrument->adsr[4]->pos = 1.0;
    
    // wavetable
    for(s_x = 0; s_x < synth->wavetable_width; s_x++) {
        for(s_y = 0; s_y < synth->wavetable_height; s_y++) {
            if(synth->wavetable_map != NULL && synth->wavetable_map[s_x] != NULL) {
                if(synth->wavetable_map[s_x][s_y] != NULL) {
                    synth->wavetable_map[s_x][s_y]->active = false;
                    synth->wavetable_map[s_x][s_y]->value = 1;
                    synth->wavetable_map[s_x][s_y]->speed = 1;
                    if (s_y == 0) {
                        synth->wavetable_map[s_x][s_y]->speed = 50;
                    } else if(s_y == 2) {
                        synth->wavetable_map[s_x][s_y]->active = true;
                        synth->wavetable_map[s_x][s_y]->value = 1;
                    }
                }
            }
        }
    }
}

void cSynthCleanup(struct CSynthContext *synth) {
    
    int i;
    int a;
    int s_x;
    int s_y;
    int c;
    
    if(synth != NULL) {
        /* patterns and voices */
        for(i = 0; i < synth->patterns_and_voices_width; i++) {
            synth->patterns_and_voices[i] = cAllocatorFree(synth->patterns_and_voices[i]);
        }
        synth->patterns_and_voices = cAllocatorFree(synth->patterns_and_voices);

        for(i = 0; i < synth->patterns_width; i++) {
            synth->patterns[i] = cAllocatorFree(synth->patterns[i]);
        }
        synth->patterns = cAllocatorFree(synth->patterns);

        
        /* track */
        for(a = 0; a < synth->patterns_height; a++) {
            for(s_x = 0; s_x < synth->track_width; s_x++) {
                for(s_y = 0; s_y < synth->track_max_height; s_y++) {
                    if(synth->track[a] != NULL && synth->track[a][s_x] != NULL) {
                        synth->track[a][s_x][s_y] = cAllocatorFree(synth->track[a][s_x][s_y]);
                    }
                }
            }
        }
        for(a = 0; a < synth->patterns_height; a++) {
            for(i = 0; i < synth->track_width; i++) {
                if(synth->track[a] != NULL) {
                    synth->track[a][i] = cAllocatorFree(synth->track[a][i]);
                }
            }
        }
        for(a = 0; a < synth->patterns_height; a++) {
            synth->track[a] = cAllocatorFree(synth->track[a]);
        }
        synth->track = cAllocatorFree(synth->track);
        
        
        /* swap track */
        for(s_x = 0; s_x < synth->track_width; s_x++) {
            for(s_y = 0; s_y < synth->track_max_height; s_y++) {
                if(synth->swap_track != NULL && synth->swap_track[s_x] != NULL) {
                    synth->swap_track[s_x][s_y] = cAllocatorFree(synth->swap_track[s_x][s_y]);
                }
            }
        }
        for(i = 0; i < synth->track_width; i++) {
            if(synth->swap_track != NULL) {
                synth->swap_track[i] = cAllocatorFree(synth->swap_track[i]);
            }
        }
        synth->swap_track = cAllocatorFree(synth->swap_track);
        
        /* copy track */
        for(s_x = 0; s_x < synth->track_width; s_x++) {
            for(s_y = 0; s_y < synth->track_max_height; s_y++) {
                if(synth->copy_track != NULL && synth->copy_track[s_x] != NULL) {
                    synth->copy_track[s_x][s_y] = cAllocatorFree(synth->copy_track[s_x][s_y]);
                }
            }
        }
        for(i = 0; i < synth->track_width; i++) {
            if(synth->copy_track != NULL) {
                synth->copy_track[i] = cAllocatorFree(synth->copy_track[i]);
            }
        }
        synth->copy_track = cAllocatorFree(synth->copy_track);
        
        /* instrument effects */
        for(s_x = 0; s_x < synth->max_instrument_effects; s_x++) {
            for(s_y = 0; s_y < synth->max_instrument_effects; s_y++) {
                if(synth->instrument_effects != NULL && synth->instrument_effects[s_x] != NULL) {
                    synth->instrument_effects[s_x][s_y] = cAllocatorFree(synth->instrument_effects[s_x][s_y]);
                }
            }
        }
        for(i = 0; i < synth->max_instrument_effects; i++) {
            if(synth->instrument_effects != NULL) {
                synth->instrument_effects[i] = cAllocatorFree(synth->instrument_effects[i]);
            }
        }
        synth->instrument_effects = cAllocatorFree(synth->instrument_effects);
        
        /* temp mixdown buffer */
        for(i = 0; i < synth->max_voices; i++) {
            if(synth->temp_mixdown_buffer != NULL) {
                synth->temp_mixdown_buffer[i] = cAllocatorFree(synth->temp_mixdown_buffer[i]);
            }
        }
        synth->temp_mixdown_buffer = cAllocatorFree(synth->temp_mixdown_buffer);
        
        
        /* voices and instruments */
        for(i = 0; i < synth->max_voices; i++) {
            if(synth->voices[i]->delay_buffer != NULL) {
                synth->voices[i]->delay_buffer = cAllocatorFree(synth->voices[i]->delay_buffer);
            }
            synth->voices[i] = cAllocatorFree(synth->voices[i]);
            
        }
        synth->voices = cAllocatorFree(synth->voices);
        
        for(i = 0; i < synth->max_instruments; i++) {
            
            for(c = 0; c < synth->instruments[i]->adsr_nodes; c++) {
                synth->instruments[i]->adsr[c] = cAllocatorFree(synth->instruments[i]->adsr[c]);
            }
            synth->instruments[i]->adsr = cAllocatorFree(synth->instruments[i]->adsr);
            synth->instruments[i] = cAllocatorFree(synth->instruments[i]);
        }
        synth->instruments = cAllocatorFree(synth->instruments);
        
        for(c = 0; c < synth->custom_instrument->adsr_nodes; c++) {
            synth->custom_instrument->adsr[c] = cAllocatorFree(synth->custom_instrument->adsr[c]);
        }
        synth->custom_instrument->adsr = cAllocatorFree(synth->custom_instrument->adsr);
        synth->custom_instrument = cAllocatorFree(synth->custom_instrument);
        
        /* tempo */
        for(s_x = 0; s_x < synth->tempo_width; s_x++) {
            for(s_y = 0; s_y < synth->tempo_height; s_y++) {
                if(synth->tempo_map != NULL && synth->tempo_map[s_x] != NULL) {
                    synth->tempo_map[s_x][s_y] = cAllocatorFree(synth->tempo_map[s_x][s_y]);
                }
            }
        }
        for(i = 0; i < synth->tempo_width; i++) {
            if(synth->tempo_map != NULL) {
                synth->tempo_map[i] = cAllocatorFree(synth->tempo_map[i]);
            }
        }
        synth->tempo_map = cAllocatorFree(synth->tempo_map);

        /* wavetable */
        for(s_x = 0; s_x < synth->wavetable_width; s_x++) {
            for(s_y = 0; s_y < synth->wavetable_height; s_y++) {
                if(synth->wavetable_map != NULL && synth->wavetable_map[s_x] != NULL) {
                    synth->wavetable_map[s_x][s_y] = cAllocatorFree(synth->wavetable_map[s_x][s_y]);
                }
            }
        }
        for(i = 0; i < synth->wavetable_width; i++) {
            if(synth->wavetable_map != NULL) {
                synth->wavetable_map[i] = cAllocatorFree(synth->wavetable_map[i]);
            }
        }
        synth->wavetable_map = cAllocatorFree(synth->wavetable_map);

        /* tables */
        synth->sine_wave_table = cAllocatorFree(synth->sine_wave_table);
        synth->sawtooth_wave_table = cAllocatorFree(synth->sawtooth_wave_table);
        synth->square_wave_table = cAllocatorFree(synth->square_wave_table);
        synth->triangle_wave_table = cAllocatorFree(synth->triangle_wave_table);
        synth->noise_table = cAllocatorFree(synth->noise_table);
        synth->custom_table = cAllocatorFree(synth->custom_table);
        synth->active_tracks = cAllocatorFree(synth->active_tracks);
        
        /* synth */
        cAllocatorFree(synth);
    }
}

void cSynthInitData(struct CSynthContext *synth) {
    
    int a, i, s_x, s_y;
    
    if(synth != NULL) {
        
        synth->temp_mixdown_buffer = cAllocatorAlloc(sizeof(int16_t*)*synth->max_voices, "mixdown buffer 1");
        for (i = 0; i < synth->max_voices; i++) {
            synth->temp_mixdown_buffer[i] = cAllocatorAlloc(sizeof(int16_t)*synth->temp_mixdown_size, "mixdown buffer 2");
        }
        for(i = 0; i < synth->max_voices; i++) {
            for(s_x = 0; s_x < synth->temp_mixdown_size; s_x++) {
                synth->temp_mixdown_buffer[i][s_x] = 0;
            }
        }
        
        synth->sine_wave_table = cAllocatorAlloc(sizeof(int16_t)*synth->wave_length, "PCM table");
        cSynthBuildSineWave(synth->sine_wave_table, synth->wave_length);
        
        synth->sawtooth_wave_table = cAllocatorAlloc(sizeof(int16_t)*synth->wave_length, "PCM table");
        cSynthBuildSawtoothWave(synth->sawtooth_wave_table, synth->wave_length);
        
        synth->square_wave_table = cAllocatorAlloc(sizeof(int16_t)*synth->wave_length, "PCM table");
        cSynthBuildSquareWave(synth->square_wave_table, synth->wave_length);
        
        synth->triangle_wave_table = cAllocatorAlloc(sizeof(int16_t)*synth->wave_length, "PCM table");
        cSynthBuildTriangleWave(synth->triangle_wave_table, synth->wave_length);
        
        synth->noise_table = cAllocatorAlloc(sizeof(int16_t)*synth->noise_length, "PCM table");
        cSynthBuildNoise(synth->noise_table, synth->noise_length);
        
        synth->custom_table = cAllocatorAlloc(sizeof(int16_t)*synth->wave_length, "PCM table");
        for (i = 0; i < synth->wave_length; i++) {
            synth->custom_table[i] = 0;
        }
    
        synth->voices = cAllocatorAlloc(sizeof(struct CVoice*)*synth->max_voices, "Synth Voices");
        for(i = 0; i < synth->max_voices; i++) {
            synth->voices[i] = NULL;
        }
                
        synth->instruments = cAllocatorAlloc(sizeof(struct CInstrument*)*synth->max_instruments, "Synth Instruments");
        for(i = 0; i < synth->max_instruments; i++) {
            synth->instruments[i] = cSynthNewInstrument(synth);
            synth->instruments[i]->instrument_number = i;
        }
        
        for (i = 0; i < synth->max_voices; i++) {
            struct CVoice *v = cSynthNewVoice(synth, synth->sine_wave_table);
            v->active = false;
            v->int_id = i;
            synth->voices[i] = v;
        }
        
        synth->custom_instrument = cSynthNewInstrument(synth);
        synth->custom_instrument->adsr[0]->amp = 0.0;
        synth->custom_instrument->adsr[0]->pos = 0.0;
        synth->custom_instrument->adsr[1]->amp = 1.0;
        synth->custom_instrument->adsr[1]->pos = 0.4;
        synth->custom_instrument->adsr[2]->amp = 0.0;
        synth->custom_instrument->adsr[2]->pos = 0.6;
        synth->custom_instrument->adsr[3]->amp = -1.0;
        synth->custom_instrument->adsr[3]->pos = 0.8;
        synth->custom_instrument->adsr[4]->amp = 0.0;
        synth->custom_instrument->adsr[4]->pos = 1.0;
        
        cSynthWriteCustomTableFromNodes(synth);
        
        /* init track */
        synth->track = cAllocatorAlloc(synth->track_width * sizeof(int)*synth->patterns_height, "Synth Tracks");
        if(synth->track == NULL) {
            fprintf(stderr, "synth track out of memory\n");
        }
        for(a = 0; a < synth->patterns_height; a++) {
            if(synth->track != NULL) {
                synth->track[a] = cAllocatorAlloc(synth->track_width * sizeof(struct CTrackNode**), "Synth Track 1");
                for(i = 0; i < synth->track_width; i++) {
                    if(synth->track[a] != NULL) {
                        synth->track[a][i] = cAllocatorAlloc(synth->track_max_height * sizeof(struct CTrackNode*), "Synth Track 2");
                        if(synth->track[a][i] == NULL) {
                            fprintf(stderr, "synth track out of memory\n");
                        }
                    }
                }
            }
        }
        
        for(a = 0; a < synth->patterns_height; a++) {
            if(synth->track != NULL) {
                for(s_x = 0; s_x < synth->track_width; s_x++) {
                    for(s_y = 0; s_y < synth->track_max_height; s_y++) {
                        if(synth->track[a] != NULL && synth->track[a][s_x] != NULL) {
                            synth->track[a][s_x][s_y] = NULL;
                        } else {
                            fprintf(stderr, "error not setting null! track");
                        }
                    }
                }
            }
        }

        /* swap track */
        synth->swap_track = cAllocatorAlloc(synth->track_width * sizeof(struct CTrackNode**), "Synth swap track 1");
        for(i = 0; i < synth->track_width; i++) {
            if(synth->swap_track != NULL) {
                synth->swap_track[i] = cAllocatorAlloc(synth->track_max_height * sizeof(struct CTrackNode*), "Synth swap track 2");
                if(synth->swap_track[i] == NULL) {
                    fprintf(stderr, "synth swap track out of memory\n");
                }
            }
        }
        
        for(s_x = 0; s_x < synth->track_width; s_x++) {
            for(s_y = 0; s_y < synth->track_max_height; s_y++) {
                if(synth->swap_track != NULL && synth->swap_track[s_x] != NULL) {
                    synth->swap_track[s_x][s_y] = NULL;
                } else {
                    fprintf(stderr, "error not setting null!");
                }
            }
        }
        
        
        /* copy track */
        
        synth->copy_track = cAllocatorAlloc(synth->track_width * sizeof(struct CTrackNode**), "Synth copy track 1");
        for(i = 0; i < synth->track_width; i++) {
            if(synth->copy_track != NULL) {
                synth->copy_track[i] = cAllocatorAlloc(synth->track_max_height * sizeof(struct CTrackNode*), "Synth copy track 2");
                if(synth->copy_track[i] == NULL) {
                    fprintf(stderr, "synth swap track out of memory\n");
                }
            }
        }
        
        for(s_x = 0; s_x < synth->track_width; s_x++) {
            for(s_y = 0; s_y < synth->track_max_height; s_y++) {
                if(synth->copy_track != NULL && synth->copy_track[s_x] != NULL) {
                    synth->copy_track[s_x][s_y] = NULL;
                } else {
                    fprintf(stderr, "error not setting null! copy");
                }
            }
        }


        /* instrument effects */
        synth->instrument_effects = cAllocatorAlloc(synth->max_instrument_effects * sizeof(struct CTrackNode**), "Synth instrument effects 1");
        for(i = 0; i < synth->max_instrument_effects; i++) {
            if(synth->instrument_effects != NULL) {
                synth->instrument_effects[i] = cAllocatorAlloc(synth->max_instrument_effects * sizeof(struct CTrackNode*), "Synth instrument effects 2");
                if(synth->instrument_effects[i] == NULL) {
                    fprintf(stderr, "synth swap track out of memory\n");
                }
            }
        }
        
        for(s_x = 0; s_x < synth->max_instrument_effects; s_x++) {
            for(s_y = 0; s_y < synth->max_instrument_effects; s_y++) {
                if(synth->instrument_effects != NULL && synth->instrument_effects[s_x] != NULL) {
                    synth->instrument_effects[s_x][s_y] = NULL;
                } else {
                    fprintf(stderr, "error not setting null! ins");
                }
            }
        }

        
        /* init voice types and settings */
        
        synth->patterns_and_voices = cAllocatorAlloc(synth->patterns_and_voices_width * sizeof(int*), "Synth Patterns and Voices");
        if(synth->patterns_and_voices == NULL) {
            fprintf(stderr, "out of memory\n");
        } else {
            for(i = 0; i < synth->patterns_and_voices_width; i++) {
                synth->patterns_and_voices[i] = cAllocatorAlloc(synth->patterns_and_voices_height * sizeof(int), "Synth Patterns and Voices");
                if(synth->patterns_and_voices[i] == NULL)
                {
                    fprintf(stderr, "out of memory\n");
                }
            }
            
            for(s_x = 0; s_x < synth->patterns_and_voices_width; s_x++) {
                for(s_y = 0; s_y < synth->patterns_and_voices_height; s_y++) {
                    if(synth->patterns_and_voices != NULL && synth->patterns_and_voices[s_x] != NULL) {
                        synth->patterns_and_voices[s_x][s_y] = 0;
                    }
                }
            }
            
            for(s_x = 0; s_x < synth->patterns_and_voices_width; s_x++) {
                for(s_y = 0; s_y < synth->patterns_and_voices_height; s_y++) {
                    if(synth->patterns_and_voices != NULL && synth->patterns_and_voices[s_x] != NULL) {
                        if(s_x == 0 && s_y == 0) {
                            synth->patterns_and_voices[s_x][s_y] = 0;
                        } else if(s_x == 1 && s_y == 0) {
                            synth->patterns_and_voices[s_x][s_y] = 1;
                        } else if(s_x == 2 && s_y == 0) {
                            synth->patterns_and_voices[s_x][s_y] = 2;
                        } else if(s_x == 3 && s_y == 0) {
                            synth->patterns_and_voices[s_x][s_y] = 3;
                        } else if(s_x == 4 && s_y == 0) {
                            synth->patterns_and_voices[s_x][s_y] = 4;
                        } else if(s_x == 5 && s_y == 0) {
                            synth->patterns_and_voices[s_x][s_y] = 4;
                        }
                    }
                }
            }
        }
        
        /* patterns */
        synth->patterns = cAllocatorAlloc(synth->patterns_width * sizeof(int*), "Synth Patterns");
        if(synth->patterns == NULL) {
            fprintf(stderr, "out of memory\n");
        } else {
            for(i = 0; i < synth->patterns_width; i++) {
                synth->patterns[i] = cAllocatorAlloc(synth->patterns_height * sizeof(int), "Synth Patterns 2");
                if(synth->patterns[i] == NULL)
                {
                    fprintf(stderr, "out of memory\n");
                }
            }
            
            for(s_x = 0; s_x < synth->patterns_width; s_x++) {
                for(s_y = 0; s_y < synth->patterns_height; s_y++) {
                    if(synth->patterns != NULL && synth->patterns[s_x] != NULL) {
                        synth->patterns[s_x][s_y] = 0;
                    }
                }
            }
        }
        
        synth->voices[0]->waveform = synth->sine_wave_table;
        synth->voices[1]->waveform = synth->sawtooth_wave_table;
        synth->voices[2]->waveform = synth->square_wave_table;
        synth->voices[3]->waveform = synth->triangle_wave_table;
        synth->voices[4]->waveform = synth->noise_table;
        synth->voices[5]->waveform = synth->noise_table;
        
        synth->active_tracks = cAllocatorAlloc(sizeof(int) * synth->patterns_height, "active tracks");
        for (i = 0; i < synth->patterns_height; i++) {
            synth->active_tracks[i] = 0;
        }
        
        // activate first track
        synth->active_tracks[0] = 1;
        
        // no track or voice is solo to begin with.
        synth->solo_track = -1;
        synth->solo_voice = -1;
        
        /* tempo */
        
        synth->tempo_map = cAllocatorAlloc(synth->tempo_width * sizeof(struct CTempoNode**), "Synth tempo 1");
        for(i = 0; i < synth->tempo_width; i++) {
            if(synth->tempo_map != NULL) {
                synth->tempo_map[i] = cAllocatorAlloc(synth->tempo_height * sizeof(struct CTempoNode*), "Synth tempo 2");
                if(synth->tempo_map[i] == NULL) {
                    fprintf(stderr, "synth tempo out of memory\n");
                }
            }
        }
        
        for(s_x = 0; s_x < synth->tempo_width; s_x++) {
            for(s_y = 0; s_y < synth->tempo_height; s_y++) {
                if(synth->tempo_map != NULL && synth->tempo_map[s_x] != NULL) {
                    synth->tempo_map[s_x][s_y] = cSynthNewTempoNode();
                    if (s_y == 0) {
                        synth->tempo_map[s_x][s_y]->bpm = synth->default_bpm;
                    } else if(s_y == 1 || (s_x == 0 && s_y < 5 && s_y > 0)) {
                        synth->tempo_map[s_x][s_y]->active = true;
                        synth->tempo_map[s_x][s_y]->ticks = 4;
                    }
                } else {
                    fprintf(stderr, "error allocating tempo_node, setting null.");
                }
            }
        }
        synth->current_tempo_column = 0;
        
        
        /* wavetable */
        
        synth->wavetable_map = cAllocatorAlloc(synth->wavetable_width * sizeof(struct CWavetableNode**), "Synth wavetable 1");
        for(i = 0; i < synth->wavetable_width; i++) {
            if(synth->wavetable_map != NULL) {
                synth->wavetable_map[i] = cAllocatorAlloc(synth->wavetable_height * sizeof(struct CWavetableNode*), "Synth wavetable 2");
                if(synth->wavetable_map[i] == NULL) {
                    fprintf(stderr, "synth tempo out of memory\n");
                }
            }
        }
        
        for(s_x = 0; s_x < synth->wavetable_width; s_x++) {
            for(s_y = 0; s_y < synth->wavetable_height; s_y++) {
                if(synth->wavetable_map != NULL && synth->wavetable_map[s_x] != NULL) {
                    synth->wavetable_map[s_x][s_y] = cSynthNewWavetableNode();
                    if (s_y == 0) {
                        synth->wavetable_map[s_x][s_y]->speed = 50;
                    } else if(s_y == 2) {
                        synth->wavetable_map[s_x][s_y]->active = true;
                        synth->wavetable_map[s_x][s_y]->value = 1;
                    }
                } else {
                    fprintf(stderr, "error allocating tempo_node, setting null.");
                }
            }
        }

        synth->mod_adsr_cursor = cSynthGetRelativeModifier(synth->base_mod_adsr_cursor, synth->sample_rate, synth->interleaved, false);
        synth->mod_noteoff_slope = cSynthGetRelativeModifier(synth->base_mod_noteoff_slope, synth->sample_rate, synth->interleaved, false);
        synth->mod_advance_amp_targets = cSynthGetRelativeModifier(synth->base_mod_advance_amp_targets, synth->sample_rate, synth->interleaved, false);
        synth->mod_pwm_speed = cSynthGetRelativeModifier(synth->base_mod_pwm_speed, synth->sample_rate, synth->interleaved, false);
        synth->mod_vibrato_speed = cSynthGetRelativeModForChunks(synth->base_mod_vibrato_speed, synth->sample_rate, synth->chunk_size, synth->interleaved, false);
        synth->mod_vibrato_depth = 0.000005;
        synth->mod_pitch_1 = cSynthGetRelativeModForChunks(synth->base_mod_pitch_1, synth->sample_rate, synth->chunk_size, synth->interleaved, false);
        synth->mod_pitch_2 = cSynthGetRelativeModForChunks(synth->base_mod_pitch_2, synth->sample_rate, synth->chunk_size, synth->interleaved, false);
        synth->mod_arp_speed = cSynthGetRelativeModForChunks(synth->base_mod_arp_speed, synth->sample_rate, synth->chunk_size, synth->interleaved, false);
        synth->mod_portamento_speed = cSynthGetRelativeModForChunks(synth->base_mod_portamento_speed, synth->sample_rate, synth->chunk_size, synth->interleaved, false);
        synth->mod_detune = cSynthGetRelativeModForChunks(synth->base_mod_detune, synth->sample_rate, synth->chunk_size, synth->interleaved, false);
        
    } else {
        fprintf(stderr, "CSynth error: cannot init data");
    }
}

static struct CTempoNode *cSynthNewTempoNode(void) {
    
    struct CTempoNode *t = cAllocatorAlloc(sizeof(struct CTempoNode), "CTempoNode");
    t->active = false;
    t->ticks = 1;
    t->bpm = 0;
    return t;
}

static struct CWavetableNode *cSynthNewWavetableNode(void) {
    
    struct CWavetableNode *t = cAllocatorAlloc(sizeof(struct CWavetableNode), "CWavetableNode");
    t->active = false;
    t->value = 1;
    t->speed = 1;
    return t;
}

static void cSynthSetWaveformsFromPattern(struct CSynthContext *synth) {
    
    for(int x = 0; x < synth->track_width; x++) {
        int current_waveform = synth->patterns_and_voices[x][0];
        if(current_waveform == 0) {
            synth->voices[x]->waveform = synth->sine_wave_table;
        }
        if(current_waveform == 1) {
            synth->voices[x]->waveform = synth->sawtooth_wave_table;
        }
        if(current_waveform == 2) {
            synth->voices[x]->waveform = synth->square_wave_table;
        }
        if(current_waveform == 3) {
            synth->voices[x]->waveform = synth->triangle_wave_table;
        }
        if(current_waveform == 4) {
            synth->voices[x]->waveform = synth->noise_table;
        }
        if(current_waveform == 5) {
            synth->voices[x]->waveform = synth->custom_table;
        }
    }
}

static void cSynthChangeWaveformForChannel(struct CSynthContext *synth, int channel, int waveform) {
    
    if(channel > -1 && channel < synth->max_voices
       && waveform > -1 && waveform < 6) {
        synth->patterns_and_voices[channel][0] = waveform;
        
        if(waveform == 0) {
            synth->voices[channel]->waveform = synth->sine_wave_table;
        }
        if(waveform == 1) {
            synth->voices[channel]->waveform = synth->sawtooth_wave_table;
        }
        if(waveform == 2) {
            synth->voices[channel]->waveform = synth->square_wave_table;
        }
        if(waveform == 3) {
            synth->voices[channel]->waveform = synth->triangle_wave_table;
        }
        if(waveform == 4) {
            synth->voices[channel]->waveform = synth->noise_table;
        }
        if(waveform == 5) {
            synth->voices[channel]->waveform = synth->custom_table;
        }
    }
}

void cSynthMoveNotes(struct CSynthContext *synth, bool up, bool down, int cursor_x, int cursor_y, int pattern) {
    
    // copy notes to swap track
    for(int y = 0; y < synth->track_height; y++) {
        struct CTrackNode *node = synth->track[pattern][cursor_x][y];
        synth->swap_track[cursor_x][y] = node;
        synth->track[pattern][cursor_x][y] = NULL;
    }
    
    if(up) {
        for(int y = 0; y < synth->track_height; y++) {
            if(y > -1) {
                if(cursor_y > y) {
                    if(y == cursor_y-1) {
                        synth->swap_track[cursor_x][y] = cAllocatorFree(synth->swap_track[cursor_x][y]);
                    } else {
                        synth->track[pattern][cursor_x][y] = synth->swap_track[cursor_x][y];
                    }
                } else {
                    if(y-1 > -1) {
                        synth->track[pattern][cursor_x][y-1] = synth->swap_track[cursor_x][y];
                    }
                }
            } else {
                synth->swap_track[cursor_x][y] = cAllocatorFree(synth->swap_track[cursor_x][y]);
            }
        }
    } else {
        for(int y = 0; y < synth->track_height; y++) {
            if(y+1 < synth->track_height) {
                if(cursor_y > y) {
                    synth->track[pattern][cursor_x][y] = synth->swap_track[cursor_x][y];
                } else {
                    synth->track[pattern][cursor_x][y+1] = synth->swap_track[cursor_x][y];
                }
            } else {
                synth->swap_track[cursor_x][y] = cAllocatorFree(synth->swap_track[cursor_x][y]);
            }
        }
    }
    
    for(int y = 0; y < synth->track_height; y++) {
        synth->swap_track[cursor_x][y] = NULL;
    }
}

void cSynthAddTrackNodeParams(struct CSynthContext *synth, int current_track, int x, int y, int instrument, char effect, char effect_param1, char effect_param2) {
    
    int pattern = synth->patterns[x][current_track];
    struct CTrackNode *t = synth->track[pattern][x][y];
    if(t == NULL) {
        t = cSynthNewTrackNode();
        t->tone_active = false;
    }
    
    if(instrument != -1) {
        t->instrument_nr = instrument;
        t->instrument = synth->instruments[instrument];
        if(synth->debuglog) { printf("setting instrument:%d\n", instrument); }
        if(synth->debuglog) { printf("pattern:%d x:%d y:%d\n", pattern, x, y); }
    }
        
    if(effect != -1) {
        t->effect = cSynthGetCharFromParam(effect);
        t->effect_value = effect;
    }
        
    if(effect_param1 != -1) {
        t->effect_param1 = cSynthGetCharFromParam(effect_param1);
        t->effect_param1_value = effect_param1;
    }
        
    if(effect_param2 != -1) {
        t->effect_param2 = cSynthGetCharFromParam(effect_param2);
        t->effect_param2_value = effect_param2;
    }
        
    synth->track[pattern][x][y] = t;
}

void cSynthRemoveTrackNodeParams(struct CSynthContext *synth, int current_track, int x, int y, bool instrument, bool effect, bool effect_param1, bool effect_param2) {
    
    int pattern = synth->patterns[x][current_track];
    struct CTrackNode *t = synth->track[pattern][x][y];
    if(t == NULL) {
        return;
    }
    
    if(instrument) {
        t->instrument_nr = 0;
        t->instrument = NULL;
    }
    
    if(effect) {
        t->effect = '-';
        t->effect_value = -1;
    }
    
    if(effect_param1) {
        t->effect_param1 = '-';
        t->effect_param1_value = -1;
    }
    
    if(effect_param2) {
        t->effect_param2 = '-';
        t->effect_param2_value = -1;
    }
    
    synth->track[pattern][x][y] = t;
}


void cSynthTurnOffSustain(struct CSynthContext *synth) {
    
    for (int i = 0; i < synth->max_voices; i++) {
        synth->voices[i]->sustain_active = false;
    }
}

void cSynthAddTrackNode(struct CSynthContext *synth, int instrument_nr, int current_track, int x, int y, bool editing, bool preview, int tone, bool playing) {
    
    if(x > -1 && x < synth->track_width
       && y > -1 && y < synth->track_height) {
        
        struct CInstrument *instrument = synth->instruments[instrument_nr];
        
        if(editing) {
            int pattern = synth->patterns[x][current_track];
            struct CTrackNode *t = synth->track[pattern][x][y];
            if(t != NULL) {
                t->tone = tone;
                
                if(t->instrument == NULL) {
                    // don't overwrite existing instrument! only set if null.
                    t->instrument_nr = synth->current_instrument;
                    t->instrument = instrument;
                }
                t->tone_active = true;
            } else {
                t = cSynthNewTrackNode();
                t->tone = tone;
                t->instrument = instrument;
                t->instrument_nr = synth->current_instrument;
                t->tone_active = true;
                synth->track[pattern][x][y] = t;
            }
            
        }
        
        if(preview && synth->preview_enabled /*&& !playing*/ && synth->sustain_active && !synth->preview_started) {
            
            synth->preview_started = true;
            
            int x_node = x;
            synth->voices[x_node]->tone = tone;
            synth->voices[x_node]->adsr_cursor = 0;
            synth->voices[x_node]->preview_slope = true;

            synth->voices[x_node]->note_on = true;
            synth->voices[x_node]->active = true;
            synth->voices[x_node]->instrument = instrument;
            cSynthResetAllEffects(synth->voices[x_node]);
            synth->voices[x_node]->sustain_active = synth->sustain_active;
            
            // apply instrument effects
            struct CTrackNode *pre_effect_node = NULL;
            for (int pre_effect = 0; pre_effect < synth->max_instrument_effects; pre_effect++) {
                if(instrument != NULL) {
                    if(synth->instrument_effects[instrument->instrument_number][pre_effect] != NULL) {
                        pre_effect_node = synth->instrument_effects[instrument->instrument_number][pre_effect];
                    }
                    if(pre_effect_node != NULL && pre_effect_node->effect_value > -1) {
                        pre_effect_node->tone = tone;
                        pre_effect_node->tone_active = true;
                        pre_effect_node->instrument = instrument;
                        pre_effect_node->instrument_nr = instrument->instrument_number;
                        cSynthVoiceSetEffect(synth, synth->voices[x_node], pre_effect_node, synth->voices[x_node]->last_preview_tone);
                    }
                }
            }
            
            synth->voices[x_node]->last_preview_tone = tone;
            cSynthPostEffectFix(synth, synth->voices[x_node], pre_effect_node);
        }
    }
}

void cSynthRemoveTrackNode(struct CSynthContext *synth, int current_track, int x, int y) {

    if(x > -1 && x < synth->track_width
       && y > -1 && y < synth->track_height) {
        int pattern = synth->patterns[x][current_track];
        struct CTrackNode *t = synth->track[pattern][x][y];
        if(t != NULL) {
            t->tone_active = false;
        }
    }
}

void cSynthUpdateTrackCursor(struct CSynthContext *synth, int x, int y) {
    
    if(x >= synth->track_width) {
        x = 0;
    }
    
    if(x < 0) {
        x = synth->track_width-1;
    }
    
    if(y >= synth->track_height) {
        y = 0;
    }
    
    if(y < 0) {
        y = synth->track_height-1;
    }
    
    if(x > -1 && x < synth->track_width
       && y > -1 && y < synth->track_height) {
        synth->track_cursor_x = x;
        synth->track_cursor_y = y;
    }
}

int cSynthGetNextActiveTrack(int current_track, struct CSynthContext *synth, bool forward) {
    
    if(synth->solo_track > -1) {
        return  synth->solo_track;
    }
    
    bool next_track_set = false;
    int next_track = 0;
    
    if(forward) {
        for(int i = 0; i < synth->patterns_height; i++) {
            if(i > current_track && synth->active_tracks[i] == 1) {
                next_track_set = true;
                next_track = i;
                break;
            }
        }
        
        if(!next_track_set) {
            for(int i = 0; i < synth->patterns_height; i++) {
                if(current_track >= i && synth->active_tracks[i] == 1) {
                    next_track_set = true;
                    next_track = i;
                    break;
                }
            }
        }
    } else {
        for(int i = synth->patterns_height-1; i > -1; i--) {
            if(i < current_track && synth->active_tracks[i] == 1) {
                next_track_set = true;
                next_track = i;
                break;
            }
        }
        
        if(!next_track_set) {
            for(int i = synth->patterns_height-1; i > -1; i--) {
                if(current_track <= i && synth->active_tracks[i] == 1) {
                    next_track_set = true;
                    next_track = i;
                    break;
                }
            }
        }
    }
    
    if(next_track_set) {
        return next_track;
    } else {
        return 0;
    }
}

static void cSynthLookAheadForActivatingSlope(struct CSynthContext *synth) {
    
    int i;
    int pattern;
    bool d_is_detected = false;
    struct CTrackNode *t = NULL;
    int next_row;
    int next_track;
    
    double p = synth->track_progress_int - synth->track_progress;
    if (p < -0.90) {
        /* check what the next row contains and apply a slope if amp needs to be lowered for voices to avoic sound clipping */

        for (i = 0; i < synth->track_width; i++) {
            next_row = synth->track_progress_int+1;
            next_track = synth->current_track;
            if(next_row >= synth->track_height) {
                next_track = cSynthGetNextActiveTrack(synth->current_track, synth, true);
                next_row = 0;
            }
            pattern = synth->patterns[i][next_track];
            t = synth->track[pattern][i][next_row];
            if(t != NULL) {
                if(t->tone_active) {
                    synth->voices[i]->noteoff_slope = true;
                }
                if(t->effect_value == 13 && t->effect_param1_value == -1 && t->effect_param2_value == -1) {
                    synth->voices[i]->noteoff_slope = true;
                    d_is_detected = true;
                }
            }
        }
        
        if(d_is_detected) {
            for (i = 0; i < synth->track_width; i++) {
                synth->voices[i]->noteoff_slope = false;
                next_row = synth->track_height;
                next_track = synth->current_track;
                if(next_row >= synth->track_height) {
                    next_track = cSynthGetNextActiveTrack(synth->current_track, synth, true);
                    next_row = 0;
                }
                pattern = synth->patterns[i][next_track];
                t = synth->track[pattern][i][next_row];
                if(t != NULL) {
                    if(t->tone_active) {
                        synth->voices[i]->noteoff_slope = true;
                    }
                }
            }
        }
    }
}


void cSynthAdvanceTrackProgress(struct CSynthContext *synth) {
    
    if(synth->track_progress_int >= synth->track_height) {
        
        if(synth->debuglog) { printf("track_progress_int:%d track_height:%d\n", synth->track_progress_int, synth->track_height); }
        
        synth->track_progress = synth->track_progress - synth->track_height;
        synth->track_progress_int = (int)synth->track_progress;
        synth->track_advance = true;
        int last_track = synth->current_track;
        synth->current_track = cSynthGetNextActiveTrack(synth->current_track, synth, true);
        
        // check if we have an pending switch of tempo columns.
        if(synth->pending_tempo_column > -1) {
            if(synth->debuglog) { printf("resetting tempo index\n"); }
            synth->current_tempo_column = synth->pending_tempo_column;
            synth->pending_tempo_column = -1;
            cSynthResetTempoIndex(synth);
            synth->tempo_skip_step = true;
        }
        
        if(synth->current_track <= last_track) {
            if(synth->debuglog) { printf("looped. current:%d last_track:%d\n", synth->current_track, last_track); }
            synth->looped = true;
            cSynthResetOnLoopBack(synth);
        } else {
            if(synth->debuglog) { printf("-- NOT looped. current:%d last_track:%d\n", synth->current_track, last_track); }
        }
    }
}

void cSynthTempoIncrement(struct CSynthContext *synth) {
    
    int next_index = -1;
    bool next_index_set = false;
    
    if(synth->tempo_skip_step) {
        next_index = synth->tempo_index;
        next_index_set = true;
        synth->tempo_skip_step = false;
        if(synth->debuglog) { printf("=== tempo skip step set to false.\n"); }
    } else {
    
        // find next active node for current column.
        for(int i = 1; i < synth->tempo_height; i++) {
            struct CTempoNode *node = synth->tempo_map[synth->current_tempo_column][i];
            if(node->active) {
                if(i > synth->tempo_index && !next_index_set) {
                    next_index = i;
                    next_index_set = true;
                }
            }
        }
        if(!next_index_set) {
            for(int i = 1; i < synth->tempo_height; i++) {
                struct CTempoNode *node = synth->tempo_map[synth->current_tempo_column][i];
                if(node->active) {
                    if(!next_index_set) {
                        next_index = i;
                        next_index_set = true;
                    }
                }
            }
        }
    }
    
    if(!next_index_set) {
        if(synth->debuglog) { printf("tempo error: could not set next index.\n"); }
    } else {
        synth->tempo_index = next_index;
    }
    
    char ticks_value = synth->tempo_map[synth->current_tempo_column][synth->tempo_index]->ticks;
    int ticks_int = (int)ticks_value;
    synth->ticks_modifier = 1.0 / (double)ticks_int;
}

void cSynthUpdateTicksModifier(struct CSynthContext *synth) {
    
    char ticks_value = synth->tempo_map[synth->current_tempo_column][synth->tempo_index]->ticks;
    int ticks_int = (int)ticks_value;
    synth->ticks_modifier = 1.0 / (double)ticks_int;
}

void cSynthAdvanceTrack(struct CSynthContext *synth, long samples) {

    int i;
    int previous_tone;
    struct CVoice *v = NULL;
    double bpm = synth->tempo_map[synth->current_tempo_column][0]->bpm;
    double sample_time_modifier = (double)bpm/(((double)synth->sample_rate*2.0)*60);
    if(!synth->interleaved) {
        sample_time_modifier /= 2;
    }
    
    if (synth->ticks_modifier < 0) {
        if(synth->debuglog) { printf("ticks_mod is not set. initing."); }
        cSynthUpdateTicksModifier(synth);
    }
    double speed = sample_time_modifier * (1 / synth->ticks_modifier);
    int p = synth->track_progress_int;
    double speed_inc = ((double)samples*speed);
   
    synth->track_progress += speed_inc;
    synth->track_progress_int = (int)synth->track_progress;
    
    cSynthAdvanceTrackProgress(synth);
    cSynthLookAheadForActivatingSlope(synth);
    
    if(synth->track_progress_int > p || synth->track_advance) {
        
        cSynthTempoIncrement(synth);
        synth->track_advance = false;
        synth->needs_redraw = true;
        
        for (i = 0; i < synth->track_width; i++) {
            // remove preview_slopes
            v = synth->voices[i];
            v->preview_slope = false;
        }
        
        for (i = 0; i < synth->track_width; i++) {
            int pattern = synth->patterns[i][synth->current_track];
            struct CTrackNode *t = synth->track[pattern][i][synth->track_progress_int];
            v = synth->voices[i];
            
            if(t != NULL) {
                // Dxx - jump to next pattern.
                if(t->effect_value == 13 && v->muted == false && t->effect_param1_value == -1 && t->effect_param2_value == -1) {
                    if(synth->debuglog) { printf("jump to next pattern\n"); }
                    synth->track_progress_int = synth->track_height;
                    synth->track_progress = synth->track_progress_int;
                    return;
                // D11 - jump to next pattern and reset tempo seq.
                } else if(t->effect_value == 13 && v->muted == false && t->effect_param1_value == 1 && t->effect_param2_value == 1) {
                    if(synth->debuglog) { printf("jump to next pattern and reset tempo seq\n"); }
                    synth->track_progress_int = synth->track_height;
                    synth->track_progress = synth->track_progress_int;
                    cSynthResetTempoIndex(synth);
                    synth->tempo_skip_step = true;
                    return;
                // D1x - reset tempo seq.
                } else if(t->effect_value == 13 && v->muted == false && t->effect_param1_value == 1 && t->effect_param2_value == -1) {
                    if(synth->debuglog) { printf("reset tempo seq\n"); }
                    cSynthResetTempoIndex(synth);
                    cSynthUpdateTicksModifier(synth);
                // D2x - switch tempo_seq column. x = tempo seq column (0-5)
                } else if(t->effect_value == 13 && v->muted == false && t->effect_param1_value == 2) {
                    if(t->effect_param2_value > -1 && t->effect_param2_value < synth->tempo_width) {
                        synth->current_tempo_column = t->effect_param2_value;
                        cSynthResetTempoIndex(synth);
                        cSynthUpdateTicksModifier(synth);
                    }
                }
            }
            
            /* reset slopes */
            v->noteoff_slope = false;
            v->noteoff_slope_value = 1;
            
            if(t != NULL && t->tone_active && t->instrument != NULL) {
               
                /* set previous tone before tone. */
                v->previous_tone = v->tone;
                v->instrument = t->instrument;
                v->adsr_cursor = 0;
                v->note_on = true;
                v->tone = t->tone;
                v->last_valid_tone = t->tone;
            }
            
            previous_tone = v->previous_tone;
            
            if(t != NULL) {
                
                if(t->effect_param1_value > -1) {
                    v->previous_effect_param1 = t->effect_param1_value;
                }
                
                if(t->effect_param2_value > -1) {
                    v->previous_effect_param2 = t->effect_param2_value;
                }
            
                if(t->tone_active) {
                    v->arpeggio_tone = t->tone;
                }
            
                if(t != NULL && t->tone_active) {
                    cSynthResetAllEffects(v);
                    
                    if(v->instrument != NULL) {
                        // update previous instrument number.
                        v->previous_instrument_number = v->instrument->instrument_number;
                    }
                    
                    for (int pre_effect = 0; pre_effect < synth->max_instrument_effects; pre_effect++) {
                        if(v->instrument != NULL) {
                            struct CTrackNode *pre_effect_node = synth->instrument_effects[v->instrument->instrument_number][pre_effect];
                            if(pre_effect_node != NULL && pre_effect_node->effect_value > -1) {
                                pre_effect_node->tone = t->tone;
                                pre_effect_node->tone_active = t->tone_active;
                                pre_effect_node->instrument = t->instrument;
                                pre_effect_node->instrument_nr = t->instrument_nr;
                                cSynthVoiceSetEffect(synth, v, pre_effect_node, previous_tone);
                            }
                        }
                    }
                    
                    // reset waveform to original if wavetable is inactive.
                    if(v->wavetable_previous_table != NULL) {
                        v->waveform = v->wavetable_previous_table;
                    }
                }
                
                /* set effect from track */
                cSynthVoiceSetEffect(synth, v, t, previous_tone);
                
                /* add additional effects stuff */
                cSynthPostEffectFix(synth, v, t);
            }
        }
    }
}

static void cSynthPostEffectFix(struct CSynthContext *synth, struct CVoice *v, struct CTrackNode *t) {
    
    if(v->downsample_sweep_down) {
        
        v->downsample_sweep_slope = -1;
        
        if(v->last_downsample_effect1 > 0) {
            v->downsample_sweep_slope = v->last_downsample_effect1;
        } else if(v->last_downsample_effect2 > 0) {
            v->downsample_sweep_speed = v->last_downsample_effect2;
        } else {
            v->downsample_sweep_down = false;
        }
        
        if(v->downsample_sweep_speed < 1 && v->downsample_sweep_slope < 1) {
            v->downsample_sweep_down = false;
        }
        
        int limit = (int)(v->downsample_sweep_speed * synth->sample_rate/4);
        
        if(!synth->interleaved) {
            limit /= 2;
        }
        
        v->downsample_limit = limit;
        
        if(v->downsample_sweep_speed < 1) {
            limit = synth->sample_rate/4;
            if(!synth->interleaved) {
                limit /= 2;
            }
            v->downsample_limit = limit;
        }
        
        if(t != NULL && t->tone_active) {
            v->downsample_sweep_start = v->downsample_limit/2;
            v->downsample_count = 0;
        }
    }
    
    if(v->downsample_sweep_up) {
        
        v->downsample_sweep_slope = -1;
        
        if(v->last_downsample_effect1 > 0) {
            v->downsample_sweep_slope = v->last_downsample_effect1;
        } else if(v->last_downsample_effect2 > 0) {
            v->downsample_sweep_speed = v->last_downsample_effect2;
        } else {
            v->downsample_sweep_up = false;
        }
        
        if(v->downsample_sweep_speed < 1 && v->downsample_sweep_slope < 1) {
            v->downsample_sweep_up = false;
        }
        
        int limit = (int)(v->downsample_sweep_speed * synth->sample_rate/4);
        if(!synth->interleaved) {
            limit /= 2;
        }
        v->downsample_limit = limit;
        
        if(v->downsample_sweep_speed < 1) {
            limit = synth->sample_rate/4;
            if(!synth->interleaved) {
                limit /= 2;
            }
            v->downsample_limit = limit;
        }
        
        if(t != NULL && t->tone_active) {
            v->downsample_sweep_start = 0;
            v->downsample_count = 0;
        }
    }
}

static void cSynthVoiceSetEffect(struct CSynthContext *synth, struct CVoice *v, struct CTrackNode *t, int previous_tone) {
    
    // multiple effects can be added per row, so it's important not to have increments etc here, just set init values.
    
    if(t->effect_value == 0 || v->arpeggio_active) {
        if(t->effect_value == 0) {
            v->arpeggio_active = true;
            v->last_arpeggio_effect1 = v->previous_effect_param1;
            v->last_arpeggio_effect2 = v->previous_effect_param2;
            v->last_arpeggio_effect1 = t->effect_param1_value;
            v->last_arpeggio_effect2 = t->effect_param2_value;
            
            if(t->effect_param1_value == -1 && t->effect_param2_value == -1) {
                v->arpeggio_active = false;
            }
        }

        if(v->last_arpeggio_effect1 > -1 && v->last_arpeggio_effect2 > -1) {
            if(v->last_valid_tone > -1) {
                v->arpeggio_tone_0 = v->last_valid_tone;
                v->arpeggio_tone_1 = v->last_arpeggio_effect1+v->last_valid_tone;
                v->arpeggio_tone_2 = v->last_arpeggio_effect2+v->last_valid_tone;
            } else if(v->tone > -1){
                v->arpeggio_tone_0 = v->tone;
                v->arpeggio_tone_1 = v->last_arpeggio_effect1+v->tone;
                v->arpeggio_tone_2 = v->last_arpeggio_effect2+v->tone;
            } else if(previous_tone > -1) {
                v->arpeggio_tone_0 = previous_tone;
                v->arpeggio_tone_1 = v->last_arpeggio_effect1+previous_tone;
                v->arpeggio_tone_2 = v->last_arpeggio_effect2+previous_tone;
            } else {
                /* we don't have a tone to make the arp of. Cancel. */
            }
        } else {
            v->arpeggio_active = false;
        }
        
        v->arpeggio_count = 0;
    }

    // 1 arp speed
    if(t->effect_value == 1) {
        v->last_arp_speed_effect1 = t->effect_param1_value;
        v->last_arp_speed_effect2 = t->effect_param2_value;
        if(v->last_arp_speed_effect1 > -1 && v->last_arp_speed_effect2 > -1) {
            v->arp_speed = v->last_arp_speed_effect1 * v->last_arp_speed_effect2;
        } else if(v->last_arp_speed_effect1 > -1) {
            v->arp_speed = v->last_arp_speed_effect1;
        } else if(v->last_arp_speed_effect2 > -1) {
            v->arp_speed = v->last_arp_speed_effect2;
        } else {
            // use global arp speed
            v->arp_speed = -1;
        }
    }

    // 2 delay
    if(t->effect_value == 2) {
        double effect_scalar = 0.063;
        int length_scalar = 4096;
        if(t->effect_param1_value > 0 && t->effect_param2_value > 0) {
            v->param_delay_length = t->effect_param1_value * length_scalar;
            v->param_delay_decay = t->effect_param2_value * effect_scalar;
            v->delay_active = true;
        } else {
            v->delay_active = false;
        }
    }
    
    // 3 portamento
    if((t->tone_active && t->effect_value == 3)
       || (t->tone_active && v->portamento_active)) {

        int portamento_previous_tone = previous_tone;
        if (portamento_previous_tone == -1) {
            portamento_previous_tone = t->tone;
        }
        
        double f_previous_tone = portamento_previous_tone;
        double f_new_tone = t->tone;
        double diff = f_previous_tone - f_new_tone;
        
        v->portamento_diff = diff;
        v->portamento_active = true;
        v->portamento_original_diff = diff;

        if(t->effect_value == 3) {

            v->last_portamento_effect1 = t->effect_param1_value;
            v->last_portamento_effect2 = t->effect_param2_value;

            if(v->last_portamento_effect1 > -1 || v->last_portamento_effect2 > -1) {
                v->portamento_speed = v->last_portamento_effect1*v->last_portamento_effect2;
                if(v->last_portamento_effect1 <= 0 && v->last_portamento_effect2 > 0) {
                    v->portamento_speed = v->last_portamento_effect2;
                } else if(v->last_portamento_effect1 > 0 && v->last_portamento_effect2 <= 0) {
                    v->portamento_speed = v->last_portamento_effect1;
                }
            } else if(v->last_portamento_effect1 == -1 && v->last_portamento_effect2 == -1) {
                v->portamento_active = false;
            }
        }
        
        if(v->portamento_speed < 0) {
            v->portamento_speed = 0;
        }
        
        if(v->portamento_speed == 0) {
            v->portamento_active = false;
        }
        
        if (previous_tone == -1) {
            v->portamento_diff = 0;
            v->portamento_active = false;
            v->portamento_original_diff = 0;
        }
    }

    // 4 vibrato
    if(t->effect_value == 4) {
        v->last_vibrato_effect1 = t->effect_param1_value;
        v->last_vibrato_effect2 = t->effect_param2_value;
        v->vibrato_active = true;
        if(v->last_vibrato_effect1 > -1 && v->last_vibrato_effect2 > -1) {
            v->vibrato_speed = v->last_vibrato_effect1;
            v->vibrato_depth = v->last_vibrato_effect2;
        } else {
            v->vibrato_active = false;
        }
    }

    // 5 dist
    if(t->effect_value == 5) {
        v->last_dist_effect1 = t->effect_param1_value;
        v->last_dist_effect2 = t->effect_param2_value;
        v->dist_active = true;

        double pre_amp_scalar_mod = 0.1;
        double post_amp = 3.0;
        
        if(v->last_dist_effect1 > 0 && v->last_dist_effect2 > 0) {
            v->dist_amp_pre_clamp = (v->last_dist_effect1 * v->last_dist_effect2) * pre_amp_scalar_mod;
            v->dist_amp_post_clamp = post_amp;
        } else if(v->last_dist_effect1 > 0) {
            v->dist_amp_pre_clamp = v->last_dist_effect1 * pre_amp_scalar_mod;
            v->dist_amp_post_clamp = post_amp;
        } else if(v->last_dist_effect2 > 0) {
            v->dist_amp_pre_clamp = v->last_dist_effect2 * pre_amp_scalar_mod;
            v->dist_amp_post_clamp = post_amp;
        } else {
            v->dist_active = false;
            v->dist_amp_pre_clamp = 0;
            v->dist_amp_post_clamp =  0;
        }
    }
    
    // 6 FM
    if(t->effect_value == 6) {
        v->last_fm_effect1 = t->effect_param1_value;
        v->last_fm_effect2 = t->effect_param2_value;
        if(v->last_fm_effect1 > 0 && v->last_fm_effect2 > 0) {
            v->fm_active = true;
            double effect_scalar = 0.063; // convert 0-15 to 0-1
            v->fm_depth = v->last_fm_effect1 * effect_scalar; // convert 0-15 to 0-1
            v->fm_speed = v->last_fm_effect2;
        } else {
            v->fm_active = false;
        }
    }
    
    // 7 detune
    if(t->effect_value == 7) {
        double effect_max = 225;
        double combined = 0;
        if(t->effect_param1_value > 0 && t->effect_param2_value > 0) {
            combined = t->effect_param1_value * t->effect_param2_value;
        } else if(t->effect_param1_value > 0) {
            combined = t->effect_param1_value;
        } else if(t->effect_param2_value > 0) {
            combined = t->effect_param2_value;
        } else {
            v->detune_active = false;
        }
        
        if(combined != 0){
            v->detune_amount = (combined / effect_max)-0.3;
            v->detune_active = true;
        }
    }
    
    // 8 PWM
    if(t->effect_value == 8) {
        v->last_pwm_effect1 = t->effect_param1_value;
        v->last_pwm_effect2 = t->effect_param2_value;
        if(v->last_pwm_effect1 > 0 && v->last_pwm_effect2 < 1) {
            v->pwm_active = true;
            v->pwm_speed = 0;
            v->pwm_static_pos = ((synth->wave_length/16) * v->last_pwm_effect1)/4;
            if(v->pwm_static_pos > v->waveform_length) {
                v->pwm_static_pos = v->waveform_length;
            }
        } else if(v->last_pwm_effect2 > 0) {
            v->pwm_static_pos = 0;
            v->pwm_active = true;
            double effect_scalar = 0.063; // convert 0-15 to 0-1
            v->pwm_depth = v->last_pwm_effect1 * effect_scalar; // convert 0-15 to 0-1
            v->pwm_speed = v->last_pwm_effect2 * synth->mod_pwm_speed;
        } else {
            v->pwm_active = false;
        }
    }
    
    // 9 change waveform or activate wavetable lane.
    if(t->effect_value == 9) {
        if(t->effect_param1_value < 6 && t->effect_param1_value > -1) {
            v->wavetable_active = true;
            v->wavetable_lane = t->effect_param1_value * 2;
            v->wavetable_cursor = 2;
            v->wavetable_previous_table = v->waveform;
        } else if(t->effect_param2_value < 6) {
            // change the default waveform for the channel. If wavetable is reset, it should return to
            // this value.
            cSynthChangeWaveformForChannel(synth, v->int_id, t->effect_param2_value);
            v->wavetable_previous_table = v->waveform;
            synth->needs_redraw = true;
        }
    }

    // A
    if(t->effect_value == 10) {
        v->last_amp_effect1 = t->effect_param1_value;
        v->last_amp_effect2 = t->effect_param2_value;

        double effect_scalar = 0.063; // convert 0-15 to 0-1
        if(v->last_amp_effect1 > 0) {
            v->amp_target_left = v->last_amp_effect1 * effect_scalar;
        }
        if(v->last_amp_effect2 > 0) {
            v->amp_target_right = v->last_amp_effect2 * effect_scalar;
        }

        if(t->effect_param1_value < 1) {
            v->amp_target_left = 0;
        }

        if(t->effect_param2_value < 1) {
            v->amp_target_right = 0;
        }

        if(v->amp_left > v->amp_target_left) {
            v->amp_target_left_higher = false;
            v->amp_target_left_reached = false;
        } else if(v->amp_left < v->amp_target_left) {
            v->amp_target_left_higher = true;
            v->amp_target_left_reached = false;
        } else {
            // same as target
        }

        if(v->amp_right > v->amp_target_right) {
            v->amp_target_right_higher = false;
            v->amp_target_right_reached = false;
        } else if(v->amp_right < v->amp_target_right) {
            v->amp_target_right_higher = true;
            v->amp_target_right_reached = false;
        } else {
            // same as target
        }
    }

    // B downsample sweep down
    if(t->effect_value == 11) {
        v->downsample_sweep_down = true;
        v->downsample_sweep_up = false;
        v->downsample_next_sample = true;
        v->last_downsample_effect1 = t->effect_param1_value;
        v->last_downsample_effect2 = t->effect_param2_value;
    }

    // C downsample sweep up
    if(t->effect_value == 12) {
        v->downsample_sweep_up = true;
        v->downsample_sweep_down = false;
        v->downsample_next_sample = true;
        v->last_downsample_effect1 = t->effect_param1_value;
        v->last_downsample_effect2 = t->effect_param2_value;
    }

    // E
    if(t->effect_value == 14) {

        if(t->effect_param1_value > -1 || t->effect_param2_value > -1) {
            v->last_pitch_effect1 = t->effect_param1_value;
            v->last_pitch_effect2 = t->effect_param2_value;

            v->pitch_up_active = true;
            v->pitch_down_active = false;
            
            if(v->last_pitch_effect1 > -1) {
                v->pitch_speed1 = v->last_pitch_effect1;
            } else {
                v->pitch_speed1 = 0;
            }
            if(v->last_pitch_effect2 > -1) {
                v->pitch_speed2 = v->last_pitch_effect2;
            } else {
                v->pitch_speed2 = 0;
            }
        }
    }

    // F
    if(t->effect_value == 15) {
        
        if(t->effect_param1_value > -1 || t->effect_param2_value > -1) {
            v->last_pitch_effect1 = t->effect_param1_value;
            v->last_pitch_effect2 = t->effect_param2_value;

            v->pitch_down_active = true;
            v->pitch_up_active = false;

            if(v->last_pitch_effect1 > -1) {
                v->pitch_speed1 = -v->last_pitch_effect1;
            } else {
                v->pitch_speed1 = 0;
            }
            
            if(v->last_pitch_effect2 > -1) {
                v->pitch_speed2 = -v->last_pitch_effect2;
            } else {
                v->pitch_speed2 = 0;
            }
        }
    }
    
    // G bitcrush
    if(t != NULL) {
        if(t->effect_value == 16) {
            if(t->effect_param1_value > 0 || t->effect_param2_value > 0) {
                if (t->effect_param1_value > 0 && t->effect_param2_value > 0) {
                    synth->bitcrush_active = true;
                    synth->bitcrush_depth = (t->effect_param1_value * t->effect_param2_value);
                } else if(t->effect_param1_value > 0) {
                    synth->bitcrush_active = true;
                    synth->bitcrush_depth = t->effect_param1_value;
                } else if(t->effect_param2_value > 0) {
                    synth->bitcrush_active = true;
                    synth->bitcrush_depth = t->effect_param2_value;
                } else {
                    synth->bitcrush_active = false;
                }
            } else {
                synth->bitcrush_active = false;
            }
        }
    }
}

void cSynthVoiceApplyEffects(struct CSynthContext *synth, struct CVoice *v) {
    v->tone_with_fx = v->tone;
   
    if (v->arpeggio_active) {
        v->tone_with_fx = v->arpeggio_tone;
    }
    
    if(v->vibrato_active) {
        cSynthVoiceApplyVibrato(synth, v);
    }
    
    if(v->pitch_up_active) {
        cSynthVoiceApplyPitchUpDown(synth, v);
    }
    
    if(v->pitch_down_active) {
        cSynthVoiceApplyPitchUpDown(synth, v);
    }
    
    if(v->detune_active) {
        cSynthVoiceApplyDetune(synth, v);
    }
    
    if(v->arpeggio_active) {
        cSynthVoiceApplyArpeggio(synth, v);
    }
    
    if(v->portamento_active) {
        cSynthVoiceApplyPortamento(synth, v);
    }
    
    if (v->wavetable_active) {
        cSynthUpdateWavetable(synth, v);
    }
}

void cSynthAdvanceAmpTargets(struct CSynthContext *synth, struct CVoice *v) {
    
    double bpm = synth->tempo_map[synth->current_tempo_column][0]->bpm;
    double speed = bpm * synth->mod_advance_amp_targets;
    if(!v->amp_target_left_reached) {
        if(v->amp_target_left_higher) {
            v->amp_left += speed;
            if(v->amp_left >= v->amp_target_left) {
                v->amp_left = v->amp_target_left;
                v->amp_target_left_reached = true;
            }
        } else {
            v->amp_left -= speed;
            if(v->amp_left <= v->amp_target_left) {
                v->amp_left = v->amp_target_left;
                v->amp_target_left_reached = true;
            }
        }
    }
    
    if(!v->amp_target_right_reached) {
        if(v->amp_target_right_higher) {
            v->amp_right += speed;
            if(v->amp_right >= v->amp_target_right) {
                v->amp_right = v->amp_target_right;
                v->amp_target_right_reached = true;
            }
        } else {
            v->amp_right -= speed;
            if(v->amp_right <= v->amp_target_right) {
                v->amp_right = v->amp_target_right;
                v->amp_target_right_reached = true;
            }
        }
    }
}

void cSynthRampUpToFullAmp(struct CVoice *v) {
    
    v->amp_target_left = 1;
    v->amp_target_right = 1;
    
    if(v->amp_left > v->amp_target_left) {
        v->amp_target_left_higher = false;
        v->amp_target_left_reached = false;
    } else if(v->amp_left < v->amp_target_left) {
        v->amp_target_left_higher = true;
        v->amp_target_left_reached = false;
    } else {
        // same as target
    }
    
    if(v->amp_right > v->amp_target_right) {
        v->amp_target_right_higher = false;
        v->amp_target_right_reached = false;
    } else if(v->amp_right < v->amp_target_right) {
        v->amp_target_right_higher = true;
        v->amp_target_right_reached = false;
    } else {
        // same as target
    }
}

int16_t cSynthGetFMSample(struct CSynthContext *synth, struct CVoice *v, double delta_phi) {
   
    int fm_pos_int = 0;
    double fm_speed_mod = v->fm_speed;
    double d_waveformLength = v->waveform_length;
    double delta_phi2 = delta_phi;
    
    v->fm_pos += delta_phi2 * fm_speed_mod;
    if(v->fm_pos >= synth->wave_length) {
        v->fm_pos -= synth->wave_length;
    }
    
    if(v->fm_pos < 0) {
        v->fm_pos += d_waveformLength;
    } else if(v->fm_pos >= d_waveformLength) {
        v->fm_pos -= d_waveformLength;
    }
    
    fm_pos_int = (int)v->fm_pos;
    
    if(fm_pos_int >= synth->wave_length) {
        fm_pos_int -= synth->wave_length-1;
    }
    
    if(fm_pos_int < 0) {
        fm_pos_int = 0;
    }
    
    if(fm_pos_int >= synth->wave_length) {
        fm_pos_int = synth->wave_length-1;
    }
    
    const double int16_to_wavelength = 0.0156;
    double sine_phase = ((double)synth->sine_wave_table[fm_pos_int]+INT16_MAX) * int16_to_wavelength;
    sine_phase *= v->fm_depth;
    int phase_int = (int)(v->phase_double + sine_phase);
    
    if(phase_int < 0) {
        phase_int += synth->wave_length;
    } else if(phase_int >= synth->wave_length) {
        phase_int -= synth->wave_length;
    }
    
    if(phase_int < 0) {
        phase_int = 0;
    } else if(phase_int >= synth->wave_length) {
        phase_int = synth->wave_length-1;
    }

    int16_t sample = v->waveform[phase_int];
    return sample;
}

int16_t cSynthGetPWMSample(struct CSynthContext *synth, struct CVoice *v, int phase_int) {
    
    bool use_sine = false;
    int pwm_pos_int = 0;
    if(v->pwm_static_pos > 0 && v->pwm_speed < 1) {
        pwm_pos_int = (int)v->pwm_static_pos;
        v->pwm_depth = 0;
    } else if(v->pwm_speed > 0) {
        use_sine = true;
        v->pwm_pos += v->pwm_speed;
        pwm_pos_int = (int)v->pwm_pos;
        if(v->pwm_pos > synth->wave_length) {
            v->pwm_pos -= synth->wave_length;
            pwm_pos_int = (int)v->pwm_pos;
        }
    }
    
    const double int16_to_wavelength = 0.0156;
    double half_wavelength = synth->wave_length/2; // 512
    double sine_phase = ((double)synth->sine_wave_table[pwm_pos_int]+INT16_MAX) * int16_to_wavelength;
    // 512 is the middle, 0 - 1024 the edges. Scale the value closer to 512 for decreased depth.
    if(use_sine) {
        double diff = sine_phase - half_wavelength;
        double mod_diff = 0;
        if(diff < 0) {
            mod_diff = -diff;
        } else {
            mod_diff = diff;
        }
        mod_diff *= v->pwm_depth;
        if(diff < 0) {
            sine_phase = half_wavelength - mod_diff;
        } else {
            sine_phase = half_wavelength + mod_diff;
        }
    }
    
    if(sine_phase > phase_int) {
        return (int16_t)INT16_MAX;
    } else {
        return (int16_t)INT16_MIN;
    }
}

void cSynthVoiceApplyVibrato(struct CSynthContext *synth, struct CVoice *v) {
    
    if(v->vibrato_depth > 0) {
        double phase = v->vibrato_phase;
        double depth = v->vibrato_depth;
        double speed = v->vibrato_speed;
        double wave_length = (double)synth->wave_length;
        phase += (synth->mod_vibrato_speed * wave_length) + speed;
        
        if(phase > synth->wave_length) {
            phase = synth->wave_length - phase;
        }
        
        if(phase < 0) {
            phase = 0;
        }
        
        v->vibrato_phase = phase;
        
        double d_depth = synth->mod_vibrato_depth * depth;
        int phase_int = (int)phase;
        
        double mod = synth->sine_wave_table[phase_int] * d_depth;
        v->tone_with_fx = v->tone_with_fx + mod;
    }
}

void cSynthVoiceApplyPitchUpDown(struct CSynthContext *synth, struct CVoice *v) {
    
    double upper_limit = 119;
    double lower_limit = 3;
    if(v->pitch_speed1 != 0 || v->pitch_speed2 != 0) {
        double mod = v->pitch_speed1 * synth->mod_pitch_1 + v->pitch_speed2 * synth->mod_pitch_2;
        v->pitch_up_down_phase += mod;
        v->tone_with_fx = v->tone_with_fx + v->pitch_up_down_phase;
        if(v->tone_with_fx > upper_limit) {
            v->tone_with_fx = upper_limit;
        } else if(v->tone_with_fx < lower_limit) {
            v->tone_with_fx = lower_limit;
        }
    }
}

void cSynthVoiceApplyDetune(struct CSynthContext *synth, struct CVoice *v) {
    
    double upper_limit = 119;
    double lower_limit = 3;
    double detune_factor = synth->mod_detune;
    v->tone_with_fx = v->tone_with_fx + v->detune_amount * detune_factor;
    if(v->tone_with_fx > upper_limit) {
        v->tone_with_fx = upper_limit;
    } else if(v->tone_with_fx < lower_limit) {
        v->tone_with_fx = lower_limit;
    }
}

void cSynthVoiceApplyDownsampleSweep(struct CSynthContext *synth, struct CVoice *v, bool up) {

    double f_sample_rate = synth->sample_rate;
    double mod = 0.001;
    if(!synth->interleaved) {
        mod /= 2;
    }
    
    if(up) {
        if (v->downsample_sweep_slope > 0) {
            v->downsample_count += v->downsample_sweep_slope * (f_sample_rate * mod);
        } else if (v->downsample_sweep_speed > 0) {
            v->downsample_sweep_start += 1;
            v->downsample_count += v->downsample_sweep_start;
        }
        if(v->downsample_count > v->downsample_limit) {
            v->downsample_next_sample = true;
            v->downsample_count = 0;
        }
    } else {
        if (v->downsample_sweep_slope > 0) {
            v->downsample_count += v->downsample_sweep_slope * (f_sample_rate * mod);
        } else if (v->downsample_sweep_speed > 0) {
            if(v->downsample_sweep_start > f_sample_rate * mod) {
                v->downsample_sweep_start -= 1;
            }
            v->downsample_count += v->downsample_sweep_start;
        }
        if(v->downsample_count > v->downsample_limit) {
            v->downsample_next_sample = true;
            v->downsample_count = 0;
        }
   }
}

void cSynthVoiceApplyArpeggio(struct CSynthContext *synth, struct CVoice *v) {
    int arpeggio_speed = v->arp_speed;
    if(arpeggio_speed < 0) {
        // use global speed instead.
        arpeggio_speed = synth->arpeggio_speed;
    }
    v->arpeggio_count += synth->mod_arp_speed * arpeggio_speed;
    
    if(v->arpeggio_count > v->arpeggio_limit) {
        if(v->arpeggio_tone_toggle == 0) {
            v->arpeggio_tone = v->arpeggio_tone_1;
            v->arpeggio_count = 0;
            v->arpeggio_tone_toggle++;
        } else if(v->arpeggio_tone_toggle == 1) {
            v->arpeggio_tone = v->arpeggio_tone_2;
            v->arpeggio_count = 0;
            v->arpeggio_tone_toggle++;
        } else if(v->arpeggio_tone_toggle == 2) {
            v->arpeggio_tone = v->arpeggio_tone_0;
            v->arpeggio_count = 0;
            v->arpeggio_tone_toggle = 0;
        }
    }
}

void cSynthVoiceApplyPortamento(struct CSynthContext *synth, struct CVoice *v) {
   
    double segment = synth->mod_portamento_speed * (v->portamento_speed * v->portamento_speed);
    if(v->portamento_original_diff > 0) {
        v->portamento_diff -= segment;
        if(v->portamento_diff < 0) {
            v->portamento_diff = 0;
        }
    } else {
        v->portamento_diff += segment;
        if(v->portamento_diff > 0) {
            v->portamento_diff = 0;
        }
    }
  
    v->tone_with_fx += v->portamento_diff;
}


static void cSynthUpdateWavetable(struct CSynthContext *synth, struct CVoice *v) {
    /*
     Each voice has a wavetable_count which is the cursor for a lane in the wavetable_map.
     This effect will change the waveform of the channel.
     When the effect is reset, the channel should be reset to its original waveform.
     Also for each noteon, the wavetable_count should be set to 0.
     */

    double speed_mod = 0.001;
    int wavetable_lane = (int)v->wavetable_lane;
    int wavetable_cursor = (int)v->wavetable_cursor;
    int new_wavetable_cursor = 0;
    bool loop = false;
    
    if (wavetable_lane > -1 && wavetable_lane < synth->wavetable_width-1) {
        struct CWavetableNode *w_loop_node = synth->wavetable_map[wavetable_lane][1];
        if(w_loop_node->active > 0) {
            loop = true;
        }
        
        bool cursor_set = false;
        struct CWavetableNode *w_base_node = synth->wavetable_map[wavetable_lane][0];
        struct CWavetableNode *w_right_node = synth->wavetable_map[wavetable_lane+1][wavetable_cursor];
        if(w_right_node != NULL) {
            int n_speed = w_right_node->value;
            v->wavetable_cursor += (w_base_node->speed * speed_mod) * n_speed;
        }
        
        new_wavetable_cursor = (int)v->wavetable_cursor;
        if(new_wavetable_cursor > wavetable_cursor && new_wavetable_cursor < synth->wavetable_height) {
            for (int i = new_wavetable_cursor; i < synth->wavetable_height; i++) {
                struct CWavetableNode *w_new_node = synth->wavetable_map[wavetable_lane][i];
                if (w_new_node->active) {
                    new_wavetable_cursor = i;
                    cursor_set = true;
                    break;
                }
            }
            
            if(!cursor_set && loop) {
                new_wavetable_cursor = 2;
                v->wavetable_cursor = 2;
                cursor_set = true;
            } else if(!cursor_set) {
                new_wavetable_cursor = wavetable_cursor;
                v->wavetable_cursor = wavetable_cursor;
                cursor_set = true;
            }
        }
        
        if(cursor_set) {
            wavetable_cursor = new_wavetable_cursor;
        }
    }
    
    if (wavetable_cursor >= synth->wavetable_height) {
        if(loop) {
            wavetable_cursor = 2;
            v->wavetable_cursor = 2;
        } else {
            wavetable_cursor = synth->wavetable_height - 1;
        }
    }
    
    if (wavetable_lane > -1 && wavetable_lane < synth->wavetable_width && wavetable_cursor >= 0 && wavetable_cursor < synth->wavetable_height) {
        
        struct CWavetableNode *w_node = synth->wavetable_map[wavetable_lane][wavetable_cursor];

        char wave_type = w_node->value;
        if(wave_type == 0) {
            v->waveform = synth->sine_wave_table;
        } else if(wave_type == 1) {
            v->waveform = synth->sawtooth_wave_table;
        } else if(wave_type == 2) {
            v->waveform = synth->square_wave_table;
        } else if(wave_type == 3) {
            v->waveform = synth->triangle_wave_table;
        } else if(wave_type == 4) {
            v->waveform = synth->noise_table;
        } else if(wave_type == 5) {
            v->waveform = synth->custom_table;
        }
    }
}


void cSynthResetOnLoopBack(struct CSynthContext *synth) {
    
    // reset stuff when song loops.
    
    for (int i = 0; i < synth->track_width; i++) {
        struct CVoice *v = synth->voices[i];
        
        // tones
        v->last_valid_tone = -1;
        v->previous_tone = -1;
        
        // arpeggio
        v->arpeggio_active = false;
        
        // reset amp
        v->amp_target_left = 1;
        v->amp_target_right = 1;
        v->amp_target_left_higher = false;
        v->amp_target_right_higher = false;
        v->amp_target_left_reached = true;
        v->amp_target_right_reached = true;
        v->last_amp_effect1 = 0;
        v->last_amp_effect2 = 0;
    }
}

void cSynthResetPortamento(struct CVoice *v) {
    
    v->portamento_active = false;
    v->portamento_diff = 0;
    v->portamento_original_diff = 0;
    v->portamento_speed = 0;
    v->last_portamento_effect1 = -1;
    v->last_portamento_effect2 = -1;
}

void cSynthResetAllEffects(struct CVoice *v) {
    
    cSynthResetPortamento(v);
    
    v->vibrato_active = false;
    v->vibrato_phase = 0;
    v->pitch_up_active = false;
    v->pitch_down_active = false;
    
    // downsample
    v->downsample_sweep_up = false;
    v->downsample_sweep_down = false;
    v->downsample_last_sample = 0;
    v->downsample_limit = 100;
    v->downsample_count = 0;
    v->downsample_next_sample = true;
    v->downsample_sweep_start = 0;
    v->downsample_sweep_slope = 0;
    v->downsample_sweep_speed = 0;
    v->last_downsample_effect1 = -1;
    v->last_downsample_effect2 = -1;
    
    v->pitch_up_down_phase = 0;
    v->pitch_speed1 = 0;
    v->pitch_speed2 = 0;
    v->arpeggio_active = false;
    
    v->portamento_active = false;
    v->last_valid_tone = -1;
    v->previous_tone = -1;
    v->previous_effect = -1;
    v->sustain_active = false;
    v->pwm_active = false;
    v->pwm_static_pos = 0;
    v->pwm_speed = 0;
    
    v->fm_active = false;
    v->fm_speed = 0;
    v->fm_depth = 0;
    
    v->arp_speed = -1;
    v->last_arp_speed_effect1 = -1;
    v->last_arp_speed_effect2 = -1;
    
    v->dist_active = false;
    v->dist_amp_pre_clamp = 0;
    v->dist_amp_post_clamp = 0;
    v->last_dist_effect1 = -1;
    v->last_dist_effect2 = -1;
    
    v->wavetable_active = false;
    v->wavetable_lane = 0;
    v->wavetable_cursor = 2;
    if(v->wavetable_previous_table != NULL) {
        v->waveform = v->wavetable_previous_table;
    }
    v->wavetable_previous_table = NULL;
    v->delay_active = false;
    
    // detune
    v->detune_active = false;
    v->detune_amount = 0;
    
    // ramp upp to full vol
    cSynthRampUpToFullAmp(v);
    
}

void cSynthResetTempoIndex(struct CSynthContext *synth) {
    
    for(int i = 1; i < synth->tempo_height; i++) {
        struct CTempoNode *node = synth->tempo_map[synth->current_tempo_column][i];
        // go to the first active tempo-node in the current tempo column.
        if(node->active) {
            synth->tempo_index = i;
            break;
        }
    }
}

void cSynthUpdateHighlightInterval(struct CSynthContext *synth) {
    
    int count = 0;
    for(int i = 1; i < synth->tempo_height; i++) {
        struct CTempoNode *node = synth->tempo_map[synth->current_tempo_column][i];
        if(node->active) {
            count++;
        }
    }
    synth->track_highlight_interval = count;
}

void cSynthResetTrackProgress(struct CSynthContext *synth, int start_pattern, int start_row) {
    
    synth->track_progress = start_row;
    synth->track_progress_int = (int)synth->track_progress;
    synth->track_advance = true;
    synth->current_track = start_pattern;
    
    for (int i = 0; i < synth->track_width; i++) {
        struct CVoice *v = synth->voices[i];
        cSynthResetAllEffects(v);
    }
}

struct CVoice *cSynthNewVoice(struct CSynthContext *synth, int16_t *waveform) {
    
    struct CVoice *v = cAllocatorAlloc(sizeof(struct CVoice), "CVoice");
    v->delay_buffer = NULL;
    v->wavetable_previous_table = NULL;
    v->preview_amp_current = 0;
    v->int_id = 0;
    cSynthResetVoice(synth, waveform, v);
    return v;
}

static void cSynthResetVoice(struct CSynthContext *synth, int16_t *waveform, struct CVoice *v) {
    
    v->muted = 0;
    v->volume = 0.2;
    v->waveform = waveform;
    v->waveform_length = synth->wave_length;
    v->phase_int = 0;
    v->phase_double = 0;
    v->phase_double_inc = 0;
    v->active = true;
    v->tone = -1;
    v->last_valid_tone = -1;
    v->previous_tone = -1;
    v->previous_effect = -1;
    v->previous_effect_param1 = -1;
    v->previous_effect_param2 = -1;
    v->instrument = NULL;
    v->previous_instrument_number = -1;
    v->adsr_cursor = 0;
    v->tone_with_fx = 0;
    v->last_preview_tone = -1;
    v->sustain_active = false;

    // 4
    v->vibrato_active = false;
    v->vibrato_depth = 0;
    v->vibrato_speed = 0;
    v->vibrato_phase = 0;
    v->last_vibrato_effect1 = -1;
    v->last_vibrato_effect2 = -1;
    
    // E
    v->pitch_up_active = false;
    v->pitch_down_active = false;
    v->pitch_speed1 = 0;
    v->pitch_speed2 = 0;
    v->pitch_up_down_phase = 0;
    v->last_pitch_effect1 = -1;
    v->last_pitch_effect2 = -1;
    
    // downsample
    v->downsample_last_sample = 0;
    v->downsample_limit = 100;
    v->downsample_count = 0;
    v->downsample_next_sample = true;
    
    // B
    v->downsample_sweep_up = false;
    
    // C
    v->downsample_sweep_down = false;
    v->downsample_sweep_start = 0;
    v->downsample_sweep_slope = 0;
    v->downsample_sweep_speed = 0;
    
    v->last_downsample_effect1 = -1;
    v->last_downsample_effect2 = -1;
    
    v->wavetable_active = false;
    v->wavetable_lane = 0;
    v->wavetable_cursor = 2;
    if(v->wavetable_previous_table != NULL) {
        v->waveform = v->wavetable_previous_table;
    }
    v->wavetable_previous_table = NULL;
    
    // avoid clicks/pops when cutting off note.
    v->noteoff_slope = 0;
    v->noteoff_slope_value = 0;
    
    v->preview_slope = false;
    v->preview_amp_target = 0;
    
    // 0
    v->arpeggio_active = false;
    v->arpeggio_tone = -1;
    v->arpeggio_count = 0;
    v->arpeggio_limit = 0.01;
    v->arpeggio_tone_0 = 0;
    v->arpeggio_tone_1 = 0;
    v->arpeggio_tone_2 = 0;
    v->arpeggio_tone_toggle = 0;
    v->last_arpeggio_effect1 = 0;
    v->last_arpeggio_effect2 = 0;
    
    // 3
    cSynthResetPortamento(v);
    
    // A
    v->amp_left = 1;
    v->amp_right = 1;
    v->amp_target_left = 0;
    v->amp_target_right = 0;
    v->amp_target_left_higher = false;
    v->amp_target_right_higher = false;
    v->amp_target_left_reached = true;
    v->amp_target_right_reached = true;
    v->last_amp_effect1 = 0;
    v->last_amp_effect2 = 0;
    
    // 1 arp speed
    v->arp_speed = -1;
    v->last_arp_speed_effect1 = -1;
    v->last_arp_speed_effect2 = -1;
    
    // 8 PWM
    v->pwm_active = false;
    v->pwm_pos = 0;
    v->pwm_speed = 0;
    v->pwm_depth = 0;
    v->pwm_static_pos = 0;
    v->last_pwm_effect1 = 0;
    v->last_pwm_effect2 = 0;
    
    // FM
    v->fm_active = false;
    v->fm_pos = 0;
    v->fm_speed = 0;
    v->fm_depth = 0;
    v->last_fm_effect1 = 0;
    v->last_fm_effect2 = 0;
    
    // 5 dist
    v->dist_active = false;
    v->dist_amp_pre_clamp = 0;
    v->dist_amp_post_clamp = 0;
    v->last_dist_effect1 = -1;
    v->last_dist_effect2 = -1;
    
    // delay
    v->delay_active = false;
    v->param_delay_length = 0;
    v->param_delay_decay = 0;
    v->delay_buffer_size = synth->sample_rate*8;
    v->delay_buffer_limit = synth->sample_rate*2;
    v->delay_buffer_write_cursor = 0;
    v->delay_buffer_read_cursor = 0;
    
    if(v->delay_buffer == NULL) {
        v->delay_buffer = cAllocatorAlloc(sizeof(int16_t*)*v->delay_buffer_size, "delay buffer 1");
    }
    for(int i = 0; i < v->delay_buffer_size; i++) {
        v->delay_buffer[i] = 0;
    }
    
    // 7 detune
    v->detune_active = false;
    v->detune_amount = 0;
}

void cSynthInstrumentResetEnvelope(struct CInstrument *i) {
    
    i->adsr[0]->amp = 0.0;
    i->adsr[0]->pos = 0.0;
    i->adsr[1]->amp = 0.8;
    i->adsr[1]->pos = 0.001;
    i->adsr[2]->amp = 0.0;
    i->adsr[2]->pos = 0.15;
    i->adsr[3]->amp = 0.0;
    i->adsr[3]->pos = 0.6;
    i->adsr[4]->amp = 0.0;
    i->adsr[4]->pos = 0.9;
    i->adsr_cursor = 0;
}

struct CInstrument *cSynthNewInstrument(struct CSynthContext *synth) {
    
    struct CInstrument *i = cAllocatorAlloc(sizeof(struct CInstrument), "CInstrument");
    i->volume_scalar = 0.2;
    i->adsr_nodes = synth->max_adsr_nodes;
    i->adsr = cAllocatorAlloc(sizeof(struct CadsrNode*) * i->adsr_nodes, "Synth adsr nodes");
    for(int c = 0; c < i->adsr_nodes; c++) {
        i->adsr[c] = cAllocatorAlloc(sizeof(struct CadsrNode), "CadsrNode");
        i->adsr[c]->amp = 0;
        i->adsr[c]->pos = 0;
    }
    i->adsr[0]->amp = 0.0;
    i->adsr[0]->pos = 0.0;
    i->adsr[1]->amp = 0.8;
    i->adsr[1]->pos = 0.001;
    i->adsr[2]->amp = 0.0;
    i->adsr[2]->pos = 0.15;
    i->adsr[3]->amp = 0.0;
    i->adsr[3]->pos = 0.6;
    i->adsr[4]->amp = 0.0;
    i->adsr[4]->pos = 0.9;
    i->adsr_cursor = 0;
    i->instrument_number = -1;
    return i;
}

void cSynthWriteCustomTableFromNodes(struct CSynthContext *synth) {
    
    int i = 0;
    struct CInstrument *ins = synth->custom_instrument;
    for (i = 0; i < synth->wave_length; i++) {
        double pos = i / (double)synth->wave_length;
        double amp = 0;
        if(pos < ins->adsr[1]->pos) {
            amp = cSynthInstrumentVolumeByBaseNode(ins, 0, pos);
        } else if(pos < ins->adsr[2]->pos) {
            amp = cSynthInstrumentVolumeByBaseNode(ins, 1, pos);
        } else if(pos < ins->adsr[3]->pos) {
            amp = cSynthInstrumentVolumeByBaseNode(ins, 2, pos);
        } else if(pos < ins->adsr[4]->pos) {
            amp = cSynthInstrumentVolumeByBaseNode(ins, 3, pos);
        } else {
            amp = ins->adsr[4]->amp;
        }
        
        int sample = (int)(amp * INT16_MAX);
        if(sample > INT16_MAX) {
            sample = INT16_MAX;
        } else if(sample < INT16_MIN) {
            sample = INT16_MIN;
        }
        synth->custom_table[i] = (int16_t)sample;
    }
}

static double cSynthInstrumentVolumeByBaseNode(struct CInstrument *i, int base_node, double cursor) {
    
    double relative_cursor_pos = (cursor - i->adsr[base_node]->pos) / (i->adsr[base_node+1]->pos - i->adsr[base_node]->pos);
    double amp_diff = (i->adsr[base_node+1]->amp - i->adsr[base_node]->amp);
    double amp = i->adsr[base_node]->amp+(relative_cursor_pos*amp_diff);
    return amp;
}

void cSynthIncAdsrCursor(struct CVoice *v, struct CInstrument *i, struct CSynthContext *synth) {
    
    if(v->sustain_active) {
        // do not increase cursor if sustain is active.
        if(v->adsr_cursor > i->adsr[3]->pos) {
            return;
        }
    }
    v->adsr_cursor += synth->mod_adsr_cursor;
}

double cSynthInstrumentVolumeWithPreviewSlope(double amp, struct CVoice *v) {
    
    double mod = 0.001;
    v->preview_amp_target = amp;
    if(v->preview_amp_target > v->preview_amp_current) {
        v->preview_amp_current += mod;
        if(v->preview_amp_current >= v->preview_amp_target) {
            v->preview_amp_current = v->preview_amp_target;
        }
    } else if(v->preview_amp_target < v->preview_amp_current) {
        v->preview_amp_current -= mod;
        if(v->preview_amp_current <= v->preview_amp_target) {
            v->preview_amp_current = v->preview_amp_target;
        }
    }
    amp = v->preview_amp_current;
    
    return amp;
}

double cSynthInstrumentVolumeByPos(struct CInstrument* i, struct CVoice *v) {
    
    double pos = v->adsr_cursor;
    double amp = 0;
    if(pos < i->adsr[1]->pos) {
        amp = cSynthInstrumentVolumeByBaseNode(i, 0, pos);
    } else if(pos < i->adsr[2]->pos) { //todo: crash here in iOS 1.0.7. investigate further.
        amp = cSynthInstrumentVolumeByBaseNode(i, 1, pos);
    } else if(pos < i->adsr[3]->pos) {
        amp = cSynthInstrumentVolumeByBaseNode(i, 2, pos);
    } else if(pos < i->adsr[4]->pos) {
        amp = cSynthInstrumentVolumeByBaseNode(i, 3, pos);
    } else {
        // just use the last pos of adsr[4]
        amp = i->adsr[4]->amp;
    }
    
    if(v->preview_slope) {
        return cSynthInstrumentVolumeWithPreviewSlope(amp, v);
    }
    
    return amp;
}

struct CTrackNode *cSynthNewTrackNode() {
    
    struct CTrackNode *t = cAllocatorAlloc(sizeof(struct CTrackNode), "CTrackNode");
    t->instrument = NULL;
    t->tone = 0;
    t->tone_active = false;
    t->instrument_nr = 0;
    t->effect = '-';
    t->effect_param1 = '-';
    t->effect_param2 = '-';
    t->effect_value = -1;
    t->effect_param1_value = -1;
    t->effect_param2_value = -1;
    return t;
}

struct CTrackNode *cSynthCopyTracknode(struct CTrackNode *track_node) {
    struct CTrackNode *t = NULL;
    if(track_node != NULL) {
        t = cAllocatorAlloc(sizeof(struct CTrackNode), "CTrackNode copy");
        t->instrument = track_node->instrument;
        t->tone = track_node->tone;
        t->tone_active = track_node->tone_active;
        t->instrument_nr = track_node->instrument_nr;
        t->effect = track_node->effect;
        t->effect_param1 = track_node->effect_param1;
        t->effect_param2 = track_node->effect_param2;
        t->effect_value = track_node->effect_value;
        t->effect_param1_value = track_node->effect_param1_value;
        t->effect_param2_value = track_node->effect_param2_value;
    }
    return t;
}

void cSynthBuildSineWave(int16_t *data, int wave_length) {
    
    double pi = 3.14159265358979323846;
    double phaseIncrement = (2.0f * pi)/(double)wave_length;
    double currentPhase = 0.0;
    for(int i = 0; i < wave_length; i++) {
        int sample = (int)(sin(currentPhase) * INT16_MAX);
        data[i] = (int16_t)sample;
        currentPhase += phaseIncrement;
    }
}

void cSynthBuildSawtoothWave(int16_t *data, int wave_length) {
    
    double phaseIncrement = INT16_MAX * 2 / (double)wave_length;
    double currentPhase = 0.0;
    for(int i = 0; i < wave_length; i++) {
        int16_t sample = (int16_t)(INT16_MAX-(int)currentPhase);
        data[i] = sample;
        currentPhase += phaseIncrement;
    }
}

void cSynthBuildSquareWave(int16_t *data, int wave_length) {
    
    for(int i = 0; i < wave_length; i++) {
        int16_t sample = INT16_MAX;
        if(i > wave_length/2) {
            sample = INT16_MIN;
        }
        data[i] = sample;
    }
}

void cSynthBuildTriangleWave(int16_t *data, int wave_length) {
    
    double phaseIncrement = (INT16_MAX * 2 / (double)wave_length) * 2;
    double currentPhase = -INT16_MAX;
    for (int i = 0; i < wave_length; i++) {
        int16_t sample = (int16_t)currentPhase;
        if(currentPhase > INT16_MAX) {
            sample = INT16_MAX;
        }
        data[i] = sample;
        if(i < wave_length/2) {
            currentPhase += phaseIncrement;
        } else {
            currentPhase -= phaseIncrement;
        }
    }
}

void cSynthBuildNoise(int16_t *data, int wave_length) {
    
    for (int i = 0; i < wave_length; i++) {
        data[i] = static_noise_table[i];
    }
}

double cSynthGetFrequency(double pitch) {
    
    return pow(ChromaticRatio, pitch - 57) * 440;
}

void cSynthIncPhase(struct CVoice *v, double inc) {
    
    v->phase_double += inc;
    v->phase_int = (int)v->phase_double;
    if(v->phase_double >= v->waveform_length) {
        double diff = v->phase_double - v->waveform_length;
        v->phase_double = diff;
        v->phase_int = (int)diff;
    }
}

char *cSynthToneToChar(int tone) {
    
    char *return_str;
    switch (tone) {
            
        // octave 0
        case 12:
            return_str = "C-0";
            break;
        case 13:
            return_str = "C#0";
            break;
        case 14:
            return_str = "D-0";
            break;
        case 15:
            return_str = "D#0";
            break;
        case 16:
            return_str = "E-0";
            break;
        case 17:
            return_str = "F-0";
            break;
        case 18:
            return_str = "F#0";
            break;
        case 19:
            return_str = "G-0";
            break;
        case 20:
            return_str = "G#0";
            break;
        case 21:
            return_str = "A-0";
            break;
        case 22:
            return_str = "A#0";
            break;
        case 23:
            return_str = "B-0";
            break;
            
        // octave 1
        case 24:
            return_str = "C-1";
            break;
        case 25:
            return_str = "C#1";
            break;
        case 26:
            return_str = "D-1";
            break;
        case 27:
            return_str = "D#1";
            break;
        case 28:
            return_str = "E-1";
            break;
        case 29:
            return_str = "F-1";
            break;
        case 30:
            return_str = "F#1";
            break;
        case 31:
            return_str = "G-1";
            break;
        case 32:
            return_str = "G#1";
            break;
        case 33:
            return_str = "A-1";
            break;
        case 34:
            return_str = "A#1";
            break;
        case 35:
            return_str = "B-1";
            break;

        // octave 2
        case 36:
            return_str = "C-2";
            break;
        case 37:
            return_str = "C#2";
            break;
        case 38:
            return_str = "D-2";
            break;
        case 39:
            return_str = "D#2";
            break;
        case 40:
            return_str = "E-2";
            break;
        case 41:
            return_str = "F-2";
            break;
        case 42:
            return_str = "F#2";
            break;
        case 43:
            return_str = "G-2";
            break;
        case 44:
            return_str = "G#2";
            break;
        case 45:
            return_str = "A-2";
            break;
        case 46:
            return_str = "A#2";
            break;
        case 47:
            return_str = "B-2";
            break;
        
        // octave 3
        case 48:
            return_str = "C-3";
            break;
        case 49:
            return_str = "C#3";
            break;
        case 50:
            return_str = "D-3";
            break;
        case 51:
            return_str = "D#3";
            break;
        case 52:
            return_str = "E-3";
            break;
        case 53:
            return_str = "F-3";
            break;
        case 54:
            return_str = "F#3";
            break;
        case 55:
            return_str = "G-3";
            break;
        case 56:
            return_str = "G#3";
            break;
        case 57:
            return_str = "A-3";
            break;
        case 58:
            return_str = "A#3";
            break;
        case 59:
            return_str = "B-3";
            break;
            
        // octave 4
        case 60:
            return_str = "C-4";
            break;
        case 61:
            return_str = "C#4";
            break;
        case 62:
            return_str = "D-4";
            break;
        case 63:
            return_str = "D#4";
            break;
        case 64:
            return_str = "E-4";
            break;
        case 65:
            return_str = "F-4";
            break;
        case 66:
            return_str = "F#4";
            break;
        case 67:
            return_str = "G-4";
            break;
        case 68:
            return_str = "G#4";
            break;
        case 69:
            return_str = "A-4";
            break;
        case 70:
            return_str = "A#4";
            break;
        case 71:
            return_str = "B-4";
            break;
            
        // octave 5
        case 72:
            return_str = "C-5";
            break;
        case 73:
            return_str = "C#5";
            break;
        case 74:
            return_str = "D-5";
            break;
        case 75:
            return_str = "D#5";
            break;
        case 76:
            return_str = "E-5";
            break;
        case 77:
            return_str = "F-5";
            break;
        case 78:
            return_str = "F#5";
            break;
        case 79:
            return_str = "G-5";
            break;
        case 80:
            return_str = "G#5";
            break;
        case 81:
            return_str = "A-5";
            break;
        case 82:
            return_str = "A#5";
            break;
        case 83:
            return_str = "B-5";
            break;
            
            
            // octave 6
        case 84:
            return_str = "C-6";
            break;
        case 85:
            return_str = "C#6";
            break;
        case 86:
            return_str = "D-6";
            break;
        case 87:
            return_str = "D#6";
            break;
        case 88:
            return_str = "E-6";
            break;
        case 89:
            return_str = "F-6";
            break;
        case 90:
            return_str = "F#6";
            break;
        case 91:
            return_str = "G-6";
            break;
        case 92:
            return_str = "G#6";
            break;
        case 93:
            return_str = "A-6";
            break;
        case 94:
            return_str = "A#6";
            break;
        case 95:
            return_str = "B-6";
            break;
            
            
            // octave 7
        case 96:
            return_str = "C-7";
            break;
        case 97:
            return_str = "C#7";
            break;
        case 98:
            return_str = "D-7";
            break;
        case 99:
            return_str = "D#7";
            break;
        case 100:
            return_str = "E-7";
            break;
        case 101:
            return_str = "F-7";
            break;
        case 102:
            return_str = "F#7";
            break;
        case 103:
            return_str = "G-7";
            break;
        case 104:
            return_str = "G#7";
            break;
        case 105:
            return_str = "A-7";
            break;
        case 106:
            return_str = "A#7";
            break;
        case 107:
            return_str = "B-7";
            break;
            
            
            // octave 8
        case 108:
            return_str = "C-8";
            break;
        case 109:
            return_str = "C#8";
            break;
        case 110:
            return_str = "D-8";
            break;
        case 111:
            return_str = "D#8";
            break;
        case 112:
            return_str = "E-8";
            break;
        case 113:
            return_str = "F-8";
            break;
        case 114:
            return_str = "F#8";
            break;
        case 115:
            return_str = "G-8";
            break;
        case 116:
            return_str = "G#8";
            break;
        case 117:
            return_str = "A-8";
            break;
        case 118:
            return_str = "A#8";
            break;
        case 119:
            return_str = "B-8";
            break;
            
            // octave 9
        case 120:
            return_str = "C-9";
            break;
        case 121:
            return_str = "C#9";
            break;
        case 122:
            return_str = "D-9";
            break;
        case 123:
            return_str = "D#9";
            break;
        case 124:
            return_str = "E-9";
            break;
        case 125:
            return_str = "F-9";
            break;
        case 126:
            return_str = "F#9";
            break;
        case 127:
            return_str = "G-9";
            break;
        case 128:
            return_str = "G#9";
            break;
        case 129:
            return_str = "A-9";
            break;
        case 130:
            return_str = "A#9";
            break;
        case 131:
            return_str = "B-9";
            break;
        default:
            return_str = "err";
            break;
    }
    
    return return_str;
}

char cSynthGetCharFromParam(char value) {
    char ch = '-';
    
    switch (value) {
        case 0:
            ch = '0';
            break;
        case 1:
            ch = '1';
            break;
        case 2:
            ch = '2';
            break;
        case 3:
            ch = '3';
            break;
        case 4:
            ch = '4';
            break;
        case 5:
            ch = '5';
            break;
        case 6:
            ch = '6';
            break;
        case 7:
            ch = '7';
            break;
        case 8:
            ch = '8';
            break;
        case 9:
            ch = '9';
            break;
        case 10:
            ch = 'A';
            break;
        case 11:
            ch = 'B';
            break;
        case 12:
            ch = 'C';
            break;
        case 13:
            ch = 'D';
            break;
        case 14:
            ch = 'E';
            break;
        case 15:
            ch = 'F';
            break;
        case 16:
            ch = 'G';
            break;
        default:
            break;
    }
    return ch;
}

char cSynthGetValueForParam(char value) {
    char ch = '-';
    
    switch (value) {
        case '0':
            ch = 0;
            break;
        case '1':
            ch = 1;
            break;
        case '2':
            ch = 2;
            break;
        case '3':
            ch = 3;
            break;
        case '4':
            ch = 4;
            break;
        case '5':
            ch = 5;
            break;
        case '6':
            ch = 6;
            break;
        case '7':
            ch = 7;
            break;
        case '8':
            ch = 8;
            break;
        case '9':
            ch = 9;
            break;
        case 'A':
            ch = 10;
            break;
        case 'B':
            ch = 11;
            break;
        case 'C':
            ch = 12;
            break;
        case 'D':
            ch = 13;
            break;
        case 'E':
            ch = 14;
            break;
        case 'F':
            ch = 15;
            break;
        case 'G':
            ch = 16;
            break;
        default:
            break;
    }
    return ch;
}

void cSynthPasteNotesFromPattern(struct CSynthContext *synth, int origin_x, int origin_y, int target_x, int target_y) {
    
    if(origin_x > -1 && origin_x < synth->patterns_width
       && target_x > -1 && target_x < synth->patterns_width
       && origin_y > -1 && origin_y < synth->patterns_height
       && target_y > -1 && target_y < synth->patterns_height) {
        
        int origin_pattern = synth->patterns[origin_x][origin_y];
        int target_pattern = synth->patterns[target_x][target_y];
        
        for(int y = 0; y < synth->track_max_height; y++) {
            struct CTrackNode *track_node = synth->track[origin_pattern][origin_x][y];
            if(synth->track[target_pattern][target_x][y] != NULL) {
                synth->track[target_pattern][target_x][y] = cAllocatorFree(synth->track[target_pattern][target_x][y]);
            }
            synth->track[target_pattern][target_x][y] = cSynthCopyTracknode(track_node);
        }
    }
}

void cSynthTransposePattern(struct CSynthContext *synth, int track, int track_x, bool up, int amount) {

    int note_min = 12;
    int note_max = 119;
    int shift = amount;
    
    bool is_in_range = true;
    
    for(int y = 0; y < synth->track_height; y++) {
        if(y > -1 && y < synth->track_height) {
            int pattern = synth->patterns[track_x][track];
            struct CTrackNode *node = synth->track[pattern][track_x][y];
            if(node != NULL) {
                if(up) {
                    if(node->tone != -1 && node->tone_active && node->tone + shift > note_max) {
                        is_in_range = false;
                    }
                } else {
                    if(node->tone != -1 && node->tone_active && node->tone - shift < note_min) {
                        is_in_range = false;
                    }
                }
            }
        }
    }
    
    if(is_in_range) {
        for(int y = 0; y < synth->track_height; y++) {
            if(y > -1 && y < synth->track_height) {
                int pattern = synth->patterns[track_x][track];
                struct CTrackNode *node = synth->track[pattern][track_x][y];
                if(node != NULL) {
                    if(up) {
                        if(node->tone != -1 && node->tone_active && node->tone + shift <= note_max) {
                            node->tone += shift;
                        }
                    } else {
                        if(node->tone != -1 && node->tone_active && node->tone - shift >= note_min) {
                            node->tone -= shift;
                        }
                    }
                }
            }
        }
    }
}

void cSynthTransposeSelection(struct CSynthContext *synth, int track, int cursor_x, int cursor_y, int selection_x, int selection_y, bool up, int amount) {
    
    // change octave for notes
    int sel_x = (int)floor(selection_x/5);
    int cur_x = (int)floor(cursor_x/5);
    
    int s_x = 0;
    int s_y = 0;
    int s_w = 0;
    int s_h = 0;
    
    if(cur_x < sel_x) {
        s_x = cur_x;
        s_w = sel_x;
    } else {
        s_x = sel_x;
        s_w = cur_x;
    }
    
    if(cursor_y < selection_y) {
        s_y = cursor_y;
        s_h = selection_y;
    } else {
        s_y = selection_y;
        s_h = cursor_y;
    }
    
    s_w += 1;
    s_h += 1;
    
    int note_min = 12;
    int note_max = 119;
    int shift = amount;
    
    if(synth->debuglog) { printf("octave selection x:%d y:%d w:%d h:%d\n", s_x, s_y, s_w, s_h); }
    
    for(int x = 0; x < s_w-s_x; x++) {
        for(int y = 0; y < s_h-s_y; y++) {
            int pos_x = s_x + x;
            int pos_y = s_y + y;
            if(x > -1 && x < synth->track_width
               && y > -1 && y < synth->track_height
               && pos_x > -1 && pos_x < synth->track_width
               && pos_y > -1 && pos_y < synth->track_height) {
                int pattern = synth->patterns[pos_x][track];
                struct CTrackNode *node = synth->track[pattern][pos_x][pos_y];
                if(node != NULL) {
                    if(up) {
                        if(node->tone + shift <= note_max) {
                            node->tone += shift;
                        }
                    } else {
                        if(node->tone - shift >= note_min) {
                            node->tone -= shift;
                        }
                    }
                }
            }
        }
    }
}

void cSynthCopyNotesFromSelection(struct CSynthContext *synth, int track, int cursor_x, int cursor_y, int selection_x, int selection_y, bool cut, bool store) {
    
    // clear copytrack before making new copy.
    for(int x = 0; x < synth->track_width; x++) {
        for(int y = 0; y < synth->track_max_height; y++) {
            if(synth->copy_track[x][y] != NULL) {
                synth->copy_track[x][y] = cAllocatorFree(synth->copy_track[x][y]);
            }
        }
    }
    
    // copy notes from to copy track
    int sel_x = (int)floor(selection_x/5);
    int cur_x = (int)floor(cursor_x/5);
    
    int s_x = 0;
    int s_y = 0;
    int s_w = 0;
    int s_h = 0;
    
    if(cur_x < sel_x) {
        s_x = cur_x;
        s_w = sel_x;
    } else {
        s_x = sel_x;
        s_w = cur_x;
    }
    
    if(cursor_y < selection_y) {
        s_y = cursor_y;
        s_h = selection_y;
    } else {
        s_y = selection_y;
        s_h = cursor_y;
    }
    
    s_w += 1;
    s_h += 1;
    
    if(synth->debuglog) { printf("copy selection x:%d y:%d w:%d h:%d\n", s_x, s_y, s_w, s_h); }
    
    for(int x = 0; x < s_w-s_x; x++) {
        for(int y = 0; y < s_h-s_y; y++) {
            int pos_x = s_x + x;
            int pos_y = s_y + y;
            if(x > -1 && x < synth->track_width
               && y > -1 && y < synth->track_height
               && pos_x > -1 && pos_x < synth->track_width
               && pos_y > -1 && pos_y < synth->track_height) {
                
                int pattern = synth->patterns[pos_x][track];
                if(store) {
                    struct CTrackNode *track_node = synth->track[pattern][pos_x][pos_y];
                    synth->copy_track[x][y] = cSynthCopyTracknode(track_node);
                }
                if(cut) {
                    synth->track[pattern][pos_x][pos_y] = cAllocatorFree(synth->track[pattern][pos_x][pos_y]);
                }
            }
        }
    }
}

void cSynthPasteNotesToPos(struct CSynthContext *synth, int track, int cursor_x, int cursor_y) {
    
    int node_x = (int)floor(cursor_x/5);
    
    for(int x = 0; x < synth->track_width; x++) {
        for(int y = 0; y < synth->track_height; y++) {
            struct CTrackNode *track_node = synth->copy_track[x][y];
            int pos_x = node_x + x;
            int pos_y = cursor_y + y;
            if(pos_x > -1 && pos_x < synth->track_width
               && pos_y > -1 && pos_y < synth->track_height) {
            
                int pattern = synth->patterns[pos_x][track];
                if(track_node != NULL && synth->track[pattern][pos_x][pos_y] != NULL) {
                    synth->track[pattern][pos_x][pos_y] = cAllocatorFree(synth->track[pattern][pos_x][pos_y]);
                }
                
                if(track_node != NULL) {
                    synth->track[pattern][pos_x][pos_y] = cSynthCopyTracknode(track_node);
                }
            }
        }
    }
}

double cSynthGetRelativeModifierReverse(double base, int sample_rate, bool interleaved, bool log) {
    // enter the value you would like to receive as a modifier and get the value you need
    // to supply to cSynthGetRelativeModifier.
    
    if(!interleaved) {
        base *= 2;
    }
    double value = base * sample_rate;
    if(log) {
        printf("got base:%f from relative sample mod:%f\n", value, base);
    }
    return value;
}

double cSynthGetRelativeModifier(double base, int sample_rate, bool interleaved, bool log) {
    // use this for modifiers that are applied per sample or sample pair(interleaved).
    
    double value = base / sample_rate;
    if (!interleaved) {
        value /= 2;
    }
    if(log) {
        printf("got relative sample modifier:%f using base:%f\n", value, base);
    }
    return value;
}


double cSynthGetRelativeModForChunksReverse(double base, int sample_rate, int chunk_size, bool interleaved, bool log) {
    // use this for modifiers that are applied per chunk.
    double value = chunk_size / (double)sample_rate;
    double ret = base / value;
    if(!interleaved) {
        ret /= 2;
    }
    if(log) {
        printf("got base:%f relative chunk mod:%f\n", base, ret);
    }
    return ret;
}

double cSynthGetRelativeModForChunks(double base, int sample_rate, int chunk_size, bool interleaved, bool log) {
    // use this for modifiers that are applied per chunk.
    double value = chunk_size / (double)sample_rate;
    value *= base;
    if(!interleaved) {
        value /= 2;
    }
    if(log) {
        printf("got relative chunk mod:%f with base:%f\n", value, base);
    }
    return value;
}



void cSynthRenderAudio(struct CSynthContext *synth, int16_t *s_byteStream, long begin, long end, long length, bool playing, bool exporting) {
    
    if(synth == NULL) {
        fprintf(stderr, "audioCallback: synthContext is null, returning.\n");
        return;
    }
    
    int i = 0;
    
    for(i = 0; i < synth->max_voices; i++) {
        for(int s_x = 0; s_x < synth->temp_mixdown_size; s_x++) {
            synth->temp_mixdown_buffer[i][s_x] = 0;
        }
    }
    
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
            
            for (i = 0; i < length; i+=2) {
                if(voice->note_on) {
                    
                    double amp = 0;
                    double amp_left = 0;
                    double amp_right = 0;
                    
                    cSynthAdvanceAmpTargets(synth, voice);
                    
                    if(voice->waveform == synth->noise_table) {
                        if(voice->tone_with_fx > 0) {
                            voice->phase_double += voice->tone_with_fx;
                            voice->phase_int = (int)voice->phase_double;
                            if(voice->phase_double >= synth->noise_length) {
                                double diff = voice->phase_double - synth->noise_length;
                                voice->phase_double = diff;
                                voice->phase_int = (int)diff;
                            }
                        }
                    } else {
                        cSynthIncPhase(voice, delta_phi);
                    }
                    

                    if(voice->noteoff_slope) {
                        double init_amp = cSynthInstrumentVolumeByPos(ins, voice)*ins->volume_scalar;
                        amp = voice->noteoff_slope_value*init_amp;
                        double bpm = synth->tempo_map[synth->current_tempo_column][0]->bpm;
                        voice->noteoff_slope_value -= bpm * synth->mod_noteoff_slope;
                        if(voice->noteoff_slope_value < 0) {
                            voice->noteoff_slope_value = 0;
                        }
                    } else {
                        amp = cSynthInstrumentVolumeByPos(ins, voice)*ins->volume_scalar;
                    }
                        
                    amp_left = amp * voice->amp_left;
                    amp_right = amp * voice->amp_right;
                    cSynthIncAdsrCursor(voice, ins, synth);
                
                    if(voice->downsample_sweep_up || voice->downsample_sweep_down) {
                        
                        if(voice->downsample_sweep_up) {
                            cSynthVoiceApplyDownsampleSweep(synth, voice, true);
                        }
                        
                        if(voice->downsample_sweep_down) {
                            cSynthVoiceApplyDownsampleSweep(synth, voice, false);
                        }
                        
                        if(voice->downsample_next_sample) {
                            if(voice->waveform == synth->noise_table) {
                                if(voice->phase_int < synth->noise_length
                                   && voice->phase_int > -1) {
                                    voice->downsample_last_sample = (int16_t)(voice->waveform[voice->phase_int]);
                                }
                            } else {
                                if(voice->phase_int < synth->wave_length
                                   && voice->phase_int > -1) {
                                    voice->downsample_last_sample = (int16_t)(voice->waveform[voice->phase_int]);
                                }
                            }
                            
                            voice->downsample_next_sample = false;
                        }
                        
                        if(s_byteStream != NULL) {
                            int sample_left = (int)(voice->downsample_last_sample * synth->master_amp);
                            int sample_right = (int)(voice->downsample_last_sample * synth->master_amp);
                            cSynthAddSamples(synth, v_i, voice, sample_left, sample_right, s_byteStream, i, i+1, amp_left, amp_right);
                        }
                    } else {
                        if(voice->waveform == synth->noise_table) {
                            if(voice->phase_int < synth->noise_length) {
                                int16_t sample = voice->waveform[voice->phase_int];
                                if(s_byteStream != NULL) {
                                    int sample_left = (int)(sample * synth->master_amp);
                                    int sample_right = (int)(sample * synth->master_amp);
                                    cSynthAddSamples(synth, v_i, voice, sample_left, sample_right, s_byteStream, i, i+1, amp_left, amp_right);
                                }
                            }
                        } else if(voice->phase_int < synth->wave_length) {
                            int16_t sample = 0;
                            if(voice->waveform == synth->square_wave_table && voice->pwm_active) {
                                sample = cSynthGetPWMSample(synth, voice, voice->phase_int);
                            } else if(voice->fm_active) {
                                sample = cSynthGetFMSample(synth, voice, delta_phi);
                            } else {
                                sample = voice->waveform[voice->phase_int];
                            }
                            
                            if(s_byteStream != NULL) {
                                int sample_left = (int)(sample * synth->master_amp);
                                int sample_right = (int)(sample * synth->master_amp);
                                cSynthAddSamples(synth, v_i, voice, sample_left, sample_right, s_byteStream, i, i+1, amp_left, amp_right);
                            }
                        }
                    }
                }
            }
        }
    }
    
    if(s_byteStream != NULL) {
        int16_t sample_left = 0;
        int16_t sample_right = 0;
        for(int v_i = 0; v_i < synth->max_voices; v_i++) {
            if(cSynthChannelShouldBeRendered(synth, v_i)) {
                // regular mixdown
                for(i = 0; i < length; i+=2) {
                    sample_left = (int16_t)synth->temp_mixdown_buffer[v_i][i];
                    sample_right = (int16_t)synth->temp_mixdown_buffer[v_i][i+1];
                    s_byteStream[i+begin] += sample_left;
                    s_byteStream[i+begin+1] += sample_right;
                    if(sample_left > INT16_MAX || sample_left < INT16_MIN || sample_right > INT16_MAX || sample_right < INT16_MIN) {
                        synth->audio_clips = true;
                    }
                }
            }
        }
    }
    
    if(s_byteStream != NULL) {
        for(int v_i = 0; v_i < synth->max_voices; v_i++) {
            struct CVoice *voice = synth->voices[v_i];
            if(voice->delay_active) {
                int delay_samples = (int)voice->param_delay_length;
                int delay_buffer_limit = delay_samples*2;
                double decay = voice->param_delay_decay;
                
                // write delay to buffer
                for(i = 0; i < length; i+=2) {
                    int delay_pos = delay_samples + voice->delay_buffer_write_cursor;
                    if(delay_pos >= delay_buffer_limit) {
                        voice->delay_buffer_write_cursor = 0;
                        delay_pos = delay_samples + voice->delay_buffer_write_cursor;
                    }
                    voice->delay_buffer[delay_pos] += synth->temp_mixdown_buffer[v_i][i];
                    voice->delay_buffer[delay_pos+1] += synth->temp_mixdown_buffer[v_i][i+1];
                    voice->delay_buffer[delay_pos] *= decay;
                    voice->delay_buffer[delay_pos+1] *= decay;
                    voice->delay_buffer_write_cursor += 2;
                }
                
                // write delay to master
                for(i = 0; i < length; i+=2) {
                    int pos = voice->delay_buffer_read_cursor;
                    if(pos >= delay_buffer_limit) {
                        voice->delay_buffer_read_cursor = delay_samples;
                        pos = voice->delay_buffer_read_cursor;
                    }
                    voice->delay_buffer[pos] *= decay;
                    voice->delay_buffer[pos+1] *= decay;
                    s_byteStream[i+begin] += voice->delay_buffer[pos];
                    s_byteStream[i+begin+1] += voice->delay_buffer[pos+1];
                    voice->delay_buffer_read_cursor += 2;
                }
            }
            
            // write channel to master
            for(i = 0; i < length; i+=2) {
                s_byteStream[i+begin] += synth->temp_mixdown_buffer[v_i][i];
                s_byteStream[i+begin+1] += synth->temp_mixdown_buffer[v_i][i+1];
            }
            
            if(synth->bitcrush_active) {
                cSynthBitcrush(synth, s_byteStream, begin, length);
            }
        }
    }

    if(playing || exporting) {
        cSynthAdvanceTrack(synth, length);
    }
}

static void cSynthBitcrush(struct CSynthContext *synth, int16_t *s_byteStream, long begin, long length) {
    
    for(long i = 0; i < length; i+=2) {
        double bit_depth = synth->bitcrush_depth+15;
        const double default_bit_depth = 32767;
        int16_t sample_left = s_byteStream[i+begin];
        int16_t sample_right = s_byteStream[i+begin+1];
        int16_t positive_sample_left = sample_left;
        int16_t positive_sample_right = sample_right;
        
        if (positive_sample_left < 0) {
            positive_sample_left = -positive_sample_left;
        }
        if (positive_sample_right < 0) {
            positive_sample_right = -positive_sample_right;
        }
        int16_t new_sample_left = 0;
        int16_t new_sample_right = 0;
        double left_diff_base = positive_sample_left / default_bit_depth;
        double right_diff_base = positive_sample_right / default_bit_depth;
        double scale = default_bit_depth / bit_depth;

        double left_diff = left_diff_base * bit_depth;
        int16_t left_diff_int = (int16_t)left_diff;
        if(left_diff_int <= 0) {
            left_diff_int = 1;
        }
        new_sample_left = (int16_t)(left_diff_int * scale);

        double right_diff = right_diff_base * bit_depth;
        int16_t right_diff_int = (int16_t)right_diff;
        
        if(right_diff_int <= 0) {
            right_diff_int = 1;
        }
        
        new_sample_right = (int16_t)(right_diff_int * scale);
        
        if(sample_left < 0) {
            new_sample_left = -new_sample_left;
        }
        if(sample_right < 0) {
            new_sample_right = -new_sample_right;
        }
        
        s_byteStream[i+begin] = new_sample_left;
        s_byteStream[i+begin+1] = new_sample_right;
    }
}

bool cSynthChannelShouldBeRendered(struct CSynthContext *synth, int v_i) {
    
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
        return true;
    }
    return false;
}

void cSynthDistClamp(struct CVoice *voice, int16_t *byte_stream, long length) {
    
    const int16_t clamp_base = 10000;
    int16_t positive_dist_clamp = clamp_base;
    int16_t negative_dist_clamp = -clamp_base;
    
    for(int i = 0; i < length; i++) {
        byte_stream[i] *= voice->dist_amp_pre_clamp;
        if(byte_stream[i] > positive_dist_clamp) {
            byte_stream[i] = positive_dist_clamp;
            byte_stream[i] *= voice->dist_amp_post_clamp;
        } else if(byte_stream[i] < negative_dist_clamp) {
            byte_stream[i] = negative_dist_clamp;
            byte_stream[i] *= voice->dist_amp_post_clamp;
        }
    }
}

void cSynthAddSamples(struct CSynthContext *synth, int voice_index, struct CVoice *voice, int sample_left, int sample_right, int16_t *byte_stream, int index_left, int index_right, double amp_left, double amp_right) {
    
    const int16_t clamp_base = 10000;
    int16_t positive_dist_clamp = clamp_base;
    int16_t negative_dist_clamp = -clamp_base;
    
    
    if(voice->dist_active) {
        
        sample_left *= voice->dist_amp_pre_clamp;
        if(sample_left > positive_dist_clamp) {
            sample_left = positive_dist_clamp;
        } else if(sample_left < negative_dist_clamp) {
            sample_left = negative_dist_clamp;
        }
        
        sample_right *= voice->dist_amp_pre_clamp;
        if(sample_right > positive_dist_clamp) {
            sample_right = positive_dist_clamp;
        } else if(sample_right < negative_dist_clamp) {
            sample_right = negative_dist_clamp;
        }
        
        sample_left *= voice->dist_amp_post_clamp;
        sample_right *= voice->dist_amp_post_clamp;
        
        if(index_left < synth->temp_mixdown_size) {
            synth->temp_mixdown_buffer[voice_index][index_left] = (int16_t)(sample_left * amp_left);
        }
        
        if(index_right < synth->temp_mixdown_size) {
            synth->temp_mixdown_buffer[voice_index][index_right] = (int16_t)(sample_right * amp_right);
        }
    }
    else {
        if(index_left < synth->temp_mixdown_size) {
            synth->temp_mixdown_buffer[voice_index][index_left] = (int16_t)(sample_left * amp_left);
        }
        
        if(index_right < synth->temp_mixdown_size) {
            synth->temp_mixdown_buffer[voice_index][index_right] = (int16_t)(sample_right * amp_right);
        }
    }
}


