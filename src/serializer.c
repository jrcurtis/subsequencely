
#include "data.h"
#include "scale.h"
#include "sequencer.h"
#include "sequence.h"
#include "control_bank.h"

#include "serializer.h"

const uint16_t APP_ID = ('S' << 8) | 'q';
const uint16_t DATA_VERSION = 1;

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
 * lp_note_bank                          768
 *
 *                                   =======
 *                                       916
 *
 ******************************************************************************/

/*******************************************************************************
 * Utility
 ******************************************************************************/

#define write_bytes(d)  { hal_write_flash(offset, (uint8_t*)&(d), sizeof(d));  \
                          offset += sizeof(d); }

#define read_bytes(d)   { hal_read_flash(offset, (uint8_t*)&(d), sizeof(d));   \
                          offset += sizeof(d); }

/*******************************************************************************
 * Serializer
 ******************************************************************************/

void serialize_app()
{
    uint32_t offset = 0;

    write_bytes(APP_ID);
    write_bytes(DATA_VERSION);

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

    write_bytes(lp_note_bank);
}

/*******************************************************************************
 * Deserializer
 ******************************************************************************/

void deserialize_app()
{
    uint32_t offset = 0;
    uint16_t temp16 = 0;
    uint8_t temp8 = 0;

    read_bytes(temp16);
    if (temp16 != APP_ID)
    {
        return;
    }

    read_bytes(temp16);
    if (temp16 != DATA_VERSION)
    {
        return;
    }

    read_bytes(lp_midi_port);
    read_bytes(lp_rcv_clock_port);
    read_bytes(lp_flags);

    read_bytes(temp16);
    scale_set_notes(&lp_scale, temp16);

    read_bytes(temp8);
    sequencer_set_tempo_millis(&lp_sequencer, temp8);

    read_bytes(temp8);
    sequencer_set_swing_millis(&lp_sequencer, temp8);

    for (uint8_t i = 0; i < GRID_SIZE; i++)
    {
        Sequence* s = &lp_sequencer.sequences[i];
        read_bytes(s->flags);
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

    read_bytes(lp_note_bank);

}
