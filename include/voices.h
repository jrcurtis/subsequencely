
#ifndef VOICES_H
#define VOICES_H

#include "app.h"
#include "util.h"

#define NUM_VOICES          (GRID_SIZE)

typedef struct
{
    s8 note_number;
    u8 aftertouch;
} Voice;

/// Voices manages keeping track of which notes are being held down, so that
/// when one is released, the next most recent one can take over, and so that
/// aftertouch can be managed by keeping track of the highest aftertouch value
/// currently held.
typedef struct
{
    s8 max_index;
    u8 velocity;
    u8 aftertouch;
    Voice voices[GRID_SIZE];
} Voices;

/// Initializes voices to an empty state.
void voices_init(Voices* vs);

/// Called when a new note is pressed down.
void voices_add(Voices* vs, u8 note_number, u8 velocity);

/// Called when a note is released.
void voices_remove(Voices* vs, u8 note_number);

/// Sets the aftertouch of an already-held note, and potentially updates the
/// channel aftertouch value.
u8 voices_handle_aftertouch(Voices* vs, u8 note_number, u8 aftertouch);

/// Gets the most recently pressed note number, or -1 if no notes are held.
s8 voices_get_newest(Voices* vs);

/// Gets the number of held notes.
u8 voices_get_num_active(Voices* vs);

/// Quickly resets all voices.
void voices_reset(Voices* vs);

#endif
