

#include "session.h"

void session_draw(Sequencer* sr)
{
    for (u8 y = 0; y < GRID_SIZE; y++)
    {
        Sequence* s = &sr->sequences[y];

        for (u8 x = 0; x < GRID_SIZE; x++)
        {
            u8 dimness = 4 * (s->playhead != x);
            plot_pad_dim(coord_to_index(x, y), sequence_colors[y], dimness);
        }
    }
}
