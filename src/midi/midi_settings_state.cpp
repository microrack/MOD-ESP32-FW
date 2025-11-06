#include <nvs.h>
#include <nvs_flash.h>
#include <esp_err.h>
#include "midi_settings_state.h"

#define NVS_NAMESPACE "midi_settings"

MidiSettingsState::MidiSettingsState(void) {
    // Initialize mutex to nullptr
    state_mutex = nullptr;
}

MidiSettingsState::~MidiSettingsState(void) {
    if (state_mutex != nullptr) {
        vSemaphoreDelete(state_mutex);
    }
}

void MidiSettingsState::begin(void) {
    // Create mutex for thread safety
    state_mutex = xSemaphoreCreateMutex();
    recall();
}

void MidiSettingsState::store(void) {
    if (xSemaphoreTake(state_mutex, portMAX_DELAY) == pdTRUE) {
        store_nvs();
        xSemaphoreGive(state_mutex);
    }
}

esp_err_t MidiSettingsState::store_nvs(void) {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        Serial.printf("store_nvs: failed to open NVS namespace, err=0x%x\n", err);
        return err;
    }

    err = nvs_set_u32(nvs_handle, "bpm", (uint32_t)bpm);
    if (err != ESP_OK) {
        Serial.printf("store_nvs: failed to set bpm, err=0x%x\n", err);
        nvs_close(nvs_handle);
        return err;
    }

    err = nvs_set_u32(nvs_handle, "midi_channel", (uint32_t)midi_channel);
    if (err != ESP_OK) {
        Serial.printf("store_nvs: failed to set midi_channel, err=0x%x\n", err);
        nvs_close(nvs_handle);
        return err;
    }

    for (size_t i = 0; i < PWM_COUNT; i++) {
        char key[20];
        snprintf(key, sizeof(key), "midi_out_type_%zu", i);
        err = nvs_set_u32(nvs_handle, key, (uint32_t)midi_out_type[i]);
        if (err != ESP_OK) {
            Serial.printf("store_nvs: failed to set %s, err=0x%x\n", key, err);
            nvs_close(nvs_handle);
            return err;
        }
    }

    err = nvs_set_u32(nvs_handle, "midi_clk_type", (uint32_t)midi_clk_type);
    if (err != ESP_OK) {
        Serial.printf("store_nvs: failed to set midi_clk_type, err=0x%x\n", err);
        nvs_close(nvs_handle);
        return err;
    }

    err = nvs_commit(nvs_handle);
    if (err != ESP_OK) {
        Serial.printf("store_nvs: failed to commit, err=0x%x\n", err);
        nvs_close(nvs_handle);
        return err;
    }

    nvs_close(nvs_handle);
    return ESP_OK;
}

void MidiSettingsState::recall(void) {
    if (xSemaphoreTake(state_mutex, portMAX_DELAY) == pdTRUE) {
        esp_err_t err = recall_nvs();
        if (err != ESP_OK) {
            set_default();
            store_nvs();
        }
        xSemaphoreGive(state_mutex);
    }
}

esp_err_t MidiSettingsState::recall_nvs(void) {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        Serial.printf("recall_nvs: failed to open NVS namespace, err=0x%x\n", err);
        return err;
    }

    uint32_t bpm_val;
    err = nvs_get_u32(nvs_handle, "bpm", &bpm_val);
    if (err != ESP_OK) {
        Serial.printf("recall_nvs: failed to get bpm, err=0x%x\n", err);
        nvs_close(nvs_handle);
        return err;
    }
    bpm = (int)bpm_val;

    uint32_t ch_val;
    err = nvs_get_u32(nvs_handle, "midi_channel", &ch_val);
    if (err != ESP_OK) {
        Serial.printf("recall_nvs: failed to get midi_channel, err=0x%x\n", err);
        nvs_close(nvs_handle);
        return err;
    }
    midi_channel = (MidiChannel)ch_val;

    for (size_t i = 0; i < PWM_COUNT; i++) {
        char key[20];
        snprintf(key, sizeof(key), "midi_out_type_%zu", i);
        uint32_t type_val;
        err = nvs_get_u32(nvs_handle, key, &type_val);
        if (err != ESP_OK) {
            Serial.printf("recall_nvs: failed to get %s, err=0x%x\n", key, err);
            nvs_close(nvs_handle);
            return err;
        }
        midi_out_type[i] = (MidiOutType)type_val;
    }

    uint32_t clk_val;
    err = nvs_get_u32(nvs_handle, "midi_clk_type", &clk_val);
    if (err != ESP_OK) {
        Serial.printf("recall_nvs: failed to get midi_clk_type, err=0x%x\n", err);
        nvs_close(nvs_handle);
        return err;
    }
    midi_clk_type = (MidiClkType)clk_val;

    nvs_close(nvs_handle);
    return ESP_OK;
}

