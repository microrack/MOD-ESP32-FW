#include "screen_switcher.h"

ScreenSwitcher::ScreenSwitcher()
    : screens(nullptr), screen_count(0), current_index(0) {
}

ScreenSwitcher::ScreenSwitcher(ScreenInterface** screens, size_t screen_count)
    : screens(screens), screen_count(screen_count), current_index(0) {
}

size_t ScreenSwitcher::get_current() const {
    return current_index;
}

size_t ScreenSwitcher::get_next() const {
    if (screen_count == 0) {
        return 0;
    }
    return (current_index + 1) % screen_count;
}

void ScreenSwitcher::set_screen(size_t index) {
    if (screen_count == 0 || index >= screen_count) {
        return;
    }

    // Exit current screen
    screens[current_index]->exit();

    // Change index
    current_index = index;

    // Enter new screen
    screens[current_index]->enter();
}

void ScreenSwitcher::update(Event* event) {
    if (screen_count > 0) {
        screens[current_index]->update(event);
    }
}

ScreenInterface* ScreenSwitcher::get_current_screen() const {
    if (screen_count == 0) {
        return nullptr;
    }
    return screens[current_index];
}
