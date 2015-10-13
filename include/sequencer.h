
#ifndef SEQUENCER_H
#define SEQUENCER_H

#include "app.h"
#include "buttons.h"
#include "colors.h"
#include "layout.h"
#include "util.h"

typedef enum
{
    SEQ_PLAYING = 0x01,
    SEQ_MUTED = 0x02,
    SEQ_SOLOED = 0x04,
    SEQ_ARMED = 0x08,
    SEQ_QUEUED = 0x10,
    SEQ_BEAT_QUEUED = 0x20,
    SEQ_ACTIVE = 0x40
} SequenceFlags;

typedef enum
{
    SQR_ARM_HELD = 0x01,
    SQR_SELECT_HELD = 0x02,
    SQR_MUTE_HELD = 0x04,
    SQR_SOLO_HELD = 0x08,
    SQR_DIRTY = 0x10
} SequencerFlags;

typedef struct
{
    s8 note_number;
    s8 velocity;
} Note;

typedef struct
{
    Note notes[SEQUENCE_LENGTH];
    u8 channel;
    u8 playhead;
    u8 flags;
} Sequence;

typedef Sequence Sequences[GRID_SIZE];

typedef struct
{
    Layout* layout;
    u16 tempo;
    u16 timer;
    u8 zoom;
    u8 active_sequence;
    u8 soloed_tracks;
    u8 x;
    u8 y;
    u8 flags;
    Sequences sequences;
} Sequencer;

void sequencer_init(Sequencer* sr, Layout* l);
void sequencer_set_octave(Sequencer* sr, u8 octave);

void sequencer_play_draw(Sequencer* sr);
void sequencer_grid_draw(Sequencer* sr);

u8 sequencer_handle_play(Sequencer* sr, u8 index, u8 value);
u8 sequencer_handle_modifiers(Sequencer* sr, u8 index, u8 value);
u8 sequencer_grid_handle_press(Sequencer* sr, u8 index, u8 value);
u8 sequencer_handle_record(Sequencer* sr);

void sequencer_tick(Sequencer* sr);

#endif
