
#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "app.h"
#include "util.h"
#include "colors.h"
#include "layout.h"

#define NUM_KEYBOARD_PADS  (2 * ROW_SIZE - ROW_GAP)
#define FIRST_KEYBOARD_PAD (FIRST_PAD)
#define LAST_KEYBOARD_PAD  (FIRST_KEYBOARD_PAD + NUM_KEYBOARD_PADS - 1)

extern const u8 diatonic_notes[NUM_NOTES];

/// Represents an onscreen keyboard that can be transposed up or down, and have
/// keys toggled on or off. Used to select which notes to enable in a scale.
typedef struct
{
    /// The layout to update when changes are made to the keyboard. The keyboard
    /// is always drawn starting from the layout's root note.
    Layout* layout;

    /// Cache of which pad index corresponds to which key on the keyboard
    /// (0-11 <=> C-B). -1 indicates no key at that index.
    s8 index_to_note[NUM_KEYBOARD_PADS];
} Keyboard;

/// Initializes the keyboard data.
void keyboard_init(Keyboard* k, Layout* l);

/// Toggles a note on or off and updates the layout and scale to match.
u8 keyboard_handle_press(Keyboard* k, u8 index, u8 value);

/// Draws the keyboard to the grid.
void keyboard_draw(Keyboard* k);

/// When a change is made to the layout or scale, this function updates the
/// keyboard to reflect the current state.
void keyboard_update_indices(Keyboard* k);

#endif
