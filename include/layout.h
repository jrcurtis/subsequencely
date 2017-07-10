
#ifndef LAYOUT_H
#define LAYOUT_H

#include "app.h"
#include "buttons.h"
#include "colors.h"
#include "voices.h"
#include "util.h"

#define LAYOUT_DEFAULT_OCTAVE 3

/// The bits where row offset lives. Excludes the drum flag bit, so that
/// the drums state doesn't get trashed when you use the row offset slider.
#define ROW_OFFSET_MASK      (0x7F)

typedef enum
{
    /// Indicates the layout should be 4x4 drum pad style. This flag lives in
    /// the 8th bit of row_offset rather than its own flags variable because
    /// ram, and row_offset is irrelevant in drum mode anyway.
    LYT_DRUMS = 1 << 7
} LayoutFlags;

/// Cache of which midi note numbers correspond to which pad. Used to avoid
/// calculating every time a pad is pressed.
typedef int8_t PadNotes[GRID_SIZE][GRID_SIZE];

/// Cache of which pads are highlighted for note mode
typedef int8_t PadHighlights[GRID_SIZE];

/// Represents a layout of a scale on a grid. Determines which note and octave
/// to start from, and the distance in scale steps between rows of the grid.
typedef struct
{
    /// The root note (0-11 <=> C-B) of the layout.
    int8_t root_note;

    /// The octave of the root note at the bottom left of the grid. The
    /// lowest note on the grid.
    int8_t octave;

    /// How many steps of the scale to jump when moving vertically between
    /// rows. To tune in 4ths for a chromatic scale, this would be set to 5.
    int8_t row_offset;

    /// How many spaces up/down to translate the view
    int8_t offset_horizontal;
    int8_t offset_vertical;
} Layout;

/// Initialize the layout data.
void layout_init(Layout* l);

/// True if the midi note number is the same or an octave of the root note.
uint8_t layout_is_root_note(Layout* l, uint8_t note_number);

uint8_t layout_get_note_number(Layout* l, uint8_t index);

/// Calculate which pads play which notes.
void layout_assign_pads(Layout* l);
void layout_assign_scale(Layout* l);
void layout_assign_drums(Layout* l);

void layout_become_active(Layout* l);
void layout_become_inactive(Layout* l);

/// Switch layout to drum pad mode.
void layout_set_drums(Layout* l);

/// Toggles the note in the scale, and updates the layout.
void layout_toggle_note(Layout* l, uint8_t note);
/// Toggles the note highlight
void layout_toggle_highlight(Layout* l, uint8_t note_highlight);

/// Draws the layout onto the grid.
void layout_draw(Layout* l);
void layout_draw_scale(Layout* l);
void layout_draw_drums(Layout* l);
void layout_draw_transpose_octave_buttons(Layout* l);

/// Increases or decreases the root note by a half step.
void layout_transpose(Layout* l, int8_t direction);

/// Increases or decreases the octave.
void layout_transpose_octave(Layout* l, int8_t direction);

/// Sets the offset between rows and updates the layout.
void layout_set_row_offset(Layout* l, uint8_t o);

/// Lights all pads currently on screen that correspond to the given note.
void layout_light_note(Layout* l, uint8_t note_number, uint8_t on);
void layout_light_scale(Layout* l, uint8_t note_number, uint8_t on);
void layout_light_drums(Layout* l, uint8_t note_number, uint8_t on);

/// Transposes the notes of the layout by half steps or octaves.
uint8_t layout_handle_transpose(Layout* l, uint8_t index, uint8_t value);

#endif
