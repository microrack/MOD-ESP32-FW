#pragma once

#include "../screen_switcher.h"
#include "../urack_types.h"
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
        MENU_CLOCK,
        MENU_COUNT
    };

    MidiSettingsState* state;
    MenuItems current_item;
    uint8_t blink_counter;
    bool is_editing;

    void render(void);
    void render_menu(void);
    void handle_input(Event* event);
    void handle_menu_input(Event* event);
};
