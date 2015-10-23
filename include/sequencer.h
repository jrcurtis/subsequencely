
#ifndef SEQUENCER_H
#define SEQUENCER_H

#include "app.h"
#include "buttons.h"
#include "checkbox.h"
#include "colors.h"
#include "keyboard.h"
#include "layout.h"
#include "number.h"
#include "scale.h"
#include "sequence.h"
#include "slider.h"
#include "voices.h"
#include "util.h"

typedef enum
{
    SQR_DIRTY = 0x01
} SequencerFlags;

typedef Sequence Sequences[GRID_SIZE];
typedef Note NoteBank[GRID_SIZE * SEQUENCE_LENGTH];

typedef struct
{
    // Time handling
    u16 step_millis;
    u8 clock_millis;
    s8 swing_millis;
    u16 timer;
    u8 master_sequence;

    // State
    u8 active_sequence;
    u8 soloed_tracks;
    u8 flags;

    // UI widgets
    Keyboard* keyboard;
    Slider* row_offset_slider;
    Checkbox* control_checkbox;
    Number* control_number;
    Slider* control_sens_slider;
    Slider* control_offset_slider;

    // Data
    Scale scale;
    Voices voices;
    PadNotes pad_notes;
    NoteBank note_bank;
    Sequences sequences;
} Sequencer;

void sequencer_init(Sequencer* sr);
void sequencer_set_tempo_millis(Sequencer* sr, u16 millis);
void sequencer_set_tempo(Sequencer* sr, u16 bpm);
void sequencer_set_swing(Sequencer* sr, s8 swing);
void sequencer_set_octave(Sequencer* sr, u8 octave);
void sequencer_set_active(Sequencer* sr, u8 i);
Sequence* sequencer_get_active(Sequencer* sr);
Layout* sequencer_get_layout(Sequencer* sr);

void sequencer_play_draw(Sequencer* sr);

u8 sequencer_handle_play(Sequencer* sr, u8 index, u8 value);
u8 sequencer_handle_record(Sequencer* sr);

void sequencer_tick(Sequencer* sr);

#endif
