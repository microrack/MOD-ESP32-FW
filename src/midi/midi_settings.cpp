#include "midi.h"
#include "midi_settings.h"
#include "ble_midi.h"
#include "../util.h"

MidiSettings::MidiSettings(Display* display, MidiSettingsState* state, SignalProcessor* processor, ScreenSwitcher* screen_switcher)
        : ScreenInterface(display), state(state), processor(processor), screen_switcher(screen_switcher),
            current_item(MENU_CHANNEL), is_editing(false), selected_row(0), scroll_offset(0), row_number(0), paginator_selected(true) {}

void MidiSettings::set_screen_switcher(ScreenSwitcher* screen_switcher) {
    this->screen_switcher = screen_switcher;
}

void MidiSettings::enter() {
    current_item = MENU_CHANNEL;
    is_editing = false;
    selected_row = 0;
    scroll_offset = 0;
    row_number = 0;
    paginator_selected = true;
}

void MidiSettings::exit() {

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
    int visible_rows = (SCREEN_HEIGHT - PAGINATOR_HEIGHT - 2) / LINE_HEIGHT;
    if (visible_rows < 1) {
        visible_rows = 1;
    }

    int max_scroll = (ROW_COUNT > visible_rows) ? (ROW_COUNT - visible_rows) : 0;
    scroll_offset = clampi(scroll_offset, 0, max_scroll);

    for (int i = 0; i < visible_rows; i++) {
        int row_idx = scroll_offset + i;
        if (row_idx >= ROW_COUNT) {
            break;
        }

        int y = PAGINATOR_HEIGHT + 2 + i * LINE_HEIGHT;
        bool is_selected = (row_idx == selected_row) && !paginator_selected;
        bool is_editing_selected = is_selected && is_editing;
        const RenderRow& row = rows[row_idx];

        switch (row.type) {
            case RowMenu: {
                const MenuItemInfo& item = items[row.menu_index];
                ColumnType col_type = item.type;
                bool type_selected = is_selected && (col_type == SingleItem || row_number == 0);
                bool channel_selected = is_selected && col_type == ChannelItem && row_number == 1;

                if (is_selected) {
                    if (col_type == ChannelItem) {
                        if (row_number == 1) {
                            if (is_editing_selected) {
                                display->fillRect(COL3_X, y, COL3_WIDTH, LINE_HEIGHT, SSD1306_WHITE);
                            } else {
                                display->drawRect(COL3_X, y, COL3_WIDTH, LINE_HEIGHT, SSD1306_WHITE);
                            }
                        } else {
                            if (is_editing_selected) {
                                display->fillRect(COL2_X, y, COL2_WIDTH, LINE_HEIGHT, SSD1306_WHITE);
                            } else {
                                display->drawRect(COL2_X, y, COL2_WIDTH, LINE_HEIGHT, SSD1306_WHITE);
                            }
                        }
                    } else {
                        if (is_editing_selected) {
                            display->fillRect(COL1_X, y, SCREEN_WIDTH, LINE_HEIGHT, SSD1306_WHITE);
                        } else {
                            display->drawRect(COL1_X, y, SCREEN_WIDTH, LINE_HEIGHT, SSD1306_WHITE);
                        }
                    }
                }

                if (col_type == SingleItem) {
                    if (is_editing_selected) {
                        display->setTextColor(SSD1306_BLACK, SSD1306_WHITE);
                    } else {
                        display->setTextColor(SSD1306_WHITE, SSD1306_BLACK);
                    }
                    display->setCursor(COL1_X + 2, y + 1);
                    display->print(item.text);
                    display->print(" ");
                    if (row.menu_index == MENU_CHANNEL) {
                        display->print(state->get_midi_channel_str());
                    } else if (row.menu_index == MENU_CLOCK) {
                        display->print(state->get_midi_clk_type_str());
                    }
                } else {
                    display->setTextColor(SSD1306_WHITE, SSD1306_BLACK);
                    display->setCursor(COL1_X + 2, y + 1);
                    display->print(item.text);

                    bool col2_editing = type_selected && is_editing;
                    if (col2_editing) {
                        display->setTextColor(SSD1306_BLACK, SSD1306_WHITE);
                    } else {
                        display->setTextColor(SSD1306_WHITE, SSD1306_BLACK);
                    }
                    display->setCursor(COL2_X, y + 1);
                    display->print(state->get_midi_out_type_str(item.data.output_idx));

                    if (channel_selected && is_editing) {
                        display->setTextColor(SSD1306_BLACK, SSD1306_WHITE);
                    } else {
                        display->setTextColor(SSD1306_WHITE, SSD1306_BLACK);
                    }
                    display->setCursor(COL3_X, y + 1);
                    display->print(state->get_midi_out_channel_str(item.data.output_idx));
                }
                break;
            }

            case RowBluetoothToggle: {
                if (is_selected && is_editing_selected) {
                    display->fillRect(COL1_X, y, SCREEN_WIDTH, LINE_HEIGHT, SSD1306_WHITE);
                    display->setTextColor(SSD1306_BLACK, SSD1306_WHITE);
                } else if (is_selected) {
                    display->drawRect(COL1_X, y, SCREEN_WIDTH, LINE_HEIGHT, SSD1306_WHITE);
                    display->setTextColor(SSD1306_WHITE, SSD1306_BLACK);
                } else {
                    display->setTextColor(SSD1306_WHITE, SSD1306_BLACK);
                }

                display->setCursor(COL1_X + 2, y + 1);
                display->print("Bluetooth ");
                display->print(state->get_bluetooth_enabled_str());
                break;
            }

            case RowBluetoothStatus: {
                if (is_selected) {
                    display->drawRect(COL1_X, y, SCREEN_WIDTH, LINE_HEIGHT, SSD1306_WHITE);
                }
                display->setTextColor(SSD1306_WHITE, SSD1306_BLACK);
                display->setCursor(COL1_X + 2, y + 1);
                display->print("BT Status: ");
                if (state->get_bluetooth_enabled()) {
                    display->print(ble_midi.is_connected() ? "Connected" : "Waiting...");
                } else {
                    display->print("Disabled");
                }
                break;
            }
        }
    }
}

