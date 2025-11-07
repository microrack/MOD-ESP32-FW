#pragma once

#include "../urack_types.h"
#include "../screen_switcher.h"
#include "midi_settings_state.h"

class MidiSettingsState;

class MidiSettings : public ScreenInterface {
public:
    MidiSettings(Display* display, MidiSettingsState* state);
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
    };

    static constexpr MenuItemInfo items[MENU_COUNT] = {
        {"Channel", SingleItem},
        {" A", ChannelItem},
        {" B", ChannelItem},
        {" C", ChannelItem},
        {"CLK", ChannelItem},
        {"RST", ChannelItem},
        {"Clock", SingleItem}
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
    MenuItems current_item;
    bool is_editing;
    int row_number; // current column position within row (0 = first column, 1 = second column for ChannelItem)

    void render(void);
    void render_menu(void);
    void handle_input(Event* event);
    void handle_menu_input(Event* event);
};
