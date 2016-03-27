
#ifndef NOTE_H
#define NOTE_H

#include "app.h"

typedef enum
{
    NTE_ON = 1 << 0, // Set on when a note on is sent, and should always be turned off again!
    NTE_SLIDE = 1 << 1, // Is this note tied to the previous, or individually articulated?
    NTE_SKIP = 1 << 2 // Skip the playhead right over this note, taking 0 time.
} NoteFlags;

/// The note storage structure for sequences. This used to store velocity and
/// aftertouch separately, but had to be changed to save ram, so now velocity
/// is recorded when a note is first pressed, but aftertouch is recorded into
/// the velocity field while the note is held.
typedef struct
{
    int8_t note_number;
    int8_t velocity;
    uint8_t flags;
} Note;

struct Sequence_;

/// Initializes note to empty/off.
void note_init(Note* n);

void note_play(Note* n, uint8_t channel, uint8_t full_velocity);

void note_control(Note* n, struct Sequence_* s);

void note_kill(Note* n, uint8_t channel);

#endif
