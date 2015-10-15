
#ifndef SCALE_H
#define SCALE_H

#include "app.h"
#include "util.h"

/// Represents a musical scale as a subset of 12 chromatic notes.
typedef struct
{
    /// Number of notes currently enabled
    u8 num_notes;

    /// Bitfield indicating whether the nth note (relative to the root
    /// note) is enabled. [0] (the root note) is always on.
    u16 notes;

    /// Array of ints indicating how many half steps the nth scale note is
    /// offset from the root note (only the first num_notes entries are set)
    s8 offsets[NUM_NOTES];
} Scale;


/// Initializes the scale data to a chromatic scale.
void scale_init(Scale* s);

u8 scale_contains_note(Scale* s, u8 note);

/// Toggles the nth note in the scale. Automatically updates num_notes and
/// offsets.
void scale_toggle_note(Scale* s, u8 note);

#endif
