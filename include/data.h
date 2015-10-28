
#ifndef DATA_H
#define DATA_H

#include "app.h"
#include "control_bank.h"
#include "keyboard.h"
#include "scale.h"
#include "sequence.h"
#include "sequencer.h"
#include "slider.h"

/// Enum representing the different program modes.
typedef enum
{
    LP_SESSION_MODE,
    LP_NOTES_MODE,
    LP_SEQUENCER_MODE,
    LP_USER_MODE,
    LP_NUM_MODES
} LpState;

/// Global program flags.
typedef enum
{
    LP_IS_SETUP       = 1 << 0,
    LP_PORT_CHECKBOX  = 1 << 1,
    LP_TEMPO_BLINK    = 1 << 2,
    LP_POSITION_BLINK = 1 << 3,
    LP_SQR_DIRTY      = 1 << 4,
    LP_SEND_CLOCK     = 1 << 5,
    LP_ARMED          = 1 << 6
} LpFlags;

/*******************************************************************************
 * Global data
 ******************************************************************************/

// Global settings
extern u8 lp_midi_port; // Which port to send on.

// Program state
extern LpState lp_state; // What mode the program is in
extern u8 lp_flags; // Global flags

extern u16 lp_tap_tempo_timer; // Timer used to track tap tempo presses
extern u16 lp_tap_tempo_sum;
extern u8 lp_tap_tempo_counter;

// Data
extern Scale lp_scale;
extern Voices lp_voices;
extern PadNotes lp_pad_notes;
extern NoteBank lp_note_bank;
extern NoteBank lp_note_storage;
extern Sequencer lp_sequencer;

// Setup UI elements
extern Slider lp_tempo_slider;
extern Slider lp_swing_slider;

extern Keyboard lp_keyboard;
extern Slider lp_row_offset_slider;

extern Slider lp_control_sens_slider;
extern Slider lp_control_offset_slider;

extern ControlBank lp_user_control_bank;

#endif
