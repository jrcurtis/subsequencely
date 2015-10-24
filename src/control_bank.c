
#include "control_bank.h"

void control_bank_init(ControlBank* cb)
{
    cb->flags = 0x00;

    u8 index = FIRST_PAD;

    for (u8 i = 0; i < GRID_SIZE; i++)
    {
        slider_init(&cb->sliders[i],
                    HORIZONTAL, i,
                    sequence_colors[row_to_seq(i)],
                    128 / GRID_SIZE, -1,
                    0);

        for (u8 checkbox_i = 0; checkbox_i < GRID_SIZE; checkbox_i++)
        {
            checkbox_init(
                &cb->checkboxes[checkbox_i + i * GRID_SIZE],
                index + checkbox_i,
                0);
        }

        number_init(&cb->control_numbers[i], 7, index + 1, i + 1);

        number_init(&cb->channel_numbers[i], 4, index + 4, 0);

        checkbox_init(&cb->checkbox_checkboxes[i], index, 0);

        checkbox_init(&cb->bipolar_checkboxes[i], index + 2, 0);

        index += ROW_SIZE;
    }
}

void control_bank_draw(ControlBank* cb)
{
    for (u8 i = 0; i < GRID_SIZE; i++)
    {
        if (cb->checkbox_checkboxes[i].value)
        {
            for (u8 checkbox_i = 0; checkbox_i < GRID_SIZE; checkbox_i++)
            {
                checkbox_draw(&cb->checkboxes[checkbox_i + i * GRID_SIZE]);
            }
        }
        else
        {
            slider_draw(&cb->sliders[i]);
        }
    }
}

void control_bank_setup_draw(ControlBank* cb)
{
    if (flag_is_set(cb->flags, CBK_SETUP_SHIFTED))
    {
        for (u8 i = 0; i < GRID_SIZE; i++)
        {
            number_draw(&cb->control_numbers[i],
                        sequence_colors[row_to_seq(i)]);
        }
    }
    else
    {
        for (u8 i = 0; i < GRID_SIZE; i++)
        {
            checkbox_draw(&cb->checkbox_checkboxes[i]);
            checkbox_draw(&cb->bipolar_checkboxes[i]);
            number_draw(&cb->channel_numbers[i],
                        sequence_colors[row_to_seq(i)]);
        }
    }
}

u8 control_bank_handle_press(
    ControlBank* cb, u8 index, u8 value, u8 aftertouch)
{
    u8 x, y;
    if (value == 0 || !index_to_pad(index, &x, &y))
    {
        return 0;
    }

    if (cb->checkbox_checkboxes[y].value)
    {
        if (!aftertouch)
        {
            checkbox_handle_press(
                &cb->checkboxes[x + y * GRID_SIZE],
                index, value);

            send_midi(CC | cb->channel_numbers[y].value,
                      cb->control_numbers[y].value + x,
                      cb->checkboxes[x + y * GRID_SIZE].value * 127);
        }
    }
    else if (slider_handle_press(&cb->sliders[y], index, value))
    {
        // Regardless of whether this displays as bipolar, midi ccs must actually be positive
        send_midi(CC | cb->channel_numbers[y].value,
                  cb->control_numbers[y].value,
                  cb->sliders[y].value - 1);
    }
    else
    {
        return 0;
    }

    return 1;
}

u8 control_bank_setup_handle_press(ControlBank* cb, u8 index, u8 value)
{
    if (index == LP_SHIFT)
    {
        if (value > 0)
        {
            cb->flags = toggle_flag(cb->flags, CBK_SETUP_SHIFTED);
            clear_pad_leds();
        }
        else
        {
            return 0;
        }

        return 1;
    }

    u8 x, y;
    if (!index_to_pad(index, &x, &y))
    {
        return 0;
    }

    if (flag_is_set(cb->flags, CBK_SETUP_SHIFTED))
    {
        if (number_handle_press(&cb->control_numbers[y], index, value))
        {
            
        }
        else
        {
            return 0;
        }
    }
    else
    {
        if (checkbox_handle_press(
                &cb->checkbox_checkboxes[y], index, value))
        {
            
        }
        else if (checkbox_handle_press(
                     &cb->bipolar_checkboxes[y], index, value))
        {
            cb->sliders[y].offset = cb->bipolar_checkboxes[y].value
                ? -128 / 2
                : -1;
        }
        else if (number_handle_press(
                     &cb->channel_numbers[y], index, value))
        {
            
        }
        else
        {
            return 0;
        }
    }

    return 1;
}
