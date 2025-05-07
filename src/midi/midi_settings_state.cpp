#include <EEPROM.h>
#include "midi_settings_state.h"

MidiSettingsState::MidiSettingsState(void) {
    recall();
}

void MidiSettingsState::store(void) {
    uint32_t addr = MIDI_SETTINGS_EEPROM_ADDR;
    EEPROM.writeUInt(addr, magic);
    addr += sizeof(magic);
    EEPROM.writeUInt(addr, version);
    addr += sizeof(version);
    EEPROM.writeUInt(addr, bpm);
    addr += sizeof(bpm);
    EEPROM.writeUInt(addr, midi_channel);
    addr += sizeof(midi_channel);
    for (size_t i = 0; i < MIDI_OUT_COUNT; i++) {
        EEPROM.writeUInt(addr, midi_out_type[i]);
        addr += sizeof(midi_out_type[i]);
    }
    EEPROM.writeUInt(addr, midi_clk_type);
    addr += sizeof(midi_clk_type);
    EEPROM.writeUInt(addr, magic);
    EEPROM.commit();
}

void MidiSettingsState::recall(void) {
    uint32_t addr = MIDI_SETTINGS_EEPROM_ADDR;
    magic = EEPROM.readUInt(addr);
    addr += sizeof(magic);
    version = EEPROM.readUInt(addr);
    addr += sizeof(version);
    bpm = EEPROM.readUInt(addr);
    addr += sizeof(bpm);
    midi_channel = (MidiChannel)EEPROM.readUInt(addr);
    addr += sizeof(midi_channel);
    for (size_t i = 0; i < MIDI_OUT_COUNT; i++) {
        midi_out_type[i] = (MidiOutType)EEPROM.readUInt(addr);
        addr += sizeof(midi_out_type[i]);
    }
    midi_clk_type = (MidiClkType)EEPROM.readUInt(addr);
    addr += sizeof(midi_clk_type);
    magic = EEPROM.readUInt(addr);

    if (magic != MAGIC || version != VERSION) {
        set_default();
        store();
    }
}

const char* MidiSettingsState::get_bpm_str(void) {
    static char bpm_str[10];
    snprintf(bpm_str, sizeof(bpm_str), "%d", bpm);
    return bpm_str;
}

const char* MidiSettingsState::get_midi_channel_str(void) {
    return midi_channel_to_string(midi_channel);
}

const char* MidiSettingsState::get_midi_out_type_str(size_t idx) {
    return midi_out_type_to_string(midi_out_type[idx]);
}

const char* MidiSettingsState::get_midi_clk_type_str(void) {
    return midi_clk_type_to_string(midi_clk_type);
}

const char* MidiSettingsState::midi_channel_to_string(MidiChannel ch) {
    if (ch == MidiChannelAll) {
        return "all";
    }
    if (ch >= MidiChannel0 && ch <= MidiChannel16) {
        static char buf[10];
        snprintf(buf, sizeof(buf), "%d", (int)ch);
        return buf;
    }
    return "unknown";
}

const char* MidiSettingsState::midi_out_type_to_string(MidiOutType type) {
    switch (type) {
        case MidiOutGate:        return "gate";
        case MidiOutPitch:       return "pitch";
        case MidiOutVelocity:    return "velocity";
        case MidiOutAfterTouch:  return "aftertouch";
        case MidiOutPitchBend:   return "pitchbend";
        default:
            if (type >= MidiOutCc0 && type <= MidiOutCc127) {
                static char buf[8];
                int cc = type - MidiOutCc0;
                snprintf(buf, sizeof(buf), "cc%d", cc);
                return buf;
            }
            return "unknown";
    }
}

const char* MidiSettingsState::midi_clk_type_to_string(MidiClkType type) {
    switch (type) {
        case MidiClkInt: return "int";
        case MidiClkExt: return "ext";
        default: return "unknown";
    }
}

void MidiSettingsState::set_default(void) {
    magic = MAGIC;
    version = VERSION;
    bpm = 120;
    midi_channel = MidiChannelAll;
    for (size_t i = 0; i < MIDI_OUT_COUNT; i++) {
        midi_out_type[i] = MidiOutPitch;
    }
    midi_clk_type = MidiClkInt;
}
