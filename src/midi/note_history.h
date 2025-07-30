#pragma once

#include <stddef.h>
#include <stdint.h>

struct NoteHistory {
    static const size_t MIDI_NOTES_COUNT = 127;
    static const uint8_t NO_NOTE = (1 << 7);

    struct Note {
        uint8_t prev;
        uint8_t next;
        bool in_use;

        void reset(void);

        Note() { reset(); }
    };

    Note history[MIDI_NOTES_COUNT];
    int last;
    int count;

    bool push(uint8_t note);
    bool pop(uint8_t note, uint8_t& prev_note);
    bool is_in_use(uint8_t note);
    uint8_t get_prev(uint8_t note);
    uint8_t get_next(uint8_t note);
    void reset(void);
    int get_count(void);
    bool is_empty(void);
    uint8_t get_last(void);

    NoteHistory() { reset(); }
};
