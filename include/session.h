
#ifndef SESSION_H
#define SESSION_H

#include "sequencer.h"

typedef enum
{
    SES_CLICK_HELD = 0x01,
} SessionFlags;

void session_draw(Sequencer* sr);

#endif
