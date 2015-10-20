
#ifndef SESSION_H
#define SESSION_H

#include "sequencer.h"

void session_draw(Sequencer* sr);

u8 session_handle_press(Sequencer* sr, u8 index, u8 value);

#endif
