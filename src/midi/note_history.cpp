#include "note_history.h"
#include <Arduino.h>

void NoteHistory::Note::reset(void)
{
    prev = NO_NOTE;
    next = NO_NOTE;
    in_use = false;
}

bool NoteHistory::push(uint8_t note) {
    Serial.printf("  pushing note: %d\n", note);
    
    if (history[note].in_use) {
        Serial.println("  push FAILED: note already in use");
        return false;
    }

    history[note].in_use = true;
    history[note].prev = last;
    if (last != NO_NOTE) {
        history[last].next = note;
    }
    last = note;
    
    return true;
}

bool NoteHistory::pop(uint8_t note) {
    Serial.printf("  popping note: %d\n", note);
    
    if (!history[note].in_use) {
        Serial.println("  pop FAILED: note not in use");
        return false;
    }

    uint8_t prev = history[note].prev;
    uint8_t next = history[note].next;

    if (note != last) {
        history[prev].next = next;
        history[next].prev = prev;
    } else {
        last = prev;
        if (prev != NO_NOTE) {
            history[prev].next = NO_NOTE;
        }
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

int NoteHistory::get_count(void) {
    return count;
}

bool NoteHistory::is_empty(void) {
    return count == 0;
}

uint8_t NoteHistory::get_last(void) {
    return last;
}

uint8_t NoteHistory::get_current(void) {
    for (int i = MIDI_NOTES_COUNT - 1; i >= 0; i--) {
        if (history[i].in_use) {
            return i;
        }
    }
    return NO_NOTE;
}
