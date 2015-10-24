
#include "data.h"

#include "control_bank.h"

void control_bank_init(ControlBank* cb)
{
    cb->flags = 0x00;

    u8 index = FIRST_PAD;

    for (u8 i = 0; i < GRID_SIZE; i++)
    {
        slider_init(&cb->sliders[i],
                    128 / GRID_SIZE, -1,
                    0);

        for (u8 checkbox_i = 0; checkbox_i < GRID_SIZE; checkbox_i++)
        {
            cb->checkboxes[checkbox_i + i * GRID_SIZE] = 0;
        }

        cb->control_numbers[i] = i + 1;

        cb->channel_numbers[i] = 0;

        cb->checkbox_checkboxes[i] = 0;

        cb->bipolar_checkboxes[i] = 0;

        index += ROW_SIZE;
    }
}

void control_bank_draw(ControlBank* cb)
{
    for (u8 y = 0; y < GRID_SIZE; y++)
    {
        if (cb->checkbox_checkboxes[y])
        {
            for (u8 x = 0; x < GRID_SIZE; x++)
            {
                u8 checkbox_i = x + y * GRID_SIZE;
                checkbox_draw(cb->checkboxes[checkbox_i], coord_to_index(x, y));
            }
        }
        else
        {
            slider_draw(&cb->sliders[y], y, sequence_colors[row_to_seq(y)]);
        }
    }
}

void control_bank_setup_draw(ControlBank* cb)
{
    if (flag_is_set(cb->flags, CBK_SETUP_SHIFTED))
    {
        for (u8 i = 0; i < GRID_SIZE; i++)
        {
            number_draw(cb->control_numbers[i],
                        coord_to_index(CC_X, i), CC_BITS,
                        sequence_colors[row_to_seq(i)]);
        }
    }
    else
    {
        for (u8 i = 0; i < GRID_SIZE; i++)
        {
            u8 checkbox_checkbox_pos = coord_to_index(CHECKBOX_CHECKBOX_X, i);
            u8 bipolar_checkbox_pos = coord_to_index(BIPOLAR_CHECKBOX_X, i);

            checkbox_draw(cb->checkbox_checkboxes[i],
                          checkbox_checkbox_pos);
            checkbox_draw(cb->bipolar_checkboxes[i],
                          bipolar_checkbox_pos);
            number_draw(cb->channel_numbers[i],
                        coord_to_index(CHANNEL_X, i), CHANNEL_BITS,
                        sequence_colors[row_to_seq(i)]);
        }
    }
}

void control_bank_draw_slider(ControlBank* cb, u8 index)
{
    u8 x, y;
    if (!index_to_pad(index, &x, &y)
        || cb->checkbox_checkboxes[y])
    {
        return;
    }

    slider_draw(&cb->sliders[y], y, sequence_colors[row_to_seq(y)]);
}

u8 control_bank_handle_press(
    ControlBank* cb, u8 index, u8 value, u8 aftertouch)
{
    u8 x, y;
    if (value == 0 || !index_to_pad(index, &x, &y))
    {
        return 0;
    }

    if (cb->checkbox_checkboxes[y])
    {
        if (!aftertouch)
        {
            u8 checkbox_i = x + y * GRID_SIZE;
            checkbox_handle_press(
                cb->checkboxes[checkbox_i],
                index, value,
                index);

            send_midi(CC | cb->channel_numbers[y],
                      cb->control_numbers[y] + x,
                      cb->checkboxes[x + y * GRID_SIZE] * 127);
        }
    }
    else if (slider_handle_press(&cb->sliders[y], index, value, y))
    {
        // Regardless of whether this displays as bipolar, midi ccs must actually be positive
        send_midi(CC | cb->channel_numbers[y],
                  cb->control_numbers[y],
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
        if (number_handle_press(
                &cb->control_numbers[y], index, value,
                coord_to_index(CC_X, y), CC_BITS))
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
                cb->checkbox_checkboxes[y],
                index, value,
                coord_to_index(CHECKBOX_CHECKBOX_X, y)))
        {
            
        }
        else if (checkbox_handle_press(
                     cb->bipolar_checkboxes[y],
                     index, value,
                     coord_to_index(BIPOLAR_CHECKBOX_X, y)))
        {
            cb->sliders[y].offset = cb->bipolar_checkboxes[y]
                ? -128 / 2
                : -1;
        }
        else if (number_handle_press(
                     &cb->channel_numbers[y], index, value,
                     coord_to_index(CHANNEL_X, y), CHANNEL_BITS))
        {
            
        }
        else
        {
            return 0;
        }
    }

    return 1;
}
