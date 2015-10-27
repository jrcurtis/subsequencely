
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

typedef Sequence Sequences[GRID_SIZE];

typedef struct
{
    // Time handling
    u8 step_millis;
    u8 clock_millis;
    s8 swing_millis;
    u16 swung_step_millis;
    u16 step_timer;
    u8 step_counter;
    u8 clock_timer;

    // State
    u8 master_sequence;
    u8 active_sequence;
    u8 soloed_sequences;
    s8 copied_sequence;

    // Data
    Sequences sequences;
} Sequencer;

void sequencer_init(Sequencer* sr);

void sequencer_set_tempo_millis(Sequencer* sr, u16 millis);
void sequencer_set_tempo(Sequencer* sr, u16 bpm);
void sequencer_set_swing(Sequencer* sr, s8 swing);
void sequencer_set_octave(Sequencer* sr, u8 octave);
void sequencer_set_active(Sequencer* sr, u8 i);
void sequencer_kill_current_notes(Sequencer* sr);

void sequencer_copy(Sequencer* sr, u8 i);
void sequencer_paste(Sequencer* sr, u8 i);

Sequence* sequencer_get_active(Sequencer* sr);
Sequence* sequence_get_master(Sequencer* sr);
Layout* sequencer_get_layout(Sequencer* sr);

void sequencer_play_draw(Sequencer* sr);
void sequencer_blink_draw(Sequencer* sr, u8 blink, u8 position);
void sequencer_blink_clear(Sequencer* sr, u8 blink, u8 position);

u8 sequencer_handle_play(Sequencer* sr, u8 index, u8 value);

void sequencer_tick(Sequencer* sr);

#endif
