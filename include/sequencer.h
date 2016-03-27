
#ifndef SEQUENCER_H
#define SEQUENCER_H

#include "app.h"
#include "buttons.h"
#include "checkbox.h"
#include "colors.h"
#include "interface.h"
#include "keyboard.h"
#include "layout.h"
#include "number.h"
#include "scale.h"
#include "sequence.h"
#include "slider.h"
#include "voices.h"
#include "util.h"

#define SQR_COPY_SWAP    (0x80)
#define SQR_COPY_MASK    (0x7F)

/// The step counter is used to determine when to step clock divided sequences,
/// and what note to play in arpeggios. Since clock divisions and number of
/// notes in arpeggios may be anywhere from 1 to 8, the step counter wraps at
/// the least common multiple of 2-8, so that all values can loop evenly.
#define STEP_COUNTER_WRAP (840)

typedef Sequence Sequences[GRID_SIZE];

typedef struct
{
    // Time handling
    uint8_t step_millis; // Baseline for how long a single step takes.
    uint8_t clock_millis; // 1/6th of step_millis. Used for midi clock.
    int8_t swing_millis; // How many milliseconds of extra delay for swing notes.
    uint16_t swung_step_millis; // step_millis +/- swing_millis. Used to avoid repeatedly checking whether sequence is on a swing note.
    uint16_t step_timer; // Number of milliseconds since last step.
    uint16_t step_counter; // Counts passing steps. Wraps at 8. Used for clock division.
    uint8_t clock_timer; // Number of milliseconds since last midi clock message.

    // State
    uint8_t master_sequence; // Index of earliest playing sequence. Used as clock reference.
    uint8_t active_sequence; // Index of sequence used in notes mode and sequencer mode.
    uint8_t soloed_sequences; // Number of soloed sequences.
    uint8_t copied_sequence; // Index of sequence copied in session mode. Use values 8-15 to access the storage bank.

    // Data
    Sequences sequences;
} Sequencer;

/// Initializes sequencer and all held sequences.
void sequencer_init(Sequencer* sr);

/// Sets the tempo in milliseconds per step.
void sequencer_set_tempo_millis(Sequencer* sr, uint8_t millis);

/// Sets the tempo in beats per minute.
void sequencer_set_tempo(Sequencer* sr, uint8_t bpm);

/// Sets the swing in absolute milliseconds.
void sequencer_set_swing_millis(Sequencer* sr, int8_t swing_millis);

/// Sets the swing in 6ths of a step (positive or negative).
void sequencer_set_swing(Sequencer* sr, int8_t swing);

/// Sets the active sequence.
void sequencer_set_active(Sequencer* sr, uint8_t i);

/// Kills all playing notes in all playing sequences.
void sequencer_kill_current_notes(Sequencer* sr);

/// Sets the index of the sequence to copy from values 0-7 are the 8 sequences,
/// and values 8-15 are extra storage slots. Swap indicates the two slots
/// should have their data swapped, rather than one be overwritten.
void sequencer_copy(Sequencer* sr, uint8_t i, uint8_t swap);

/// Pastes from the copied sequence/storage bank, into the given one.
void sequencer_paste(Sequencer* sr, uint8_t i);

/// Calls copy if there is not already an index in the clipboard, otherwise
/// pastes.
void sequencer_copy_or_paste(Sequencer* sr, uint8_t i);

/// Returns the active sequence.
Sequence* sequencer_get_active(Sequencer* sr);

/// Returns the earliest playing sequence.
Sequence* sequence_get_master(Sequencer* sr);

/// Returns the layout of the active sequence.
Layout* sequencer_get_layout(Sequencer* sr);

/// Draws the play buttons on the right of the pads, and the related modifier
/// buttons at the bottom of the pads.
void sequencer_play_draw(Sequencer* sr);

/// Draws the tempo indicators on the click button, or the top row of buttons.
void sequencer_blink_draw(Sequencer* sr, uint8_t blink, uint8_t position);

/// Clears out the tempo indicators when they are turned off.
void sequencer_blink_clear(Sequencer* sr, uint8_t blink, uint8_t position);

/// Handle presses on the play buttons and related modifiers.
uint8_t sequencer_handle_play(Sequencer* sr, uint8_t index, uint8_t value);

/// Ticks the sequence forward 1 millisecond, sending midi clock and stepping
/// the sequences as necessary. clock_tick indicates that this tick was
/// initiated by an external clock.
void sequencer_tick(Sequencer* sr, uint8_t clock_tick);

#endif
