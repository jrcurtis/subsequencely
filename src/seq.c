
#include "seq.h"

/*******************************************************************************
 * Global data
 ******************************************************************************/

// Global settings
uint8_t lp_midi_port = DINMIDI;
uint8_t lp_rcv_clock_port = DINMIDI;

// Program state
LpState lp_state = LP_NUM_MODES;
uint16_t lp_flags = 0x0000;

uint16_t lp_tap_tempo_timer = 1000;
uint16_t lp_tap_tempo_sum = 0;
uint8_t lp_tap_tempo_counter = 0;

// Data
uint8_t lp_buffer[LP_BUFFER_SIZE];
uint32_t* lp_app_header = (uint32_t*)lp_buffer;
Note* lp_note_bank = (Note*)(lp_buffer + sizeof(uint32_t));
Note* lp_note_storage = (Note*)(lp_buffer + sizeof(uint32_t)) + NOTE_BANK_SIZE;

Scale lp_scale;
Voices lp_voices;
PadNotes lp_pad_notes;
PadNotes lp_pad_highlights;
Sequencer lp_sequencer;

TextDisplayer lp_text_displayer;
int8_t lp_quick_scale_current;

// UI
Slider lp_tempo_slider;
Slider lp_swing_slider;

Keyboard lp_keyboard;
Slider lp_row_offset_slider;

Slider lp_control_sens_slider;
Slider lp_control_offset_slider;

ControlBank lp_user_control_bank;

ModWheel lp_mod_wheel;

/*******************************************************************************
 * App functionality
 ******************************************************************************/

uint8_t tap_tempo_handle_press(uint8_t index, uint8_t value)
{
    if (index != LP_CLICK || value == 0)
    {
        return 0;
    }

    uint8_t success = 0;

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

uint8_t arp_handle_press(uint8_t index, uint8_t value)
{
    if (index != LP_RECORD || value == 0)
    {
        return 0;
    }

    lp_flags = toggle_flag(lp_flags, LP_IS_ARP);

    return 1;
}

void arp_draw()
{
    plot_pad(LP_RECORD,
             flag_is_set(lp_flags, LP_IS_ARP) ? c_note_color : off_color);
}

void session_mode_become_active()
{
    
}

void session_mode_become_inactive()
{
    
}

void session_setup_become_active()
{
    plot_pad(SAVE_BUTTON_INDEX, number_colors[1]);
    plot_pad(CLEAR_BUTTON_INDEX, number_colors[1]);
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
    for (uint8_t i = 0; i < GRID_SIZE; i++)
    {
        number_draw(lp_sequencer.sequences[i].channel,
                    coord_to_index(CHANNEL_X, row_to_seq(i)),
                    CHANNEL_BITS,
                    sequence_colors[i]);
    }
}

uint8_t session_mode_handle_press(uint8_t index, uint8_t value)
{
    if (session_handle_press(&lp_sequencer, index, value)) { }
    else
    {
        return 0;
    }

    return 1;
}

uint8_t session_setup_handle_press(uint8_t index, uint8_t value)
{
    for (uint8_t i = 0; i < GRID_SIZE; i++)
    {
        uint8_t pos = coord_to_index(CHANNEL_X, row_to_seq(i));
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

    if (index == SAVE_BUTTON_INDEX && value > 0)
    {
        clear_leds();
        serialize_app();
        session_setup_draw();
        plot_pad(SAVE_BUTTON_INDEX, number_colors[2]);
    }
    else if (index == CLEAR_BUTTON_INDEX)
    {
        if (value > 0)
        {
            plot_pad(CLEAR_BUTTON_INDEX, on_color);
            serialize_clear();
        }
        else
        {
            plot_pad(CLEAR_BUTTON_INDEX, number_colors[0]);
        }
    }
    else
    {
        return 0;
    }


    return 1;
}

void sequencer_mode_become_active()
{
    grid_update_cache(&lp_sequencer, 0);
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
    checkbox_draw(lp_flags, LP_POSITION_BLINK, PLAYHEAD_CHECKBOX_POS);
    checkbox_draw(lp_flags, LP_PORT_CHECKBOX, PORT_CHECKBOX_POS);
    checkbox_draw(lp_flags, LP_SEND_CLOCK, CLOCK_CHECKBOX_POS);
    checkbox_draw(lp_flags, LP_RCV_CLOCK, RCV_CLOCK_CHECKBOX_POS);
    checkbox_draw(lp_flags, LP_RCV_CLOCK_PORT, RCV_CLOCK_PORT_CHECKBOX_POS);
}

uint8_t sequencer_mode_handle_press(uint8_t index, uint8_t value)
{
    if (grid_handle_press(&lp_sequencer, index, value)) { }
    else
    {
        return 0;
    }

    return 1;
}

uint8_t sequencer_setup_handle_press(uint8_t index, uint8_t value)
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
                 PLAYHEAD_CHECKBOX_POS))
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
    else if (checkbox_handle_press(
                 lp_flags, LP_RCV_CLOCK,
                 index, value, RCV_CLOCK_CHECKBOX_POS))
    {
        lp_tap_tempo_counter = 0;
        lp_tap_tempo_sum = 0;
        lp_tap_tempo_timer = 0;
    }
    else if (checkbox_handle_press(
                 lp_flags, LP_RCV_CLOCK_PORT,
                 index, value, RCV_CLOCK_PORT_CHECKBOX_POS))
    {
        lp_rcv_clock_port = flag_is_set(lp_flags, LP_RCV_CLOCK_PORT)
            ? DINMIDI : USBMIDI;

        lp_tap_tempo_counter = 0;
        lp_tap_tempo_sum = 0;
        lp_tap_tempo_timer = 0;
    }
    else
    {
        return 0;
    }

    return 1;
}

