#pragma once

#include <stddef.h>
#include <stdio.h>
#include <Arduino.h>
#include "../board.h"

enum MidiClkType {
    MidiClkInt,
    MidiClkExt,
};

enum MidiChannel {
    MidiChannel0   = 0,
    MidiChannel1   = 1,
    MidiChannel2   = 2,
    MidiChannel3   = 3,
    MidiChannel4   = 4,
    MidiChannel5   = 5,
    MidiChannel6   = 6,
    MidiChannel7   = 7,
    MidiChannel8   = 8,
    MidiChannel9   = 9,
    MidiChannel10  = 10,
    MidiChannel11  = 11,
    MidiChannel12  = 12,
    MidiChannel13  = 13,
    MidiChannel14  = 14,
    MidiChannel15  = 15,
    MidiChannel16  = 16,
    MidiChannelAll = 17
};

enum MidiOutType {
    MidiOutGate,
    MidiOutPitch,
    MidiOutVelocity,
    MidiOutAfterTouch,
    MidiOutPitchBend,
    MidiOutCc0,
    MidiOutCc1,
    MidiOutCc2,
    MidiOutCc3,
    MidiOutCc4,
    MidiOutCc5,
    MidiOutCc6,
    MidiOutCc7,
    MidiOutCc8,
    MidiOutCc9,
    MidiOutCc10,
    MidiOutCc11,
    MidiOutCc12,
    MidiOutCc13,
    MidiOutCc14,
    MidiOutCc15,
    MidiOutCc16,
    MidiOutCc17,
    MidiOutCc18,
    MidiOutCc19,
    MidiOutCc20,
    MidiOutCc21,
    MidiOutCc22,
    MidiOutCc23,
    MidiOutCc24,
    MidiOutCc25,
    MidiOutCc26,
    MidiOutCc27,
    MidiOutCc28,
    MidiOutCc29,
    MidiOutCc30,
    MidiOutCc31,
    MidiOutCc32,
    MidiOutCc33,
    MidiOutCc34,
    MidiOutCc35,
    MidiOutCc36,
    MidiOutCc37,
    MidiOutCc38,
    MidiOutCc39,
    MidiOutCc40,
    MidiOutCc41,
    MidiOutCc42,
    MidiOutCc43,
    MidiOutCc44,
    MidiOutCc45,
    MidiOutCc46,
    MidiOutCc47,
    MidiOutCc48,
    MidiOutCc49,
    MidiOutCc50,
    MidiOutCc51,
    MidiOutCc52,
    MidiOutCc53,
    MidiOutCc54,
    MidiOutCc55,
    MidiOutCc56,
    MidiOutCc57,
    MidiOutCc58,
    MidiOutCc59,
    MidiOutCc60,
    MidiOutCc61,
    MidiOutCc62,
    MidiOutCc63,
    MidiOutCc64,
    MidiOutCc65,
    MidiOutCc66,
    MidiOutCc67,
    MidiOutCc68,
    MidiOutCc69,
    MidiOutCc70,
    MidiOutCc71,
    MidiOutCc72,
    MidiOutCc73,
    MidiOutCc74,
    MidiOutCc75,
    MidiOutCc76,
    MidiOutCc77,
    MidiOutCc78,
    MidiOutCc79,
    MidiOutCc80,
    MidiOutCc81,
    MidiOutCc82,
    MidiOutCc83,
    MidiOutCc84,
    MidiOutCc85,
    MidiOutCc86,
    MidiOutCc87,
    MidiOutCc88,
    MidiOutCc89,
    MidiOutCc90,
    MidiOutCc91,
    MidiOutCc92,
    MidiOutCc93,
    MidiOutCc94,
    MidiOutCc95,
    MidiOutCc96,
    MidiOutCc97,
    MidiOutCc98,
    MidiOutCc99,
    MidiOutCc100,
    MidiOutCc101,
    MidiOutCc102,
    MidiOutCc103,
    MidiOutCc104,
    MidiOutCc105,
    MidiOutCc106,
    MidiOutCc107,
    MidiOutCc108,
    MidiOutCc109,
    MidiOutCc110,
    MidiOutCc111,
    MidiOutCc112,
    MidiOutCc113,
    MidiOutCc114,
    MidiOutCc115,
    MidiOutCc116,
    MidiOutCc117,
    MidiOutCc118,
    MidiOutCc119,
    MidiOutCc120,
    MidiOutCc121,
    MidiOutCc122,
    MidiOutCc123,
    MidiOutCc124,
    MidiOutCc125,
    MidiOutCc126,
    MidiOutCc127
};

class MidiSettingsState {
public:
    const static int MAX_BPM = 255;
    const static int MIN_BPM = 1;
    const static int MAX_MIDI_CHANNEL = MidiChannelAll;
    const static int MIN_MIDI_CHANNEL = MidiChannel0;
    const static int MAX_MIDI_OUT_TYPE = MidiOutCc127;
    const static int MIN_MIDI_OUT_TYPE = MidiOutGate;
    const static int MAX_MIDI_CLK_TYPE = MidiClkExt;
    const static int MIN_MIDI_CLK_TYPE = MidiClkInt;

    MidiSettingsState(void);
    ~MidiSettingsState(void);

    void begin(void);
    void store(void);
    void recall(void);

    const char* get_bpm_str(void);
    const char* get_midi_channel_str(void);
    const char* get_midi_out_type_str(size_t idx);
    const char* get_midi_clk_type_str(void);

    void set_bpm(int bpm);
    void set_midi_channel(MidiChannel ch);
    void set_midi_out_type(size_t idx, MidiOutType type);
    void set_midi_out_channel(size_t idx, MidiChannel ch);
    void set_midi_clk_type(MidiClkType type);

    int get_bpm(void);
    MidiChannel get_midi_channel(void);
    MidiOutType get_midi_out_type(size_t idx);
    MidiChannel get_midi_out_channel(size_t idx);
    MidiClkType get_midi_clk_type(void);

    int get_max_bpm(void) { return MAX_BPM; }
    int get_min_bpm(void) { return MIN_BPM; }
    int get_max_midi_channel(void) { return MAX_MIDI_CHANNEL; }
    int get_min_midi_channel(void) { return MIN_MIDI_CHANNEL; }
    int get_max_midi_out_type(size_t idx);
    int get_min_midi_out_type(size_t idx);
    int get_max_midi_clk_type(void) { return MAX_MIDI_CLK_TYPE; }
    int get_min_midi_clk_type(void) { return MIN_MIDI_CLK_TYPE; }
    
private:
    int bpm;
    MidiChannel midi_channel;
    MidiOutType midi_out_type[PWM_COUNT];
    MidiChannel midi_out_channel[PWM_COUNT];
    MidiClkType midi_clk_type;
    SemaphoreHandle_t state_mutex;

    const char* midi_channel_to_string(MidiChannel ch);
    const char* midi_out_type_to_string(MidiOutType type);
    const char* midi_clk_type_to_string(MidiClkType type);
    void set_default(void);
    esp_err_t recall_nvs(void);
    esp_err_t store_nvs(void);
};
