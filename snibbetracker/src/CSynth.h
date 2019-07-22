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

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "cJSON/cJSON.h"

#ifndef cengine_CSynth_h
#define cengine_CSynth_h

struct CSynthContext {
    
    double base_mod_noteoff_slope;
    double base_mod_adsr_cursor;
    double base_mod_advance_amp_targets;
    double base_mod_vibrato_speed;
    double base_mod_vibrato_depth;
    double base_mod_pitch_1;
    double base_mod_pitch_2;
    double base_mod_arp_speed;
    double base_mod_portamento_speed;
    double base_mod_pwm_speed;
    double base_mod_detune;
    
    double mod_noteoff_slope;
    double mod_adsr_cursor;
    double mod_advance_amp_targets;
    double mod_vibrato_speed;
    double mod_vibrato_depth;
    double mod_pitch_1;
    double mod_pitch_2;
    double mod_arp_speed;
    double mod_portamento_speed;
    double mod_pwm_speed;
    double mod_detune;
    
    struct CStr *str_title;
    struct CStr *str_author;
    struct CStr *str_info;
    bool debuglog;
    bool errorlog;
    bool interleaved;
    double master_amp;
    int master_amp_percent;
    int default_bpm;
    bool audio_clips;
    bool preview_enabled;
    bool sustain_active;
    bool preview_locked;
    bool preview_started;
    int chunk_size;
    int build_number;
    int file_version;
    int sample_rate;
    int frame_rate;
    double samples_per_minute;
    bool looped; // query to find out if the song has looped.
    struct CVoice **voices;
    struct CInstrument **instruments;
    struct CInstrument *custom_instrument;
    int max_voices;
    int max_instruments;
    int current_instrument;
    int *active_tracks;
    int solo_track;
    int solo_voice;
    int wave_length;
    int noise_length;
    struct CTrackNode ****track;
    struct CTrackNode ***swap_track;
    struct CTrackNode ***copy_track;
    struct CTrackNode ***instrument_effects;
    int max_instrument_effects;
    int max_tracks_and_patterns;
    int track_width;
    int track_height; // number of active rows in a track.
    int track_max_height;
    double track_progress;
    int track_progress_int;
    int track_cursor_x; // will only go to max_voices.
    int track_cursor_y;
    bool track_advance;
    int current_track;
    int **patterns_and_voices;
    int patterns_and_voices_width;
    int patterns_and_voices_height;
    int **patterns;
    int patterns_width;
    int patterns_height;
    int max_adsr_nodes;
    int arpeggio_speed;
    bool needs_redraw;
    int16_t **temp_mixdown_buffer;
    int temp_mixdown_size;
    int track_highlight_interval;
    int tempo_width;
    int tempo_height;
    struct CTempoNode ***tempo_map;
    int wavetable_width;
    int wavetable_height;
    struct CWavetableNode ***wavetable_map;
    int current_tempo_column;
    int tempo_index;
    int pending_tempo_column;
    double pending_tempo_blink_counter;
    double ticks_modifier;
    bool pending_tempo_blink_counter_toggle;
    bool tempo_skip_step;
    bool bitcrush_active;
    double bitcrush_depth;
    
    // tables
    int16_t *sine_wave_table;
    int16_t *sawtooth_wave_table;
    int16_t *square_wave_table;
    int16_t *triangle_wave_table;
    int16_t *noise_table;
    int16_t *custom_table;
};

struct CadsrNode {
    double pos;
    double amp;
};

struct CInstrument {
    double volume_scalar;
    struct CadsrNode **adsr;
    double adsr_cursor;
    int adsr_nodes;
    int instrument_number;
};

struct CVoice {
    int muted;
    double adsr_cursor; // keep a cursor for voice to use same instrument for different voices.
    int int_id;
    struct CInstrument *instrument;
    int previous_instrument_number; // use to clear out preset effects when switching instrument.
    bool note_on;
    int16_t *waveform;
    int waveform_length;
    double volume;
    double phase_double;
    double phase_double_inc;
    int phase_int;
    bool active;
    int tone;
    int last_valid_tone;
    int previous_tone;
    int previous_effect;
    int previous_effect_param1;
    int previous_effect_param2;
    double tone_with_fx;
    int last_preview_tone;
    bool sustain_active;
    
