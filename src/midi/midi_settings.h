#pragma once

#include "../urack_types.h"
#include "../screen_switcher.h"
#include "midi_settings_state.h"
#include "midi_processor.h"

class MidiSettings : public ScreenInterface {
public:
    MidiSettings(Display* display, MidiSettingsState* state, MidiProcessor* processor, ScreenSwitcher* screen_switcher = nullptr);
    void set_screen_switcher(ScreenSwitcher* screen_switcher);
    void enter() override;
    void exit() override;
    void update(Event* event) override;

private:
    enum MenuItems {
        MENU_CHANNEL,
        MENU_OUT_A,
        MENU_OUT_B,
        MENU_OUT_C,
        MENU_CLOCK_OUT,
        MENU_RESET_OUT,
        MENU_CLOCK,
        MENU_COUNT
    };

    enum ColumnType {
        SingleItem = 1,
        ChannelItem = 2
    };

    struct MenuItemInfo {
        const char* text;
        ColumnType type; // number of columns
        union {
            void* unused;
            int output_idx; // output index for ChannelItem (0-4)
        } data;
    };

    static constexpr MenuItemInfo items[MENU_COUNT] = {
        {"Channel", SingleItem, {.unused = nullptr}},
        {" A", ChannelItem, {.output_idx = 0}},
        {" B", ChannelItem, {.output_idx = 1}},
        {" C", ChannelItem, {.output_idx = 2}},
        {"CLK", ChannelItem, {.output_idx = 3}},
        {"RST", ChannelItem, {.output_idx = 4}},
        {"Clock", SingleItem, {.unused = nullptr}}
    };

    enum Direction {
        UP = -1,
        DOWN = 1
    };

    const int COL1_X = 0;
    const int COL2_X = 30;
    const int COL3_X = 100;
    const int COL2_WIDTH = COL3_X - COL2_X; // Width of column 2
    const int COL3_WIDTH = SCREEN_WIDTH - COL3_X; // Width of column 3
    const int LINE_HEIGHT = 8;

    MidiSettingsState* state;
    MidiProcessor* processor;
    ScreenSwitcher* screen_switcher;
    MenuItems current_item;
    bool is_editing;
    int row_number; // current column position within row (0 = first column, 1 = second column for ChannelItem)

    void render(void);
    void render_menu(void);
    void handle_input(Event* event);
    void handle_menu_input(Event* event);
};
