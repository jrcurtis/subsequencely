
#ifndef SEQUENCER_H
#define SEQUENCER_H

#include "app.h"
#include "buttons.h"
#include "colors.h"
#include "keyboard.h"
#include "layout.h"
#include "scale.h"
#include "sequence.h"
#include "voices.h"
#include "util.h"

typedef enum
{
    SQR_ARM_HELD = 0x01,
    SQR_SELECT_HELD = 0x02,
    SQR_MUTE_HELD = 0x04,
    SQR_SOLO_HELD = 0x08,
    SQR_DIRTY = 0x10
} SequencerFlags;

typedef Sequence Sequences[GRID_SIZE];

typedef struct
{
    u16 tempo;
    u16 timer;

    u8 active_sequence;
    u8 soloed_tracks;
    u8 flags;

    Scale scale;
    Keyboard keyboard;
    Voices voices;
    PadNotes pad_notes;
    Sequences sequences;
} Sequencer;

void sequencer_init(Sequencer* sr);
void sequencer_set_octave(Sequencer* sr, u8 octave);
void sequencer_set_active(Sequencer* sr, u8 i);
Sequence* sequencer_get_active(Sequencer* sr);
Layout* sequencer_get_layout(Sequencer* sr);

void sequencer_play_draw(Sequencer* sr);
void sequencer_grid_draw(Sequencer* sr);

u8 sequencer_handle_play(Sequencer* sr, u8 index, u8 value);
u8 sequencer_handle_modifiers(Sequencer* sr, u8 index, u8 value);
u8 sequencer_handle_record(Sequencer* sr);

void sequencer_tick(Sequencer* sr);

#endif
