
#ifndef GRID_H
#define GRID_H

#include "sequencer.h"

void grid_draw(Sequencer* sr);

u8 grid_handle_translate(Sequencer* sr, u8 index, u8 value);

u8 grid_handle_zoom(Sequencer* sr, u8 index, u8 value);

u8 grid_handle_press(Sequencer* sr, u8 index, u8 value);

#endif
