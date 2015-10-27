
#include "seq.h"

/*******************************************************************************
 * Global data
 ******************************************************************************/

// Global settings
u8 lp_midi_port = USBMIDI;

// Program state
LpState lp_state = LP_NUM_MODES;
u8 lp_flags = 0x00;

u16 lp_tap_tempo_timer = 1000;
u16 lp_tap_tempo_sum = 0;
u8 lp_tap_tempo_counter = 0;

// Data
Scale lp_scale;
Voices lp_voices;
PadNotes lp_pad_notes;
NoteBank lp_note_bank;
NoteBank lp_note_storage;
u8 lp_lit_pads[GRID_SIZE];
Sequencer lp_sequencer;

// UI
Slider lp_tempo_slider;
Slider lp_swing_slider;

Keyboard lp_keyboard;
Slider lp_row_offset_slider;

Slider lp_control_sens_slider;
Slider lp_control_offset_slider;

ControlBank lp_user_control_bank;

/*******************************************************************************
 * App functionality
 ******************************************************************************/

u8 tap_tempo_handle_press(u8 index, u8 value)
{
    if (index != LP_CLICK || value == 0)
    {
        return 0;
    }

    u8 success = 0;

    if (lp_tap_tempo_timer < 1000)
    {
        lp_tap_tempo_sum += lp_tap_tempo_timer;
        lp_tap_tempo_counter++;

        if (lp_tap_tempo_counter >= 3)
        {
            sequencer_set_tempo_millis(
                &lp_sequencer,
                lp_tap_tempo_sum
                / lp_tap_tempo_counter
                / STEPS_PER_PAD);
            success = 1;
        }
    }
    else
    {
        lp_tap_tempo_sum = 0;
        lp_tap_tempo_counter = 0;
    }

    lp_tap_tempo_timer = 0;

    return success;
}

void session_mode_become_active()
{
    
}

void session_mode_become_inactive()
{
    
}

void session_setup_become_active()
{
    
} 

void session_setup_become_inactive()
{
    
}

void session_mode_draw()
{
    session_draw(&lp_sequencer);
}

void session_setup_draw()
{
    for (u8 i = 0; i < GRID_SIZE; i++)
    {
        number_draw(lp_sequencer.sequences[i].channel,
                    coord_to_index(CHANNEL_X, row_to_seq(i)),
                    CHANNEL_BITS,
                    sequence_colors[i]);
    }
}

u8 session_mode_handle_press(u8 index, u8 value)
{
    if (session_handle_press(&lp_sequencer, index, value)) { }
    else
    {
        return 0;
    }

    return 1;
}

u8 session_setup_handle_press(u8 index, u8 value)
{
    for (u8 i = 0; i < GRID_SIZE; i++)
    {
        u8 pos = coord_to_index(CHANNEL_X, row_to_seq(i));
        Sequence* s = &lp_sequencer.sequences[i];

        // Pre-detect if the press will be handled so that we can turn off any
        // note on the current channel before switching to a new channel.
        if (index >= pos && index < pos + CHANNEL_BITS)
        {
            sequence_kill_current_note(s);
        }

        if (number_handle_press(
                &s->channel,
                index, value,
                pos,
                CHANNEL_BITS))
        {
            return 1;
        }
    }

    return 0;
}

void sequencer_mode_become_active()
{
    
}

void sequencer_mode_become_inactive()
{
    
}

void sequencer_setup_become_active()
{
    
} 

void sequencer_setup_become_inactive()
{
    
}

void sequencer_mode_draw()
{
    grid_draw(&lp_sequencer);
}

void sequencer_setup_draw()
{
    slider_set_value(
        &lp_tempo_slider,
        millis_to_bpm(lp_sequencer.step_millis) / TEMPO_MUL);

    slider_draw(&lp_tempo_slider, TEMPO_POS, TEMPO_COLOR);
    slider_draw(&lp_swing_slider, SWING_POS, SWING_COLOR);
    checkbox_draw(lp_flags, LP_TEMPO_BLINK, BLINK_CHECKBOX_POS);
    checkbox_draw(lp_flags, LP_POSITION_BLINK, BLINK_CHECKBOX_POS + 1);
    checkbox_draw(lp_flags, LP_PORT_CHECKBOX, PORT_CHECKBOX_POS);
    checkbox_draw(lp_flags, LP_SEND_CLOCK, CLOCK_CHECKBOX_POS);
}

u8 sequencer_mode_handle_press(u8 index, u8 value)
{
    if (grid_handle_press(&lp_sequencer, index, value)) { }
    else
    {
        return 0;
    }

    return 1;
}