void notes_mode_become_active()
{
    // Layout is drawn when the mode becomes active (and when switching active
    // tracks) but not in the normal draw function because it's mostly static
    // and gets updated by layout_light_note instead.
    layout_draw(sequencer_get_layout(&lp_sequencer));
}

void notes_mode_become_inactive()
{
    Sequence* s = sequencer_get_active(&lp_sequencer);
    sequence_kill_voices(s, 0);
}

void notes_setup_become_active()
{

} 

void notes_setup_become_inactive()
{

}

void notes_mode_draw()
{
    sequence_draw(sequencer_get_active(&lp_sequencer));
}

void notes_setup_draw()
{
    Sequence* s = sequencer_get_active(&lp_sequencer);
    Layout* l = &s->layout;

    keyboard_draw(&lp_keyboard, flag_is_set(s->flags, NOTE_HIGHLIGHT_ONLY));
    slider_draw(&lp_row_offset_slider, ROW_OFFSET_POS, ROW_OFFSET_COLOR);

    checkbox_draw(s->flags, SEQ_RECORD_CONTROL, CONTROL_CHECKBOX_POS);
    checkbox_draw(l->row_offset, LYT_DRUMS, DRUM_CHECKBOX_POS);
    checkbox_draw(s->flags, SEQ_DRUM_MULTICHANNEL, MULTICHANNEL_CHECKBOX_POS);
    checkbox_draw(s->flags, SEQ_FULL_VELOCITY, VELOCITY_CHECKBOX_POS);
    checkbox_draw(s->flags, SEQ_MOD_WHEEL, MOD_WHEEL_CHECKBOX_POS);
    checkbox_draw(s->flags, SEQ_MOD_CC, MOD_CC_CHECKBOX_POS);
    checkbox_draw(s->flags, NOTE_HIGHLIGHT_ONLY, NOTE_HIGHLIGHT_ONLY_POS);

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

uint8_t notes_mode_handle_press(uint8_t index, uint8_t value, uint8_t shift_held, uint8_t note_held)
{
    Sequence* s = sequencer_get_active(&lp_sequencer);
    Layout* l = &s->layout;

    if (note_held)
    {
        if (quick_scale_handle_prev_next(l, index, value, note_held, flag_is_set(s->flags, NOTE_HIGHLIGHT_ONLY))) 
        {
            sequence_kill_voices(s, 0);
            layout_draw(l);
            keyboard_update_indices(&lp_keyboard);
        }
    }
    else if (layout_handle_transpose(l, index, value))
    {
        sequence_kill_voices(s, 0);
        layout_draw(l);
        keyboard_update_indices(&lp_keyboard);
    }
    else if (sequence_handle_press(s, index, value))
    {
        if (!flag_is_set(lp_flags, LP_RCV_CLOCK)
            && modifier_held(LP_CLICK)
            && value > 0
            && !flag_is_set(s->flags, SEQ_PLAYING))
        {
            uint8_t beat = lp_tap_tempo_counter % GRID_SIZE;
            Note* n = sequence_get_note(s, beat * STEPS_PER_PAD);
            n->note_number = voices_get_newest(&lp_voices);
            n->velocity = lp_voices.velocity;
            n->flags = 0x00;

            uint8_t tempo_set = tap_tempo_handle_press(LP_CLICK, 0x7F);
            if (tempo_set && beat == GRID_SIZE - 1)
            {
                // Set did_record_ahead so that the just-entered note is not
                // played twice.
                s->flags = set_flag(s->flags, SEQ_DID_RECORD_AHEAD);
                sequence_queue_at(
                    s, SEQUENCE_LENGTH - STEPS_PER_PAD, SEQ_QUEUED_STEP);
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

uint8_t notes_setup_handle_press(uint8_t index, uint8_t value)
{
    Sequence* s = sequencer_get_active(&lp_sequencer);
    Layout* l = &s->layout;

    if (slider_handle_press(&lp_row_offset_slider, index, value, ROW_OFFSET_POS))
    {
        layout_set_row_offset(l, lp_row_offset_slider.value + 1);
    }
    else if (checkbox_handle_press(
                 s->flags, NOTE_HIGHLIGHT_ONLY,
                 index, value, NOTE_HIGHLIGHT_ONLY_POS))
    {
        
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
    else if (checkbox_handle_press(
                 s->flags, SEQ_FULL_VELOCITY,
                 index, value, VELOCITY_CHECKBOX_POS))
    {
        
    }
    else if (checkbox_handle_press(
                 s->flags, SEQ_MOD_WHEEL,
                 index, value, MOD_WHEEL_CHECKBOX_POS))
    {
        sequence_prepare_mod_wheel(s);
    }
    else if (checkbox_handle_press(
                 s->flags, SEQ_MOD_CC,
                 index, value, MOD_CC_CHECKBOX_POS))
    {
        
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
        uint8_t slider_value = slider_get_value(&lp_control_offset_slider);
        s->control_offset = slider_value;

        // Send midi from the offset slider so that you can test the value
        // you put in/use midi learn on a software synth to map parameters.
        send_midi(CC | s->channel,
                  s->control_code,
                  slider_value);

    }
    else if (layout_handle_transpose(l, index, value))
    {
        keyboard_update_indices(&lp_keyboard);
    }
    else if (keyboard_handle_press(&lp_keyboard, index, value, 0, flag_is_set(s->flags, NOTE_HIGHLIGHT_ONLY))) 
    { 
        
    }
    else
    {
        return 0;
    }

    return 1;
}


void scales_setup_become_active() {

}

void scales_setup_become_inactive() {
    text_display_stop(&lp_text_displayer);
}

void scales_setup_draw() {
    Sequence* s = sequencer_get_active(&lp_sequencer);
    //Layout* l = &s->layout;

    quick_scale_draw();
    keyboard_draw_y(&lp_keyboard, 0, 0);
    keyboard_draw_y(&lp_keyboard, 1, 2);
    checkbox_draw(s->flags, NOTE_HIGHLIGHT_ONLY, SCALE_HIGHLIGHT_ONLY_POS);
    checkbox_draw_inv(s->flags, NOTE_HIGHLIGHT_ONLY, SCALE_HIGHLIGHT_ONLY_INV_POS);
}

uint8_t scales_setup_handle_press(uint8_t index, uint8_t value) {
    Sequence* s = sequencer_get_active(&lp_sequencer);
    Layout* l = &s->layout;

    if (checkbox_handle_press(
                 s->flags, NOTE_HIGHLIGHT_ONLY,
                 index, value, SCALE_HIGHLIGHT_ONLY_POS))
    {
        
    }
    else if (checkbox_handle_press(
                 s->flags, NOTE_HIGHLIGHT_ONLY,
                 index, value, SCALE_HIGHLIGHT_ONLY_INV_POS))
    {
        
    }
    else if (quick_scale_handle_press(lp_keyboard.layout, index, value, flag_is_set(s->flags, NOTE_HIGHLIGHT_ONLY))) 
    {
        if (value > 60) {
            set_state(LP_NOTES_MODE, 0, 0);
            return 0;
        }
    }
    else if (layout_handle_transpose(l, index, value))
    {
        keyboard_update_indices(&lp_keyboard);
    }
    else if (keyboard_handle_press(&lp_keyboard, index, value, 0, 0)) 
    { 
        
    }
    else if (keyboard_handle_press(&lp_keyboard, index, value, 2, 1)) 
    { 
        
    }    
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

uint8_t user_mode_handle_press(uint8_t index, uint8_t value)
{
    if (control_bank_handle_press(&lp_user_control_bank, index, value, 0)) { }
    else
    {
        return 0;
    }

    return 1;
}

uint8_t user_setup_handle_press(uint8_t index, uint8_t value)
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

void set_state(LpState st, uint8_t setup, uint8_t shift_held)
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
            if (flag_is_set(lp_flags, LP_IS_SETUP2)) {
                scales_setup_become_inactive();
            } else {
                notes_setup_become_inactive();
            }
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
            if (shift_held) {
                scales_setup_become_active();
                plot_setup(on_color);
                scales_setup_draw();                                
            } else {
                notes_setup_become_active();
                plot_setup(on_color);
                notes_setup_draw();
            }
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
    lp_flags = assign_flag(lp_flags, LP_IS_SETUP2, setup && shift_held);
}





