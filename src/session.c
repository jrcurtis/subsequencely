
#include "data.h"

#include "session.h"

void session_draw(Sequencer* sr)
{
    u8 seq_i = 0;
    u8 linked_seq_i = 0;
    u8 row_seq_i = 0;
    Sequence* s = 0;
    Sequence* linked_seq = 0;

    u8 copied_sequence = sr->copied_sequence & SQR_COPY_MASK;
    u8 copy_blink = modifier_held(LP_DUPLICATE)
        && lp_sequencer.step_counter % 4 == 0;

    u8 index = coord_to_index(0, GRID_SIZE - 1);

    for (s8 y = GRID_SIZE - 1; y >= 0; y--)
    {
        row_seq_i = row_to_seq(y);

        if (y < GRID_SIZE - 1
            && flag_is_set(linked_seq->flags, SEQ_LINKED_TO))
        {
            linked_seq_i++;
            linked_seq = &s[linked_seq_i];
        }
        else
        {
            seq_i = row_seq_i;
            linked_seq_i = 0;
            s = &sr->sequences[seq_i];
            linked_seq = &s[linked_seq_i];
        }

        u8 step = linked_seq_i * SEQUENCE_LENGTH;

        for (u8 x = 0; x < GRID_SIZE; x++)
        {
            Note* n = sequence_get_note(s, step);

            if (copy_blink
                && copied_sequence % GRID_SIZE == row_seq_i
                && x / (GRID_SIZE / 2) == copied_sequence / GRID_SIZE)
            {
                plot_pad(index,
                         flag_is_set(sr->copied_sequence, SQR_COPY_SWAP)
                         ? drum_colors[2]
                         : on_color);
            }
            else if (modifier_held(LP_CLICK)
                && x == linked_seq->clock_div - 1)
            {
                plot_pad(index, on_color);
            }
            else if (flag_is_set(n->flags, NTE_SKIP))
            {
                plot_pad(index, off_color);
            }
            else
            {
                u8 linked_playhead = s->playhead - linked_seq_i * SEQUENCE_LENGTH;
                u8 dimness = 5 * (linked_playhead / STEPS_PER_PAD != x);
                plot_pad_dim(index, sequence_colors[seq_i], dimness);
            }

            step += STEPS_PER_PAD;
            index++;
        }

        index -= ROW_SIZE + GRID_SIZE;
    }
}


u8 session_handle_press(Sequencer* sr, u8 index, u8 value)
{
    // When duplicate is pressed or released, reset the clipboard.
    if (index == LP_DUPLICATE)
    {
        sequencer_copy(sr, SQR_COPY_MASK, modifier_held(LP_SHIFT));
        return 1;
    }

    u8 x, y;
    if (value == 0 || !index_to_pad(index, &x, &y))
    {
        return 0;
    }

    u8 seq_i = row_to_seq(y);
    Sequence* s = &sr->sequences[seq_i];
    u8 shift = modifier_held(LP_SHIFT);
    u8 step = x * STEPS_PER_PAD;

    if (modifier_held(LP_CLICK))
    {
        s->clock_div = x + 1;
    }
    else if (modifier_held(LP_UNDO))
    {
        sequence_reverse(s);
    }
    else if (modifier_held(LP_DELETE))
    {
        for (u8 i = 0; i < STEPS_PER_PAD; i++)
        {
            sequence_clear_note(s, step + i);
        }
    }
    else if (modifier_held(LP_QUANTISE))
    {
        Note* n = sequence_get_note(s, step);
        u8 skip = !flag_is_set(n->flags, NTE_SKIP);
        for (u8 i = 0; i < STEPS_PER_PAD; i++)
        {
            sequence_set_skip(s, step + i, skip);
        }
    }
    else if (modifier_held(LP_DUPLICATE))
    {
        // If it's on the left half of the pads, pull from live sequence data,
        // but if it's on the right, pull from the cold storage, aka the
        // sequences stored GRID_SIZE offset from the normal ones.
        u8 offset = x / (GRID_SIZE / 2) * GRID_SIZE;
        sequencer_copy_or_paste(sr, seq_i + offset);
    }
    else if (modifier_held(LP_DOUBLE))
    {
        if (seq_i < GRID_SIZE - 1)
        {
            Sequence* linked_s = &sr->sequences[seq_i + 1];
            sequence_toggle_linked_to(s);
            sequence_toggle_linked(linked_s);
        }
    }
    else if (modifier_held(LP_OCTAVE_UP))
    {
        sequence_transpose(s, NUM_NOTES);
    }
    else if (modifier_held(LP_OCTAVE_DOWN))
    {
        sequence_transpose(s, -NUM_NOTES);
    }
    else if (modifier_held(LP_TRANSPOSE_UP))
    {
        sequence_transpose(s, 1);
    }
    else if (modifier_held(LP_TRANSPOSE_DOWN))
    {
        sequence_transpose(s, -1);
    }
    else
    {
        sequence_queue_or_jump(
            s, x * STEPS_PER_PAD, shift ? SEQ_QUEUED_BEAT : SEQ_QUEUED_STEP);
    }

    return 1;
}
