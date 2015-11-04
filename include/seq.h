
#ifndef SEQ_H
#define SEQ_H


#include "app.h"
#include "buttons.h"
#include "checkbox.h"
#include "colors.h"
#include "control_bank.h"
#include "data.h"
#include "grid.h"
#include "interface.h"
#include "keyboard.h"
#include "layout.h"
#include "mod_wheel.h"
#include "number.h"
#include "scale.h"
#include "sequence.h"
#include "sequencer.h"
#include "session.h"
#include "slider.h"
#include "util.h"
#include "voices.h"


/*******************************************************************************
 * App functionality
 ******************************************************************************/

u8 tap_tempo_handle_press(u8 index, u8 value);

/// All the handlers for all the modes.

/// become_active/become_inactive are called when the mode changes using the
/// mode buttons, or when switching between setup mode.

/// *_draw functions are called when a mode becomes active, when a pad or button
/// is pressed, and for some modes, when the sequencer ticks forward.

/// *_handle_press functions are called upon any surface event, and follow
/// the convention of returning 1 upon successfully handling the event, and 0
/// if it is not handled.

void session_mode_become_active();
void session_mode_become_inactive();
void session_setup_become_active();
void session_setup_become_inactive();
void session_mode_draw();
void session_setup_draw();
u8 session_mode_handle_press(u8 index, u8 value);
u8 session_setup_handle_press(u8 index, u8 value);

void sequencer_mode_become_active();
void sequencer_mode_become_inactive();
void sequencer_setup_become_active();
void sequencer_setup_become_inactive();
void sequencer_mode_draw();
void sequencer_setup_draw();
u8 sequencer_mode_handle_press(u8 index, u8 value);
u8 sequencer_setup_handle_press(u8 index, u8 value);

void notes_mode_become_active();
void notes_mode_become_inactive();
void notes_setup_become_active();
void notes_setup_become_inactive();
void notes_mode_draw();
void notes_setup_draw();
u8 notes_mode_handle_press(u8 index, u8 value);
u8 notes_setup_handle_press(u8 index, u8 value);

void user_mode_become_active();
void user_mode_become_inactive();
void user_setup_become_active();
void user_setup_become_inactive();
void user_mode_draw();
void user_setup_draw();
u8 user_mode_handle_press(u8 index, u8 value);
u8 user_setup_handle_press(u8 index, u8 value);

/*******************************************************************************
 * State management
 ******************************************************************************/

/// Switch the mode or switch between normal and setup modes.
void set_state(LpState st, u8 setup);

#endif
