
#ifndef SEQUENCE_H
#define SEQUENCE_H

#include "app.h"
#include "layout.h"

#define zoom_to_sequence_x(sr)        (1 << (MAX_ZOOM - (sr)->zoom))

#define grid_to_sequence_x(s, gx)     (((gx) + (s)->x)                  \
                                       * (1 << (MAX_ZOOM - (s)->zoom)))

#define grid_to_sequence_y(s, gy)     (lp_scale.offsets[                \
                                           ((gy) + (s)->y)              \
                                           % lp_scale.num_notes         \
                                           ]                            \
                                       + (s)->layout.root_note          \
                                       + NUM_NOTES * (                  \
                                           ((gy) + (s)->y)              \
                                           / lp_scale.num_notes         \
                                           ))
typedef enum
{
    NTE_ON = 1 << 0,
    NTE_SLIDE = 1 << 1,
    NTE_SKIP = 1 << 2
} NoteFlags;

typedef enum
{
    SEQ_PLAYING        = 1 << 0,  // Sequence is currently playing
    SEQ_MUTED          = 1 << 1,  // Sequence should not make sound
    SEQ_SOLOED         = 1 << 2,  // Other sequences should not make sound
    SEQ_ARMED          = 1 << 3,  // Sequence will record played notes
    SEQ_REVERSED       = 1 << 4,  // Playhead advances backwards
    SEQ_QUEUED         = 1 << 5,  // Sequence should start playing next step
    SEQ_BEAT_QUEUED    = 1 << 6,  // Only start if on beat (combined with ^^)
    SEQ_ACTIVE         = 1 << 7,  // The currently selected sequence
    SEQ_LINKED         = 1 << 8,  // Playhead is controlled by other sequence
    SEQ_LINKED_TO      = 1 << 9,  // This sequence has other ones linked to it
    SEQ_RECORD_CONTROL = 1 << 10, // Should record aftertouch values
    SEQ_DID_RECORD_AHEAD = 1 << 11, // Was the last note quantized forwards?
    SEQ_DRUM_MULTICHANNEL = 1 << 12 // Send each note on its own channel
} SequenceFlags;

typedef struct
{
    s8 note_number;
    s8 velocity;
    u8 flags;
} Note;

typedef Note NoteBank[GRID_SIZE * SEQUENCE_LENGTH];

typedef struct Sequence_
{
    u16 flags;
    Layout layout;
    u8 channel;

    u8 control_code;
    u8 control_div;
    s8 control_sgn;
    s8 control_offset;

    u8 playhead;
    s8 jump_step;
    u8 clock_div;

    u8 zoom;
    u8 x;
    u8 y;

    Note* notes;
} Sequence;

void note_init(Note* n);

void sequence_init(Sequence* s, u8 channel, Note* notes);

Note* sequence_get_note(Sequence* s, u8 playhead);

u8 sequence_get_channel(Sequence* s, u8 note_number);

void sequence_become_active(Sequence* s);

void sequence_become_inactive(Sequence* s);

void sequence_kill_current_note(Sequence* s);

void sequence_play_current_note(Sequence* s);

void sequence_clear_note(Sequence* s, u8 step);

void sequence_clear_notes(Sequence* s);

void sequence_transpose(Sequence* s, s8 amt);

void sequence_set_skip(Sequence* s, u8 step, u8 skip);

void sequence_toggle_linked_to(Sequence* s);

void sequence_toggle_linked(Sequence* s);

Sequence* sequence_get_supersequence(Sequence* s);

void sequence_queue(Sequence* s, u8 is_beat);

void sequence_queue_at(Sequence* s, u8 step, u8 is_beat);

void sequence_jump_to(Sequence* s, u8 step);

void sequence_queue_or_jump(Sequence* s, u8 step, u8 is_beat);

void sequence_stop(Sequence* s);

void sequence_reverse(Sequence* s);

void sequence_handle_record(Sequence* s, u8 press);

u8 sequence_handle_press(Sequence* s, u8 index, u8 value);

u8 sequence_handle_aftertouch(Sequence* s, u8 index, u8 value);

void sequence_step(Sequence* s, u8 audible, u8 is_beat);

void sequence_off_step(Sequence* s);

#endif
