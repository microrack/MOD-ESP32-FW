#include "midi_settings.h"
#include "util.h"

MidiSettings::MidiSettings(Display* display, MidiSettingsState* state)
    : ScreenInterface(display), state(state),
      current_item(MENU_CHANNEL), blink_counter(0), is_editing(false) {}

void MidiSettings::enter() {
    current_item = MENU_CHANNEL;
    is_editing = false;
}

void MidiSettings::exit() {
    display->clearDisplay();
    display->display();
}

void MidiSettings::render() {
    display->clearDisplay();

    display->setTextSize(1);
    display->setTextColor(SSD1306_WHITE);
    display->setCursor(0,0);

    render_menu();

    display->display();
}

void MidiSettings::render_menu() {
    const char* items[MENU_COUNT] = {
        "Channel: %s",
        "Out A: %s",
        "Out B: %s",
        "Out C: %s",
        "Clock: %s"
    };

    char buffer[32];
    for (int i = 0; i < MENU_COUNT; i++) {
        if (i == current_item) {
            if (is_editing) {
                if (blink_counter < 10) {
                    display->print(F("> "));
                } else {
                    display->print(F("  "));
                }
            } else {
                display->print(F("> "));
            }
        } else {
            display->print(F("  "));
        }

        switch (i) {

            case MENU_CHANNEL:
                sprintf(buffer, items[i], state->get_midi_channel_str());
                break;
            case MENU_OUT_A:
                sprintf(buffer, items[i], state->get_midi_out_type_str(0));
                break;
            case MENU_OUT_B:
                sprintf(buffer, items[i], state->get_midi_out_type_str(1));
                break;
            case MENU_OUT_C:
                sprintf(buffer, items[i], state->get_midi_out_type_str(2));
                break;
            case MENU_CLOCK:
                sprintf(buffer, items[i], state->get_midi_clk_type_str());
                break;
        }
        display->println(buffer);
    }
}

void MidiSettings::handle_input(Event* event) {
    if (event == nullptr) return;

    if (event->button_sw == ButtonPress) {
        is_editing = !is_editing;
        blink_counter = 0;
    }

    handle_menu_input(event);
}

void MidiSettings::handle_menu_input(Event* event) {
    if (event->encoder != 0) {
        if (is_editing) {
            // Adjust value
            switch (current_item) {
                case MENU_CHANNEL:
                    state->set_midi_channel((MidiChannel)clampi(state->get_midi_channel() + event->encoder,
                                                             state->get_min_midi_channel(),
                                                             state->get_max_midi_channel()));
                    break;
                case MENU_OUT_A:
                    state->set_midi_out_type(0, (MidiOutType)clampi(state->get_midi_out_type(0) + event->encoder,
                                                                 state->get_min_midi_out_type(),
                                                                 state->get_max_midi_out_type()));
                    break;
                case MENU_OUT_B:
                    state->set_midi_out_type(1, (MidiOutType)clampi(state->get_midi_out_type(1) + event->encoder,
                                                                 state->get_min_midi_out_type(),
                                                                 state->get_max_midi_out_type()));
                    break;
                case MENU_OUT_C:
                    state->set_midi_out_type(2, (MidiOutType)clampi(state->get_midi_out_type(2) + event->encoder,
                                                                 state->get_min_midi_out_type(),
                                                                 state->get_max_midi_out_type()));
                    break;
                case MENU_CLOCK:
                    state->set_midi_clk_type((MidiClkType)clampi(state->get_midi_clk_type() + event->encoder,
                                                               state->get_min_midi_clk_type(),
                                                               state->get_max_midi_clk_type()));
                    break;
            }
            state->store();
        } else {
            // Move selection
            current_item = (MenuItems)clampi(current_item + event->encoder,
                                           0, MENU_COUNT - 1);
        }
    }
}



void MidiSettings::update(Event* event) {
    handle_input(event);

    // Update blink counter
    blink_counter++;
    if (blink_counter >= 20) {
        blink_counter = 0;
    }

    render();
}