void MidiSettingsState::set_bpm(int bpm) {
    if (xSemaphoreTake(state_mutex, portMAX_DELAY) == pdTRUE) {
        this->bpm = bpm;
        xSemaphoreGive(state_mutex);
    }
}

void MidiSettingsState::set_midi_channel(MidiChannel ch) {
    if (xSemaphoreTake(state_mutex, portMAX_DELAY) == pdTRUE) {
        this->midi_channel = ch;
        xSemaphoreGive(state_mutex);
    }
}

void MidiSettingsState::set_midi_out_type(size_t idx, MidiOutType type) {
    if (xSemaphoreTake(state_mutex, portMAX_DELAY) == pdTRUE) {
        if (idx < PWM_COUNT) {
            this->midi_out_type[idx] = type;
        }
        xSemaphoreGive(state_mutex);
    }
}

void MidiSettingsState::set_midi_clk_type(MidiClkType type) {
    if (xSemaphoreTake(state_mutex, portMAX_DELAY) == pdTRUE) {
        this->midi_clk_type = type;
        xSemaphoreGive(state_mutex);
    }
}

int MidiSettingsState::get_bpm(void) {
    int result = 0;
    if (xSemaphoreTake(state_mutex, portMAX_DELAY) == pdTRUE) {
        result = this->bpm;
        xSemaphoreGive(state_mutex);
    }
    return result;
}

MidiChannel MidiSettingsState::get_midi_channel(void) {
    MidiChannel result = MidiChannel0;
    if (xSemaphoreTake(state_mutex, portMAX_DELAY) == pdTRUE) {
        result = this->midi_channel;
        xSemaphoreGive(state_mutex);
    }
    return result;
}

MidiOutType MidiSettingsState::get_midi_out_type(size_t idx) {
    MidiOutType result = MidiOutGate;
    if (xSemaphoreTake(state_mutex, portMAX_DELAY) == pdTRUE) {
        if (idx < PWM_COUNT) {
            result = this->midi_out_type[idx];
        }
        xSemaphoreGive(state_mutex);
    }
    return result;
}

MidiClkType MidiSettingsState::get_midi_clk_type(void) {
    MidiClkType result = MidiClkInt;
    if (xSemaphoreTake(state_mutex, portMAX_DELAY) == pdTRUE) {
        result = this->midi_clk_type;
        xSemaphoreGive(state_mutex);
    }
    return result;
}

const char* MidiSettingsState::get_bpm_str(void) {
    static char bpm_str[10];
    if (xSemaphoreTake(state_mutex, portMAX_DELAY) == pdTRUE) {
        snprintf(bpm_str, sizeof(bpm_str), "%d", bpm);
        xSemaphoreGive(state_mutex);
    }
    return bpm_str;
}

const char* MidiSettingsState::get_midi_channel_str(void) {
    MidiChannel ch = get_midi_channel();
    return midi_channel_to_string(ch);
}

const char* MidiSettingsState::get_midi_out_type_str(size_t idx) {
    MidiOutType type = get_midi_out_type(idx);
    return midi_out_type_to_string(type);
}

const char* MidiSettingsState::get_midi_clk_type_str(void) {
    MidiClkType type = get_midi_clk_type();
    return midi_clk_type_to_string(type);
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

int MidiSettingsState::get_max_midi_out_type(size_t idx) {
    if (idx >= PWM_COUNT) return 0;
    if (idx < 0) return 0;

    if (OUT_CHANNELS[idx].isPwm) {
        return MAX_MIDI_OUT_TYPE;
    } else {
        return MidiOutGate;
    }
}

int MidiSettingsState::get_min_midi_out_type(size_t idx) {
    if (idx >= PWM_COUNT) return 0;
    if (idx < 0) return 0;

    if (OUT_CHANNELS[idx].isPwm) {
        return MIN_MIDI_OUT_TYPE;
    } else {
        return MidiOutGate;
    }
}

void MidiSettingsState::set_default(void) {
    bpm = 120;
    midi_channel = MidiChannelAll;
    for (size_t i = 0; i < PWM_COUNT; i++) {
        midi_out_type[i] = MidiOutPitch;
    }
    midi_clk_type = MidiClkInt;
}
