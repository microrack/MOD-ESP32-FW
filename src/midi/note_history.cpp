#include "note_history.h"
#include <Arduino.h>
#include "../board.h"

void NoteHistory::Note::reset(void)
{
    prev = NO_NOTE;
    next = NO_NOTE;
    in_use = false;
}

bool NoteHistory::push(uint8_t note) {
    if(DEBUG_MIDI_PROCESSOR) Serial.printf("  pushing note: %d\n", note);
    
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
    if(DEBUG_MIDI_PROCESSOR) Serial.printf("  popping note: %d\n", note);
    
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
    if (last == NO_NOTE) {
        return NO_NOTE;
    }
    
    uint8_t max_note = last;
    uint8_t current = history[last].prev;
    
    while (current != NO_NOTE) {
        if (current > max_note) {
            max_note = current;
        }
        current = history[current].prev;
    }
    
    return max_note;
}
