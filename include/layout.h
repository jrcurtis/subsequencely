
#ifndef LAYOUT_H
#define LAYOUT_H

#include "app.h"
#include "util.h"
#include "colors.h"
#include "scale.h"

typedef u8 PadNotes[GRID_SIZE][GRID_SIZE];

/// Represents a layout of a scale on a grid. Determines which note and octave
/// to start from, and the distance in scale steps between rows of the grid.
typedef struct
{
    /// The scale this layout is based on. Only notes in the scale are included
    /// in the layout.
    Scale* scale;

    /// The root note (0-11 <=> C-B) of the layout.
    s8 root_note;

    /// The octave of the root note at the bottom left of the grid. The
    /// lowest note on the grid.
    s8 octave;

    /// How many steps of the scale to jump when moving vertically between
    /// rows. To tune in 4ths for a chromatic scale, this would be set to 5.
    u8 row_offset;

    /// Cache of which midi note numbers correspond to which pad. Used to avoid
    /// calculating every time a pad is pressed.
    PadNotes* pad_notes;
} Layout;

/// Initialize the layout data.
void layout_init(Layout* l, Scale* s, PadNotes* pad_notes);

/// True if the midi note number is the same or an octave of the root note.
u8 layout_is_root_note(Layout* l, u8 note_number);

/// Toggles the note in the scale, and updates the layout.
void layout_toggle_note(Layout* l, u8 note);

/// Draws the layout onto the grid.
void layout_draw(Layout* l);

/// Increases or decreases the root note by a half step.
void layout_transpose(Layout* l, s8 direction);

/// Increases or decreases the octave.
void layout_transpose_octave(Layout* l, s8 direction);

/// Sets the offset between rows and updates the layout.
void layout_set_row_offset(Layout* l, u8 o);

/// Plays a note on the grid.
void layout_play(Layout* l, u8 index, u8 value, u8 midi_channel);

#endif
