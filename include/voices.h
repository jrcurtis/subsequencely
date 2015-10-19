
#ifndef VOICES_H
#define VOICES_H

#include "app.h"
#include "util.h"

#define NUM_VOICES          (GRID_SIZE)

typedef struct
{
    s8 note_number;
} Voice;

typedef struct
{
    s8 max_index;
    u8 velocity;
    Voice voices[GRID_SIZE];
} Voices;

void voices_init(Voices* vs);

void voices_add(Voices* vs, u8 note_number, u8 velocity);

void voices_remove(Voices* vs, u8 note_number);

s8 voices_get_newest(Voices* vs);

u8 voices_get_num_active(Voices* vs);

void voices_reset(Voices* vs);

#endif
