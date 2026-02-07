#include "midi.h"
#include "midi_info.h"
#include "ble_midi.h"
#include "../util.h"

// bluetooth rune for connection indicator in the header
static const uint8_t BLUETOOTH_ICON[] PROGMEM = { 0x1, 0x0, 0x1, 0x80, 0x1, 0x40, 0x11, 0x20, 0x9, 0x10, 0x5, 0x20, 0x3, 0x40, 0x1, 0x80, 0x1, 0x80, 0x3, 0x40, 0x5, 0x20, 0x9, 0x10, 0x11, 0x20, 0x1, 0x40, 0x1, 0x80, 0x1, 0x0 };
MidiInfo::MidiInfo(Display *display, MidiSettingsState *state, SignalProcessor *processor, ScreenSwitcher *screen_switcher)
    : ScreenInterface(display), state(state), processor(processor), screen_switcher(screen_switcher)
{
    // Initialize any specific properties
}

void MidiInfo::set_screen_switcher(ScreenSwitcher *screen_switcher)
{
    this->screen_switcher = screen_switcher;
}

void MidiInfo::enter()
{
}

void MidiInfo::exit()
{
}

void MidiInfo::render()
{
    display->clearDisplay();
    display->setTextSize(1);
    display->setTextColor(SSD1306_WHITE);
    display->setCursor(0, 0);

    char buffer[32];
    display->setTextSize(2);
    sprintf(buffer, "BPM: %s", state->get_bpm_str());
    display->println(buffer);
    display->setTextSize(1);

    bool ble_enabled = state->get_bluetooth_enabled();
    bool ble_connected = ble_midi.is_connected();
    bool blink_on = ((millis() / 500) % 2) == 0; // simple 2Hz blink

    // Right-align a bluetooth icon; blink when enabled but not yet connected
    if (ble_enabled && (ble_connected || blink_on))
    {
        display->drawBitmap(SCREEN_WIDTH - 16, 0, BLUETOOTH_ICON, 16, 16, SSD1306_WHITE);
    }

    sprintf(buffer, "Ch: %s  Clk: %s",
            state->get_midi_channel_str(),
            state->get_midi_clk_type_str());
    display->println(buffer);
    display->print("         ");
    display->print(processor->last_out[OutChannelClk] > 0 ? "[CLK]" : " CLK ");
    display->print(" ");
    display->println(processor->last_out[OutChannelRst] > 0 ? "[RST]" : " RST ");

    sprintf(buffer, "A: %s %d", state->get_midi_out_type_str(OutChannelA), processor->last_out[OutChannelA]);
    display->println(buffer);
    sprintf(buffer, "B: %s %d", state->get_midi_out_type_str(OutChannelB), processor->last_out[OutChannelB]);
    display->println(buffer);
    sprintf(buffer, "C: %s %d", state->get_midi_out_type_str(OutChannelC), processor->last_out[OutChannelC]);
    display->println(buffer);

    display->display();
}

void MidiInfo::handle_input(Event *event)
{
    if (event == nullptr)
        return;

    if (event->encoder != 0)
    {
        state->set_bpm(clampi(state->get_bpm() + event->encoder,
                              state->get_min_bpm(),
                              state->get_max_bpm()));
        state->store(); // TODO: delay before storing for saving FLASH
    }

    if (event->button_sw == ButtonPress)
    {
        screen_switcher->set_screen(MidiScreen::MidiScreenSettings);
    }
}

void MidiInfo::update(Event *event)
{
    handle_input(event);
    render();
}
