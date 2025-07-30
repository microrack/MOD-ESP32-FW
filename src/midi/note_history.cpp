#include "note_history.h"

void NoteHistory::Note::reset(void) {
    prev = NO_NOTE;
    next = NO_NOTE;
    in_use = false;
}

bool NoteHistory::push(uint8_t note) {
    if (history[note].in_use) {
        return false;
    }

    history[note].in_use = true;
    history[note].prev = last;
    history[last].next = note;
    last = note;
    return true;
}

bool NoteHistory::pop(uint8_t note, uint8_t& prev_note) {
    if (!history[note].in_use)
        return false;

    uint8_t prev = history[note].prev;
    uint8_t next = history[note].next;

    if (note != last) {
        history[prev].next = next;
        history[next].prev = prev;
    } else {
        last = prev;
        history[prev].next = NO_NOTE;
    }

    history[note].reset();
    return true;
}

bool NoteHistory::is_in_use(uint8_t note) {
    return history[note].in_use;
}

uint8_t NoteHistory::get_prev(uint8_t note) {
    return history[note].prev;
}

uint8_t NoteHistory::get_next(uint8_t note) {
    return history[note].next;
}

void NoteHistory::reset(void) {
    for (int i = 0; i < MIDI_NOTES_COUNT; i++) {
        history[i].reset();
    }
    last = NO_NOTE;
    count = 0;
}

int NoteHistory::get_count(void) { return count; }

bool NoteHistory::is_empty(void) { return count == 0; }

uint8_t NoteHistory::get_last(void) { return last; }