u8 sequencer_setup_handle_press(u8 index, u8 value)
{
    if (slider_handle_press(&lp_tempo_slider, index, value, TEMPO_POS))
    {
        sequencer_set_tempo(
            &lp_sequencer,
            TEMPO_MUL * slider_get_value(&lp_tempo_slider));
    }
    else if (slider_handle_press(&lp_swing_slider, index, value, SWING_POS))
    {
        sequencer_set_swing(&lp_sequencer, slider_get_value(&lp_swing_slider));
    }
    else if (checkbox_handle_press(
                 lp_flags, LP_TEMPO_BLINK,
                 index, value,
                 BLINK_CHECKBOX_POS))
    {
        if (!flag_is_set(lp_flags, LP_TEMPO_BLINK))
        {
            sequencer_blink_clear(&lp_sequencer, 1, 0);
        }
    }
    else if (checkbox_handle_press(
                 lp_flags, LP_POSITION_BLINK,
                 index, value,
                 BLINK_CHECKBOX_POS + 1))
    {
        if (!flag_is_set(lp_flags, LP_POSITION_BLINK))
        {
            sequencer_blink_clear(&lp_sequencer, 0, 1);
        }
    }
    // Test for the port checkbox before actually handling the press so that
    // all notes on the current port can be turned off first.
    else if (index == PORT_CHECKBOX_POS)
    {
        sequencer_kill_current_notes(&lp_sequencer);

        checkbox_handle_press(
            lp_flags, LP_PORT_CHECKBOX,
            index, value, PORT_CHECKBOX_POS);

        lp_midi_port = flag_is_set(lp_flags, LP_PORT_CHECKBOX)
            ? DINMIDI : USBMIDI;
    }
    else if (checkbox_handle_press(
                 lp_flags, LP_SEND_CLOCK,
                 index, value, CLOCK_CHECKBOX_POS))
    {
        
    }
    else
    {
        return 0;
    }

    return 1;
}

void notes_mode_become_active()
{
    
}

void notes_mode_become_inactive()
{
    
}

void notes_setup_become_active()
{

} 

void notes_setup_become_inactive()
{
    
}

void notes_mode_draw()
{
    layout_draw(sequencer_get_layout(&lp_sequencer));
}

void notes_setup_draw()
{
    Sequence* s = sequencer_get_active(&lp_sequencer);
    Layout* l = &s->layout;

    keyboard_draw(&lp_keyboard);
    slider_draw(&lp_row_offset_slider, ROW_OFFSET_POS, ROW_OFFSET_COLOR);
    checkbox_draw(s->flags, SEQ_RECORD_CONTROL, CONTROL_CHECKBOX_POS);
    checkbox_draw(l->row_offset, LYT_DRUMS, DRUM_CHECKBOX_POS);
    checkbox_draw(s->flags, SEQ_DRUM_MULTICHANNEL, MULTICHANNEL_CHECKBOX_POS);
    number_draw(s->control_code,
                CC_POS, CC_BITS, CC_COLOR);
    
    slider_draw(&lp_control_sens_slider,
                CC_SENS_POS,
                s->control_sgn > 0
                    ? CC_SENS_POS_COLOR
                    : CC_SENS_NEG_COLOR);
    slider_draw(&lp_control_offset_slider,
                CC_OFFSET_POS,
                CC_OFFSET_COLOR);
}

u8 notes_mode_handle_press(u8 index, u8 value)
{
    Sequence* s = sequencer_get_active(&lp_sequencer);
    Layout* l = &s->layout;

    if (layout_handle_transpose(l, index, value))
    {
        keyboard_update_indices(&lp_keyboard);
    }
    else if (sequence_handle_press(s, index, value))
    {
        if (modifier_held(LP_CLICK)
            && value > 0
            && !flag_is_set(s->flags, SEQ_PLAYING))
        {
            u8 beat = lp_tap_tempo_counter % GRID_SIZE;
            Note* n = sequence_get_note(s, beat * STEPS_PER_PAD);
            n->note_number = voices_get_newest(&lp_voices);
            n->velocity = lp_voices.velocity;
            n->flags = 0x00;

            u8 tempo_set = tap_tempo_handle_press(LP_CLICK, 0x7F);
            if (tempo_set && beat == GRID_SIZE - 1)
            {
                sequence_queue_at(s, SEQUENCE_LENGTH - STEPS_PER_PAD + 1, 0);
            }
        }
    }
    else if (index == LP_CLICK && modifier_held(LP_SHIFT))
    {
        lp_flags = toggle_flag(lp_flags, LP_TEMPO_BLINK);
    }
    else
    {
        return 0;
    }

    return 1;
}

