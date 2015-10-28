
#ifndef SESSION_H
#define SESSION_H

#include "sequencer.h"

/// Draws the session mode view of the sequences.
void session_draw(Sequencer* sr);

/// Handles presses on the pads and the modifiers on the left side of the pads
/// in session mode.
u8 session_handle_press(Sequencer* sr, u8 index, u8 value);

#endif
