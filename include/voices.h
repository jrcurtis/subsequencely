
#ifndef VOICES_H
#define VOICES_H

#include "app.h"
#include "note.h"
#include "util.h"

#define NUM_VOICES          (GRID_SIZE)

/// Voices manages keeping track of which notes are being held down, so that
/// when one is released, the next most recent one can take over, and so that
/// aftertouch can be managed by keeping track of the highest aftertouch value
/// currently held.
typedef struct
{
    int8_t num_active;
    int8_t num_sustained;
    uint8_t velocity;
    int8_t aftertouch;
    Note voices[NUM_VOICES];
} Voices;

/// Initializes voices to an empty state.
void voices_init(Voices* vs);

/// Called when a new note is pressed down.
Note* voices_add(Voices* vs, uint8_t note_number, uint8_t velocity);

/// Called when a note is released.
Note* voices_remove(Voices* vs, uint8_t note_number);

/// Enables or disables sustaining the voices that are currently active.
void voices_sustain(Voices* vs, uint8_t enable);

/// Remove all of the sustained notes.
void voices_remove_sustained(Voices* vs);

/// Sets the aftertouch of an already-held note, and potentially updates the
/// channel aftertouch value.
uint8_t voices_handle_aftertouch(Voices* vs, int8_t note_number, int8_t aftertouch);

/// Gets the most recently pressed note number, or -1 if no notes are held.
int8_t voices_get_newest(Voices* vs);

/// Gets the number of held notes.
uint8_t voices_get_num_active(Voices* vs);

/// Gets number of notes being sustained.
uint8_t voices_get_num_sustained(Voices* vs);

/// Returns whether that note is being kept on by the sustain function.
uint8_t voices_is_sustained(Voices* vs, uint8_t note_number);

/// Quickly resets all voices.
void voices_reset(Voices* vs);

#endif