    // vibrato
    bool vibrato_active;
    double vibrato_depth;
    double vibrato_speed;
    double vibrato_phase;
    int last_vibrato_effect1;
    int last_vibrato_effect2;
    
    // pitch
    bool pitch_up_active;
    double pitch_speed1;
    double pitch_speed2;
    double pitch_up_down_phase;
    bool pitch_down_active;
    int last_pitch_effect1;
    int last_pitch_effect2;
    
    // downsample
    int16_t downsample_last_sample;
    bool downsample_next_sample;
    int downsample_limit;
    double downsample_count;
    bool downsample_sweep_up;
    bool downsample_sweep_down;
    double downsample_sweep_slope;
    double downsample_sweep_start;
    double downsample_sweep_speed;
    int last_downsample_effect1;
    int last_downsample_effect2;
    
    // wavetable
    bool wavetable_active;
    int wavetable_lane;
    double wavetable_cursor;
    int16_t *wavetable_previous_table;
    
    // smoothing for preview. As we cannot look ahead, we need to gracefully end the current note before starting the next.
    bool preview_slope;
    double preview_amp_target;
    double preview_amp_current;
    
    // smoothing/slope
    bool noteoff_slope;
    double noteoff_slope_value;

    // arpeggio
    bool arpeggio_active;
    int arpeggio_tone;
    double arpeggio_count;
    double arpeggio_limit;
    int arpeggio_tone_0;
    int arpeggio_tone_1;
    int arpeggio_tone_2;
    int arpeggio_tone_toggle;
    int last_arpeggio_effect1;
    int last_arpeggio_effect2;
    
    // portamento
    bool portamento_active;
    double portamento_diff;
    double portamento_original_diff;
    double portamento_speed;
    int last_portamento_effect1;
    int last_portamento_effect2;
    
    // amp A
    double amp_left;
    double amp_right;
    double amp_target_left;
    double amp_target_right;
    bool amp_target_left_higher;
    bool amp_target_right_higher;
    bool amp_target_left_reached;
    bool amp_target_right_reached;
    int last_amp_effect1;
    int last_amp_effect2;
    
    // arp speed
    int arp_speed;
    int last_arp_speed_effect1;
    int last_arp_speed_effect2;
    
    // PWM
    bool pwm_active;
    double pwm_pos;
    double pwm_speed;
    double pwm_depth;
    int pwm_static_pos;
    int last_pwm_effect1;
    int last_pwm_effect2;
    
    // FM
    bool fm_active;
    double fm_pos;
    double fm_speed;
    double fm_depth;
    int last_fm_effect1;
    int last_fm_effect2;
    
    // dist
    bool dist_active;
    double dist_amp_pre_clamp;
    double dist_amp_post_clamp;
    int last_dist_effect1;
    int last_dist_effect2;
    
    // delay
    bool delay_active;
    double param_delay_length;
    double param_delay_decay;
    int16_t *delay_buffer;
    int delay_buffer_size;
    int delay_buffer_limit;
    int delay_buffer_write_cursor;
    int delay_buffer_read_cursor;
    
    // 7 detune
    bool detune_active;
    double detune_amount;
};

struct CTrackNode {
    int tone;
    bool tone_active;
    struct CInstrument *instrument;
    int instrument_nr;
    char effect;
    char effect_param1;
    char effect_param2;
    char effect_value;
    char effect_param1_value;
    char effect_param2_value;
};

struct CTempoNode {
    char ticks;
    bool active;
    int bpm;
};

struct CWavetableNode {
    char value;
    bool active;
    int speed;
};

