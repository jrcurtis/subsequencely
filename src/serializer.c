
#include <string.h>

#include "data.h"
#include "scale.h"
#include "sequencer.h"
#include "sequence.h"
#include "control_bank.h"

#include "serializer.h"

/*******************************************************************************
 * Data layout
 *
 * Name                             |  bytes
 * -----------------------------------------
 * app_id                                  2
 * data_version                            2
 *                                   -------
 *                                         4
 *
 * lp_note_bank                          768
 *
 * lp_midi_port                            1
 * lp_rcv_clock_port                       1
 * lp_flags                                2
 *                                   -------
 *                                         4
 *
 * lp_scale
 * --notes                                 2
 *
 * lp_sequencer
 * --step_millis                           1
 * --swing_millis                          1
 *                                   -------
 *                                         2
 *
 * --sequences
 * ----flags                               2
 * ----layout
 * ------root_note                         1
 * ------octave                            1
 * ------row_offset                        1
 * ----channel                             1
 * ----control_code                        1
 * ----control_div                         1
 * ----control_sgn                         1
 * ----control_offset                      1
 * ----clock_div                           1
 * ----zoom                                1
 * ----x                                   1
 * ----y                                   1
 *                                   -------
 *                                        14
 *                                  x8 = 112
 *
 * lp_user_control_bank
 * --sliders
 * ----value                               1
 *                                   -------
 *                                    x8 = 8
 *
 * --checkboxes                            1
 * --control_numbers                       8
 * --channel_numbers                       8
 * --bipolar_checkboxes                    1
 *                                   -------
 *                                        18
 *
 *                                   =======
 *                                       916
 *
 ******************************************************************************/

/*******************************************************************************
 * Utility
 ******************************************************************************/

#define write_bytes(d)   memcpy(((uint8_t*)lp_note_storage) + offset, \
                                (uint8_t*)&(d),                       \
                                sizeof(d));                           \
                         offset += sizeof(d);

#define read_bytes(d)    memcpy((uint8_t*)&d,                         \
                                ((uint8_t*)lp_note_storage) + offset, \
                                sizeof(d));                           \
                         offset += sizeof(d);

uint8_t is_data_saved()
{
    uint32_t temp;

    hal_read_flash(0, (uint8_t*)&temp, sizeof(temp));
    if (temp != APP_HEADER)
    {
        return 0;
    }

    return 1;
}

// After the serializer has trashed the note storage bank, restore it with
// empty notes.
void reset_note_storage()
{
    for (uint16_t i = 0; i < GRID_SIZE * SEQUENCE_LENGTH; i++)
    {
        lp_note_storage[i].note_number = -1;
        lp_note_storage[i].velocity = 0;
        lp_note_storage[i].flags = 0x00;
    }
}

/*******************************************************************************
 * Serializer
 ******************************************************************************/

void serialize_app()
{
    uint32_t offset = 0;

    *lp_app_header = APP_HEADER;

    write_bytes(lp_midi_port);
    write_bytes(lp_rcv_clock_port);
    write_bytes(lp_flags);

    write_bytes(lp_scale.notes);

    write_bytes(lp_sequencer.step_millis);
    write_bytes(lp_sequencer.swing_millis);

    for (uint8_t i = 0; i < GRID_SIZE; i++)
    {
        Sequence* s = &lp_sequencer.sequences[i];
        write_bytes(s->flags);
        write_bytes(s->layout.root_note);
        write_bytes(s->layout.octave);
        write_bytes(s->layout.row_offset);
        write_bytes(s->channel);

        write_bytes(s->control_code);
        write_bytes(s->control_div);
        write_bytes(s->control_sgn);
        write_bytes(s->control_offset);

        write_bytes(s->clock_div);

        write_bytes(s->zoom);
        write_bytes(s->x);
        write_bytes(s->y);
    }

    for (uint8_t i = 0; i < GRID_SIZE; i++)
    {
        write_bytes(lp_user_control_bank.sliders[i].value);
    }

    write_bytes(lp_user_control_bank.checkboxes);
    write_bytes(lp_user_control_bank.control_numbers);
    write_bytes(lp_user_control_bank.channel_numbers);
    write_bytes(lp_user_control_bank.bipolar_checkboxes);

    hal_write_flash(
        0, lp_buffer, ((uint8_t*)lp_note_storage) + offset - lp_buffer);

    reset_note_storage();
}

void serialize_clear()
{
    if (!is_data_saved())
    {
        return;
    }

    uint32_t temp = 0;
    hal_write_flash(0, (uint8_t*)&temp, sizeof(temp));
}

/*******************************************************************************
 * Deserializer
 ******************************************************************************/

void deserialize_app()
{
    if (!is_data_saved())
    {
        return;
    }

    hal_read_flash(0, lp_buffer, USER_AREA_SIZE);

    uint32_t offset = 0;
    uint16_t temp16 = 0;
    uint8_t temp8 = 0;

    read_bytes(lp_midi_port);
    read_bytes(lp_rcv_clock_port);
    read_bytes(lp_flags);

    read_bytes(temp16);
    scale_set_notes(&lp_scale, temp16);
    layout_assign_pads(&sequencer_get_active(&lp_sequencer)->layout);

    read_bytes(temp8);
    sequencer_set_tempo_millis(&lp_sequencer, temp8);

    read_bytes(temp8);
    sequencer_set_swing_millis(&lp_sequencer, temp8);

    for (uint8_t i = 0; i < GRID_SIZE; i++)
    {
        Sequence* s = &lp_sequencer.sequences[i];

        // Some flags don't make sense to restore, or would require extra
        // work to avoid creating an inconsistent state on load.
        const uint16_t bad_flags =
            SEQ_PLAYING | SEQ_SOLOED | SEQ_QUEUED_MASK
            | SEQ_ACTIVE | SEQ_DID_RECORD_AHEAD;
        read_bytes(temp16);
        s->flags = (s->flags & bad_flags) | (temp16 & ~bad_flags);

        read_bytes(s->layout.root_note);
        read_bytes(s->layout.octave);
        read_bytes(s->layout.row_offset);
        read_bytes(s->channel);

        read_bytes(s->control_code);
        read_bytes(s->control_div);
        read_bytes(s->control_sgn);
        read_bytes(s->control_offset);

        read_bytes(s->clock_div);

        read_bytes(s->zoom);
        read_bytes(s->x);
        read_bytes(s->y);
    }

    for (uint8_t i = 0; i < GRID_SIZE; i++)
    {
        read_bytes(lp_user_control_bank.sliders[i].value);
    }

    read_bytes(lp_user_control_bank.checkboxes);
    read_bytes(lp_user_control_bank.control_numbers);
    read_bytes(lp_user_control_bank.channel_numbers);
    read_bytes(lp_user_control_bank.bipolar_checkboxes);

    reset_note_storage();
}
