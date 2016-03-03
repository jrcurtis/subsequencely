
#ifndef SCALE_H
#define SCALE_H

#include "app.h"
#include "util.h"

/// Represents a musical scale as a subset of 12 chromatic notes.
typedef struct
{
    /// Number of notes currently enabled
    uint8_t num_notes;

    /// Bitfield indicating whether the nth note (relative to the root
    /// note) is enabled. [0] (the root note) is always on.
    uint16_t notes;

    /// Array of ints indicating how many half steps the nth scale note is
    /// offset from the root note (only the first num_notes entries are set)
    int8_t offsets[NUM_NOTES];
} Scale;


/// Initializes the scale data to a chromatic scale.
void scale_init(Scale* s);

/// Returns true or false depending on whether then nth note (counted from the
/// root note) is contained in the scale.
uint8_t scale_contains_note(Scale* s, uint8_t note);

/// Sets all the notes of the scale at once.
void scale_set_notes(Scale* s, uint16_t notes);

/// Toggles the nth note in the scale. Automatically updates num_notes and
/// offsets.
void scale_toggle_note(Scale* s, uint8_t note);

#endif
