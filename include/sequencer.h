
#ifndef SEQUENCER_H
#define SEQUENCER_H

#include "app.h"
#include "buttons.h"
#include "colors.h"
#include "layout.h"
#include "util.h"

typedef enum
{
    PLAYING = 0x01,
    MUTED = 0x02,
    SOLOED = 0x04,
    ARMED = 0x08,
    QUEUED = 0x10
} SequenceFlags;

typedef enum
{
    ARM_HELD = 0x01,
    SELECT_HELD = 0x02,
    MUTE_HELD = 0x04,
    SOLO_HELD = 0x08,
    DIRTY = 0x10
} SequencerFlags;

typedef struct
{
    s8 note_number;
    s8 velocity;
} Note;

typedef struct
{
    Note notes[SEQUENCE_LENGTH];
    u8 playhead;
    u8 flags;
} Sequence;

typedef Sequence Sequences[GRID_SIZE];

typedef struct
{
    Sequences* sequences;
    Layout* layout;
    u16 tempo;
    u16 timer;
    u8 zoom;
    u8 active_sequence;
    u8 soloed_tracks;
    u8 x;
    u8 y;
    u8 flags;
} Sequencer;

void sequencer_init(Sequencer* sr, Sequences* ss, Layout* l);
void sequencer_set_octave(Sequencer* sr, u8 octave);
void sequencer_play_draw(Sequencer* sr);
void sequencer_grid_draw(Sequencer* sr);
void sequencer_grid_handle_press(Sequencer* sr, u8 index, u8 value);
void sequencer_tick(Sequencer* sr);

#endif