void MidiSettings::handle_input(Event* event) {
    if (event == nullptr) return;

    if(is_editing) {
        if (event->button_sw == ButtonPress || event->button_a == ButtonPress) {
            is_editing = false;
        }
    } else {
        if (event->button_sw == ButtonPress) {
            if (paginator_selected) {
                paginator_selected = false;
            } else {
                const RenderRow& row = rows[selected_row];
                if (row.type != RowBluetoothStatus) {
                    is_editing = true;
                    row_number = 0;

                    // cleanup last cc array when editing a MIDI output row
                    if (row.type == RowMenu) {
                        for(int i = 0; i < MIDI_CHANNEL_COUNT; i++) {
                            processor->last_cc[i] = 128;
                            processor->pitchbend[i] = 0;
                        }
                    }
                }
            }
        }

        if(event->button_a == ButtonPress) {
            if (!paginator_selected) {
                paginator_selected = true;
            } else {
                screen_switcher->set_screen(MidiScreen::MidiScreenInfo);
            }
        }
    }

    handle_menu_input(event);
}

void MidiSettings::handle_menu_input(Event* event) {
    if (event->encoder != 0) {
        int visible_rows = (SCREEN_HEIGHT - PAGINATOR_HEIGHT - 2) / LINE_HEIGHT;
        if (visible_rows < 1) {
            visible_rows = 1;
        }
        int max_scroll = (ROW_COUNT > visible_rows) ? (ROW_COUNT - visible_rows) : 0;

        if (paginator_selected && !is_editing) {
            scroll_offset = clampi(scroll_offset + event->encoder, 0, max_scroll);
            selected_row = clampi(selected_row, scroll_offset, scroll_offset + visible_rows - 1);
        } else if (is_editing) {
            const RenderRow& row = rows[selected_row];
            if (row.type == RowMenu) {
                current_item = (MenuItems)row.menu_index;
                const MenuItemInfo& item = items[current_item];

                if (item.type == SingleItem) {
                    switch (current_item) {
                        case MENU_CHANNEL:
                            state->set_midi_channel((MidiChannel)clampi(state->get_midi_channel() + event->encoder,
                                                                     state->get_min_midi_channel(),
                                                                     state->get_max_midi_channel()));
                            break;
                        case MENU_CLOCK:
                            state->set_midi_clk_type((MidiClkType)clampi(state->get_midi_clk_type() + event->encoder,
                                                                       state->get_min_midi_clk_type(),
                                                                       state->get_max_midi_clk_type()));
                            break;
                        default:
                            break;
                    }
                } else {
                    int idx = item.data.output_idx;
                    if (row_number == 1) {
                        state->set_midi_out_channel(idx, (MidiChannel)clampi(state->get_midi_out_channel(idx) + event->encoder,
                                                                           state->get_min_midi_out_channel(),
                                                                           state->get_max_midi_out_channel()));
                    } else {
                        state->set_midi_out_type(idx, (MidiOutType)clampi(state->get_midi_out_type(idx) + event->encoder,
                                                                       state->get_min_midi_out_type(idx),
                                                                       state->get_max_midi_out_type(idx)));
                    }
                }
                state->store();
            } else if (row.type == RowBluetoothToggle) {
                bool enabled = state->get_bluetooth_enabled();
                state->set_bluetooth_enabled(!enabled);

                if (state->get_bluetooth_enabled()) {
                    ble_midi.enable();
                } else {
                    ble_midi.disable();
                }
                state->store();
            }
        } else {
            Direction direction = (event->encoder > 0) ? DOWN : UP;
            int steps = (event->encoder > 0) ? event->encoder : -event->encoder;

            for (int step = 0; step < steps; step++) {
                const RenderRow& current_row = rows[selected_row];
                ColumnType current_type = SingleItem;
                if (current_row.type == RowMenu) {
                    current_type = items[current_row.menu_index].type;
                }

                if (current_type == SingleItem || current_row.type != RowMenu) {
                    int new_row = selected_row + direction;
                    if (new_row >= 0 && new_row < ROW_COUNT) {
                        selected_row = new_row;
                        row_number = 0;
                    }
                } else {
                    int new_row_number = row_number + direction;
                    if (new_row_number < 0) {
                        int new_row = selected_row + direction;
                        if (new_row >= 0) {
                            selected_row = new_row;
                            if (rows[selected_row].type == RowMenu) {
                                row_number = items[rows[selected_row].menu_index].type - 1;
                            } else {
                                row_number = 0;
                            }
                        } else {
                            row_number = 0;
                        }
                    } else if (new_row_number >= current_type) {
                        int new_row = selected_row + direction;
                        if (new_row < ROW_COUNT) {
                            selected_row = new_row;
                            row_number = 0;
                        } else {
                            row_number = current_type - 1;
                        }
                    } else {
                        row_number = new_row_number;
                    }
                }
            }
        }

        scroll_offset = clampi(scroll_offset, 0, max_scroll);
        if (selected_row < scroll_offset) {
            scroll_offset = selected_row;
        } else if (selected_row >= scroll_offset + visible_rows) {
            scroll_offset = selected_row - visible_rows + 1;
        }
        scroll_offset = clampi(scroll_offset, 0, max_scroll);
    }

    if (rows[selected_row].type == RowMenu) {
        current_item = (MenuItems)rows[selected_row].menu_index;
    }

    if(is_editing && !paginator_selected && rows[selected_row].type == RowMenu &&
       (processor->last_cc[current_item] != 0 || processor->pitchbend[current_item] != 0)) {
        const MenuItemInfo& item = items[current_item];
        int idx = item.data.output_idx;
        MidiChannel channel = state->get_midi_out_channel(idx);
        if(channel == MidiChannelUnchanged) {
            channel = state->get_midi_channel();
        }

        int last_cc = 128;
        int last_pitchbend = 0;

        if(channel != MidiChannelAll) {
            last_cc = processor->last_cc[channel];
            last_pitchbend = processor->pitchbend[channel];
        } else {
            for(int i = 0; i < MIDI_CHANNEL_COUNT; i++) {
                if(processor->last_cc[i] != 128) {
                    last_cc = processor->last_cc[i];
                }
                if(processor->pitchbend[i] != 0) {
                    last_pitchbend = processor->pitchbend[i];
                }
            }
        }

        if(last_cc != 128) {
            state->set_midi_out_type(idx, (MidiOutType)((int)MidiOutType::MidiOutCc0 + last_cc));
        }
        if(last_pitchbend != 0) {
            state->set_midi_out_type(idx, MidiOutType::MidiOutPitchBend);
        }

        processor->last_cc[current_item] = 0;
        processor->pitchbend[current_item] = 0;
    }
}



void MidiSettings::update(Event* event) {
    handle_input(event);
    render();
}