void cSynthInit(struct CSynthContext *synth);
struct CSynthContext *cSynthContextNew(void);
cJSON* cSynthSaveProject(struct CSynthContext *synth);
int cSynthLoadProject(struct CSynthContext *synth, const char* json);
void cSynthReset(struct CSynthContext *synth);
void cSynthCleanup(struct CSynthContext *synth);
void cSynthMoveNotes(struct CSynthContext *synth, bool up, bool down, int cursor_x, int cursor_y, int pattern);
void cSynthAddTrackNodeParams(struct CSynthContext *synth, int current_track, int x, int y, int instrument, char effect, char effect_param1, char effect_param2);
void cSynthRemoveTrackNodeParams(struct CSynthContext *synth, int current_track, int x, int y, bool instrument, bool effect, bool effect_param1, bool effect_param2);
void cSynthTurnOffSustain(struct CSynthContext *synth);
void cSynthAddTrackNode(struct CSynthContext *synth, int instrument_nr, int current_track, int x, int y, bool editing, bool preview, int tone, bool playing);
void cSynthRemoveTrackNode(struct CSynthContext *synth, int current_track, int x, int y);
void cSynthUpdateTrackCursor(struct CSynthContext *synth, int x, int y);
int cSynthGetNextActiveTrack(int current_track, struct CSynthContext *synth, bool forward);
void cSynthAdvanceTrack(struct CSynthContext *synth, long samples);
void cSynthVoiceApplyEffects(struct CSynthContext *synth, struct CVoice *v);
void cSynthAdvanceAmpTargets(struct CSynthContext *synth, struct CVoice *v);
void cSynthRampUpToFullAmp(struct CVoice *v);
int16_t cSynthGetFMSample(struct CSynthContext *synth, struct CVoice *v, double delta_phi);
int16_t cSynthGetPWMSample(struct CSynthContext *synth, struct CVoice *v, int phase_int);
void cSynthVoiceApplyDownsampleSweep(struct CSynthContext *synth, struct CVoice *v, bool up);
void cSynthResetTrackProgress(struct CSynthContext *synth, int start_pattern, int start_row);
void cSynthResetPortamento(struct CVoice *v);
void cSynthResetAllEffects(struct CVoice *v);
void cSynthResetOnLoopBack(struct CSynthContext *synth);
void cSynthResetTempoIndex(struct CSynthContext *synth);
void cSynthUpdateHighlightInterval(struct CSynthContext *synth);
struct CTrackNode *cSynthNewTrackNode(void);
struct CTrackNode *cSynthCopyTracknode(struct CTrackNode *track_node);
void cSynthIncAdsrCursor(struct CVoice *v, struct CInstrument *i, struct CSynthContext *synth);
void cSynthWriteCustomTableFromNodes(struct CSynthContext *synth);
double cSynthInstrumentVolumeByPos(struct CInstrument* i, struct CVoice *v);
double cSynthGetFrequency(double pitch);
void cSynthIncPhase(struct CVoice *v, double inc);
void cSynthInstrumentNoteOn(struct CInstrument *i);
char *cSynthToneToChar(int tone);
char cSynthGetCharFromParam(char value);
char cSynthGetValueForParam(char value);
void cSynthPasteNotesFromPattern(struct CSynthContext *synth, int origin_x, int origin_y, int target_x, int target_y);
void cSynthTransposePattern(struct CSynthContext *synth, int track, int track_x, bool up, int amount);
void cSynthTransposeSelection(struct CSynthContext *synth, int track, int cursor_x, int cursor_y, int selection_x, int selection_y, bool up, int amount);
void cSynthCopyNotesFromSelection(struct CSynthContext *synth, int track, int cursor_x, int cursor_y, int selection_x, int selection_y, bool cut, bool store);
void cSynthPasteNotesToPos(struct CSynthContext *synth, int track, int cursor_x, int cursor_y);
double cSynthGetRelativeModifierReverse(double base, int sample_rate, bool interleaved, bool log);
double cSynthGetRelativeModifier(double base, int sample_rate, bool interleaved, bool log);
double cSynthGetRelativeModForChunksReverse(double base, int sample_rate, int chunk_size, bool interleaved, bool log);
double cSynthGetRelativeModForChunks(double base, int sample_rate, int chunk_size, bool interleaved, bool log);
void cSynthRenderAudio(struct CSynthContext *synth, int16_t *s_byteStream, long begin, long end, long length, bool playing, bool exporting);
bool cSynthChannelShouldBeRendered(struct CSynthContext *synth, int v_i);
void cSynthDistClamp(struct CVoice *voice, int16_t *byte_stream, long length);
void cSynthAddSamples(struct CSynthContext *synth, int voice_index, struct CVoice *voice, int sample_left, int sample_right, int16_t *byte_stream, int index_left, int index_right, double amp_left, double amp_right);


#endif /* defined(__synth__CSynth__) */
