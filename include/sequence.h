
#ifndef SEQUENCE_H
#define SEQUENCE_H

#include "app.h"
#include "layout.h"
#include "mod_wheel.h"

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
    NTE_ON = 1 << 0, // Set on when a note on is sent, and should always be turned off again!
    NTE_SLIDE = 1 << 1, // Is this note tied to the previous, or individually articulated?
    NTE_SKIP = 1 << 2 // Skip the playhead right over this note, taking 0 time.
} NoteFlags;

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

/// The note storage structure for sequences. This used to store velocity and
/// aftertouch separately, but had to be changed to save ram, so now velocity
/// is recorded when a note is first pressed, but aftertouch is recorded into
/// the velocity field while the note is held.
typedef struct
{
    s8 note_number;
    s8 velocity;
    u8 flags;
} Note;

/// The number of notes needed for all 8 sequences. Two of these are used: one
/// for the live sequences data, and one for extra storage.
typedef Note NoteBank[GRID_SIZE * SEQUENCE_LENGTH];

typedef struct Sequence_
{
    u16 flags;
    Layout layout; // The layout to use in notes mode when this sequence is active.
    u8 channel; // The channel to send midi on (or the base channel, when in multichannel mode)

    u8 control_code; // The midi CC that aftertouch is sent on.
    u8 control_div; // The sensitivity divider for aftertouch values (see cc_div in util.h)
    s8 control_sgn; // Whether CC is positive or negative.
    s8 control_offset; // Value added to all CCs sent

    u8 playhead; // The position of the playhead/the currently playing note.
    s8 jump_step; // Set to >= 0 to make the playhead jump there on the next step.
    u8 clock_div; // The amount to divide the global tempo clock.

    u8 zoom; // Zoom level in sequencer mode.
    u8 x; // X location of view in sequencer mode.
    u8 y; // Y location of view in sequencer mode.

    Note* notes; // Pointer to the beginning of this sequence's notes in the NoteBank.
} Sequence;

/// Initializes note to empty/off.
void note_init(Note* n);

/// Initializes sequence to empty/off.
void sequence_init(Sequence* s, u8 channel, Note* notes);

/// Gets the note at the playhead.
Note* sequence_get_note(Sequence* s, u8 playhead);

/// Gets the channel. Has special logic for multichannel mode.
u8 sequence_get_channel(Sequence* s, u8 note_number);

/// Handler for when the active sequence changes.
void sequence_become_active(Sequence* s);

/// Handler for when the active sequence changes (but in the other way.)
void sequence_become_inactive(Sequence* s);

/// Prepares mod wheel by clearing out the notes in the pad_notes buffer at the
/// location of the mod wheel to avoid them being lit up while playing.
void sequence_prepare_mod_wheel(Sequence* s);

/// Send a note off/unlight lit pads/whatever else, if there is a note currently
/// playing.
void sequence_kill_current_note(Sequence* s);

/// Play the note at the playhead and light up the pads, if necessary.
void sequence_play_current_note(Sequence* s);

/// Clear note data at given step.
void sequence_clear_note(Sequence* s, u8 step);

/// Clear all note data in the sequence. Includes linked sequences.
void sequence_clear_notes(Sequence* s);

/// Kills all held down notes. Used when the pads are transposed so that the
/// note off when the pads are released aren't different from the note on.
void sequence_kill_voices(Sequence* s);

/// Transpose all the notes in the sequence by the given amount.
void sequence_transpose(Sequence* s, s8 amt);

/// Set a note to be skipped over.
void sequence_set_skip(Sequence* s, u8 step, u8 skip);

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
void sequence_queue(Sequence* s, u8 queue_mode);

/// Sets the sequence to start playing from the given step.
void sequence_queue_at(Sequence* s, u8 step, u8 queue_mode);

/// Immediately jumps the playhead to the given step, assuming the sequence is
/// already playing.
void sequence_jump_to(Sequence* s, u8 step);

/// Chooses between sequence_queue_at or sequence_jump_to based on whether
/// the sequence is already playing.
/// queue_mode does not affect sequence_jump_to.
void sequence_queue_or_jump(Sequence* s, u8 step, u8 queue_mode);

/// Immediately stops the sequence and kills any playing note.
void sequence_stop(Sequence* s);

/// Reverses the playing direction of the sequence.
void sequence_reverse(Sequence* s);

/// Handles the logic of writing new notes/aftertouch values in the sequence.
void sequence_handle_record(Sequence* s, u8 press);

/// Draws stuff in notes mode, although not responsible for drawing the layout.
void sequence_draw(Sequence* s);

/// Handles when notes are played in notes mode.
u8 sequence_handle_press(Sequence* s, u8 index, u8 value);

/// Handles aftertouch in notes mode.
u8 sequence_handle_aftertouch(Sequence* s, u8 index, s8 value);

/// Logic to step the sequence forward, handling note on/off for the appropriate
/// steps.
void sequence_step(Sequence* s, u8 audible, u8 queue_flags);

/// Called in between calls to sequence_step. If there is currently a note
/// playing, and the upcoming note in the sequence is NOT a slide note, the
/// current note is killed early so it can have time to turn off before the
/// next note.
void sequence_off_step(Sequence* s);

#endif
