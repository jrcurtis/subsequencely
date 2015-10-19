
#ifndef SEQUENCE_H
#define SEQUENCE_H

#include "app.h"
#include "layout.h"

#define zoom_to_sequence_x(sr)        (1 << (MAX_ZOOM - (sr)->zoom))

#define grid_to_sequence_x(sr, gx)    (((gx) + (s)->x)                  \
                                       * (1 << (MAX_ZOOM - (s)->zoom)))

#define grid_to_sequence_y(s, gy)     ((s)->layout.scale->offsets[      \
                                           ((gy) + (s)->y)              \
                                           % (s)->layout.scale->num_notes \
                                           ]                            \
                                       + (s)->layout.root_note          \
                                       + NUM_NOTES * (                  \
                                           ((gy) + (s)->y)              \
                                           / (s)->layout.scale->num_notes \
                                           ))
typedef enum
{
    NTE_ON = 0x01,
    NTE_SLIDE = 0x02
} NoteFlags;

typedef enum
{
    SEQ_PLAYING = 0x01,
    SEQ_MUTED = 0x02,
    SEQ_SOLOED = 0x04,
    SEQ_ARMED = 0x08,
    SEQ_QUEUED = 0x10,
    SEQ_BEAT_QUEUED = 0x20,
    SEQ_ACTIVE = 0x40,
    SEQ_LINKED = 0x80
} SequenceFlags;

typedef struct
{
    s8 note_number;
    s8 velocity;
    u8 flags;
} Note;

typedef struct
{
    Layout layout;
    u8 channel;
    u8 flags;

    u8 playhead;
    u8 zoom;
    u8 x;
    u8 y;

    Note notes[SEQUENCE_LENGTH];
} Sequence;

void sequence_init(Sequence* s, u8 channel);

void sequence_become_active(Sequence* s);

void sequence_become_inactive(Sequence* s);

void sequence_kill_current_note(Sequence* s);

void sequence_play_current_note(Sequence* s);

void sequence_queue(Sequence* s);

void sequence_stop(Sequence* s, Layout* l);

void sequence_handle_record(Sequence* s, u8 press);

void sequence_step(Sequence* s, u8 audible);

void sequence_off_step(Sequence* s);

#endif
