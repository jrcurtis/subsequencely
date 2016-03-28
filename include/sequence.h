
#ifndef SEQUENCE_H
#define SEQUENCE_H

#include "app.h"
#include "layout.h"
#include "mod_wheel.h"
#include "note.h"

#define zoom_to_sequence_x(sr)        (1 << (MAX_ZOOM - (sr)->zoom))

/// Given a sequence and a grid x coordinate, returns the sequence step it
/// represents, based the the sequence's zoom and translation.
#define grid_to_sequence_x(s, gx)     (((gx) + (s)->x)                  \
                                       * (1 << (MAX_ZOOM - (s)->zoom)))

/// Given a sequence, and a grid y coordinate, returns the represented note
/// number based on the sequence's zoom and translation.
#define grid_to_sequence_y(s, gy)     (lp_scale.offsets[                \
                                           ((gy) + (s)->y)              \
                                           % lp_scale.num_notes         \
                                           ]                            \
                                       + (s)->layout.root_note          \
                                       + NUM_NOTES * (                  \
                                           ((gy) + (s)->y)              \
                                           / lp_scale.num_notes         \
                                           ))

#define mod_to_cc(m) ((mod_wheel_get_value((m)) >> (MW_BYTE_BITS - 1)) - 128)

typedef enum
{
    SEQ_PLAYING        = 1 << 0,  // Sequence is currently playing
    SEQ_MUTED          = 1 << 1,  // Sequence should not make sound
    SEQ_SOLOED         = 1 << 2,  // Other sequences should not make sound
    SEQ_REVERSED       = 1 << 3,  // Playhead advances backwards
    SEQ_QUEUED_MASK    = 0x3 << 4,// Queueing mode the sequence is in
    SEQ_ACTIVE         = 1 << 6,  // The currently selected sequence
    SEQ_LINKED         = 1 << 7,  // Playhead is controlled by other sequence
    SEQ_LINKED_TO      = 1 << 8,  // This sequence has other ones linked to it
    SEQ_RECORD_CONTROL = 1 << 9,  // Should record aftertouch values
    SEQ_DID_RECORD_AHEAD = 1 << 10, // Was the last note quantized forwards?
    SEQ_DRUM_MULTICHANNEL = 1 << 11, // Send each note on its own channel
    SEQ_FULL_VELOCITY  = 1 << 12, // Always send notes at full velocity
    SEQ_MOD_WHEEL      = 1 << 13, // Show the mod wheel in notes mode
    SEQ_MOD_CC         = 1 << 14  // Send CC from mod wheel instead of aftertouch
} SequenceFlags;

#define SEQ_QUEUED_OFFSET (4)
#define SEQ_QUEUED_STEP   (1)
#define SEQ_QUEUED_BEAT   (2)
#define SEQ_QUEUED_START  (3)

#define seq_get_queued(f)    (get_masked(                                      \
                                  (f), SEQ_QUEUED_MASK, SEQ_QUEUED_OFFSET))
#define seq_set_queued(f, x) (set_masked(                                      \
                                  (f), SEQ_QUEUED_MASK, SEQ_QUEUED_OFFSET, (x)))

typedef struct Sequence_
{
    uint16_t flags;
    Layout layout; // The layout to use in notes mode when this sequence is active.
    uint8_t channel; // The channel to send midi on (or the base channel, when in multichannel mode)

    uint8_t control_code; // The midi CC that aftertouch is sent on.
    uint8_t control_div; // The sensitivity divider for aftertouch values (see cc_div in util.h)
    int8_t control_sgn; // Whether CC is positive or negative.
    int8_t control_offset; // Value added to all CCs sent

    uint8_t playhead; // The position of the playhead/the currently playing note.
    int8_t jump_step; // Set to >= 0 to make the playhead jump there on the next step.
    uint8_t clock_div; // The amount to divide the global tempo clock.

    uint8_t zoom; // Zoom level in sequencer mode.
    uint8_t x; // X location of view in sequencer mode.
    uint8_t y; // Y location of view in sequencer mode.

    Note* notes; // Pointer to the beginning of this sequence's notes in the NoteBank.
} Sequence;

