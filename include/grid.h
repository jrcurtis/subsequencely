
#ifndef GRID_H
#define GRID_H

#include "sequencer.h"

/// Draws the active sequence's notes layed out on a grid.
void grid_draw(Sequencer* sr);

/// Handles moving around using the arrow buttons in sequencer mode.
u8 grid_handle_translate(Sequencer* sr, u8 index, u8 value);

/// Zooms in and out.
u8 grid_handle_zoom(Sequencer* sr, u8 index, u8 value);

/// Handles all the presses.
u8 grid_handle_press(Sequencer* sr, u8 index, u8 value);

#endif
