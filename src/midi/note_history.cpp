#include "note_history.h"
#include <Arduino.h>
#include <algorithm>
#include "../board.h"

void NoteHistory::Note::reset(void)
{
    prev = NO_NOTE;
    next = NO_NOTE;
    in_use = false;
    id = 0;
}

bool NoteHistory::push(uint8_t note, uint8_t* out_id) {
    if(DEBUG_MIDI_PROCESSOR) Serial.printf("  pushing note: %d\n", note);
    
    if (history[note].in_use) {
        Serial.println("  push FAILED: note already in use");
        return false;
    }

    // Find the minimum available id
    uint8_t new_id = 0;
    if (last != NO_NOTE) {
        // Collect all used ids from the linked list
        uint8_t used_ids[MIDI_NOTES_COUNT];
        uint8_t ids_count = 0;
        uint8_t current = last;
        
        // Traverse the linked list and collect all ids
        do {
            used_ids[ids_count++] = history[current].id;
            current = history[current].prev;
        } while (current != NO_NOTE);
        
        // Sort
        std::sort(used_ids, used_ids + ids_count);
        
        // Find the first gap in one pass
        for (uint8_t i = 0; i < ids_count; i++) {
            if (used_ids[i] != i) {
                new_id = i;
                break;
            }
            new_id = i + 1;
        }
    }

    history[note].in_use = true;
    history[note].id = new_id;
    history[note].prev = last;
    if (last != NO_NOTE) {
        history[last].next = note;
    }
    last = note;
    count++;
    
    if (out_id != nullptr) {
        *out_id = new_id;
    }
    
    return true;
}

bool NoteHistory::pop(uint8_t note, uint8_t* out_id) {
    if(DEBUG_MIDI_PROCESSOR) Serial.printf("  popping note: %d\n", note);
    
    if (!history[note].in_use) {
        Serial.println("  pop FAILED: note not in use");
        return false;
    }

    uint8_t note_id = history[note].id;
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
    count--;

    if (out_id != nullptr) {
        *out_id = note_id;
    }

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