/// Initializes sequence to empty/off.
void sequence_init(Sequence* s, uint8_t channel, Note* notes);

/// Gets the note at the playhead.
Note* sequence_get_note(Sequence* s, uint8_t playhead);

/// Gets the channel. Has special logic for multichannel mode.
uint8_t sequence_get_channel(Sequence* s, uint8_t note_number);

/// Handler for when the active sequence changes.
void sequence_become_active(Sequence* s);

/// Handler for when the active sequence changes (but in the other way.)
void sequence_become_inactive(Sequence* s);

/// Prepares mod wheel by clearing out the notes in the pad_notes buffer at the
/// location of the mod wheel to avoid them being lit up while playing.
void sequence_prepare_mod_wheel(Sequence* s);

/// Kills the given note if necessary and handles unlighting the pad.
void sequence_kill_note(Sequence* s, Note* n);

/// Send a note off/unlight lit pads/whatever else, if there is a note currently
/// playing.
void sequence_kill_current_note(Sequence* s);

/// Play the note at the playhead and light up the pads, if necessary.
void sequence_play_current_note(Sequence* s);

/// Clear note data at given step.
void sequence_clear_note(Sequence* s, uint8_t step);

/// Clear all note data in the sequence. Includes linked sequences.
void sequence_clear_notes(Sequence* s);

/// Kills held down notes, or just the sustained notes. Used when the pads are
/// transposed so that the note off when the pads are released aren't different
/// from the note on.
void sequence_kill_voices(Sequence* s, uint8_t sustained);

/// Transpose all the notes in the sequence by the given amount.
void sequence_transpose(Sequence* s, int8_t amt);

/// Set a note to be skipped over.
void sequence_set_skip(Sequence* s, uint8_t step, uint8_t skip);

/// Called when this sequence becomes the supersequence to the next sequence.
void sequence_toggle_linked_to(Sequence* s);

/// Called when this sequence become the subsequence of the previous sequence.
void sequence_toggle_linked(Sequence* s);

/// Gets the first sequence preceding this one that does not have the SEQ_LINKED
/// flag set (might return the passed in sequence).
Sequence* sequence_get_supersequence(Sequence* s);

/// Set the sequence to start playing at a time based on queue_mode.
/// queue_mode is one of SEQ_QUEUED_STEP/BEAT/START and determines when the
/// sequence should become unqueued.
void sequence_queue(Sequence* s, uint8_t queue_mode);

/// Sets the sequence to start playing from the given step.
void sequence_queue_at(Sequence* s, uint8_t step, uint8_t queue_mode);

/// Immediately jumps the playhead to the given step, assuming the sequence is
/// already playing.
void sequence_jump_to(Sequence* s, uint8_t step);

/// Chooses between sequence_queue_at or sequence_jump_to based on whether
/// the sequence is already playing.
/// queue_mode does not affect sequence_jump_to.
void sequence_queue_or_jump(Sequence* s, uint8_t step, uint8_t queue_mode);

/// Immediately stops the sequence and kills any playing note.
void sequence_stop(Sequence* s);

/// Reverses the playing direction of the sequence.
void sequence_reverse(Sequence* s);

/// Handles the logic of writing new notes/aftertouch values in the sequence.
void sequence_handle_record(Sequence* s, uint8_t press);

/// Draws stuff in notes mode, although not responsible for drawing the layout.
void sequence_draw(Sequence* s);

/// Handles when notes are played in notes mode.
uint8_t sequence_handle_press(Sequence* s, uint8_t index, uint8_t value);

/// Handles aftertouch in notes mode.
uint8_t sequence_handle_aftertouch(Sequence* s, uint8_t index, int8_t value);

/// Logic to step the sequence forward, handling note on/off for the appropriate
/// steps.
void sequence_step(Sequence* s, uint8_t audible, uint8_t queue_flags);

/// Called in between calls to sequence_step. If there is currently a note
/// playing, and the upcoming note in the sequence is NOT a slide note, the
/// current note is killed early so it can have time to turn off before the
/// next note.
void sequence_off_step(Sequence* s);

#endif