u8 notes_setup_handle_press(u8 index, u8 value)
{
    Sequence* s = sequencer_get_active(&lp_sequencer);
    Layout* l = &s->layout;

    if (slider_handle_press(&lp_row_offset_slider, index, value, ROW_OFFSET_POS))
    {
        layout_set_row_offset(l, lp_row_offset_slider.value + 1);
    }
    else if (checkbox_handle_press(
                 s->flags, SEQ_RECORD_CONTROL,
                 index, value, CONTROL_CHECKBOX_POS))
    {

    }
    else if (checkbox_handle_press(
                 l->row_offset, LYT_DRUMS,
                 index, value, DRUM_CHECKBOX_POS))
    {
        layout_set_drums(l);
    }
    else if (index == MULTICHANNEL_CHECKBOX_POS)
    {
        sequence_kill_current_note(s);

        checkbox_handle_press(
            s->flags, SEQ_DRUM_MULTICHANNEL,
            index, value, MULTICHANNEL_CHECKBOX_POS);
    }
    else if (number_handle_press(
                 &s->control_code, index, value,
                 CC_POS, CC_BITS))
    {

    }
    else if (slider_handle_press(
                 &lp_control_sens_slider, index, value, CC_SENS_POS))
    {
        s->control_div = CC_SENS_RESOLUTION * GRID_SIZE
            - slider_get_value(&lp_control_sens_slider);

        if (modifier_held(LP_SHIFT))
        {
            s->control_sgn = -1;
        }
        else
        {
            s->control_sgn = 1;
        }
    }
    else if (slider_handle_press(
                 &lp_control_offset_slider, index, value, CC_OFFSET_POS))
    {
        s->control_offset = slider_get_value(&lp_control_offset_slider);
    }
    else if (layout_handle_transpose(l, index, value))
    {
        keyboard_update_indices(&lp_keyboard);
    }
    else if (keyboard_handle_press(&lp_keyboard, index, value)) { }
    else
    {
        return 0;
    }

    return 1;
}

void user_mode_become_active()
{
    
}

void user_mode_become_inactive()
{
    
}

void user_setup_become_active()
{
    
} 

void user_setup_become_inactive()
{
    
}

void user_mode_draw()
{
    control_bank_draw(&lp_user_control_bank);
}

void user_setup_draw()
{
    control_bank_setup_draw(&lp_user_control_bank);
}

u8 user_mode_handle_press(u8 index, u8 value)
{
    if (control_bank_handle_press(&lp_user_control_bank, index, value, 0)) { }
    else
    {
        return 0;
    }

    return 1;
}

u8 user_setup_handle_press(u8 index, u8 value)
{
    if (control_bank_setup_handle_press(&lp_user_control_bank, index, value)) { }
    else
    {
        return 0;
    }

    return 1;
}


/*******************************************************************************
 * State management
 ******************************************************************************/

void set_state(LpState st, u8 setup)
{
    if (lp_state == st && flag_is_set(lp_flags, LP_IS_SETUP) == setup)
    {
        return;
    }

    if (lp_state == LP_SESSION_MODE)
    {
        if (flag_is_set(lp_flags, LP_IS_SETUP))
        {
            session_setup_become_inactive();
        }
        else
        {
            session_mode_become_inactive();
        }
    }
    else if (lp_state == LP_NOTES_MODE)
    {
        if (flag_is_set(lp_flags, LP_IS_SETUP))
        {
            notes_setup_become_inactive();
        }
        else
        {
            notes_mode_become_inactive();
        }
    }
    else if (lp_state == LP_SEQUENCER_MODE)
    {
        if (flag_is_set(lp_flags, LP_IS_SETUP))
        {
            sequencer_setup_become_inactive();
        }
        else
        {
            sequencer_mode_become_inactive();
        }
    }
    else if (lp_state == LP_USER_MODE)
    {
        if (flag_is_set(lp_flags, LP_IS_SETUP))
        {
            user_setup_become_inactive();
        }
        else
        {
            user_mode_become_inactive();
        }
    }

    clear_leds();

    if (st == LP_SESSION_MODE)
    {
        plot_pad(LP_SESSION, number_colors[LP_SESSION_MODE]);

        if (setup)
        {
            session_setup_become_active();
            plot_setup(on_color);
            session_setup_draw();
        }
        else
        {
            session_mode_become_active();
            plot_setup(number_colors[LP_SESSION_MODE]);
            session_mode_draw();
        }
    }
    else if (st == LP_NOTES_MODE)
    {
        plot_pad(LP_NOTE, number_colors[LP_NOTES_MODE]);

        if (setup)
        {
            notes_setup_become_active();
            plot_setup(on_color);
            notes_setup_draw();
        }
        else
        {
            notes_mode_become_active();
            plot_setup(number_colors[LP_NOTES_MODE]);
            notes_mode_draw();
        }
    }
    else if (st == LP_SEQUENCER_MODE)
    {
        plot_pad(LP_DEVICE, number_colors[LP_SEQUENCER_MODE]);

        if (setup)
        {
            sequencer_setup_become_active();
            plot_setup(on_color);
            sequencer_setup_draw();
        }
        else
        {
            sequencer_mode_become_active();
            plot_setup(number_colors[LP_SEQUENCER_MODE]);
            sequencer_mode_draw();
        }
    }
    else if (st == LP_USER_MODE)
    {
        plot_pad(LP_USER, number_colors[LP_USER_MODE]);

        if (setup)
        {
            user_setup_become_active();
            plot_setup(on_color);
            user_setup_draw();
        }
        else
        {
            user_mode_become_active();
            plot_setup(number_colors[LP_USER_MODE]);
            user_mode_draw();
        }
    }

    sequencer_play_draw(&lp_sequencer);

    lp_state = st;
    lp_flags = assign_flag(lp_flags, LP_IS_SETUP, setup);
}





