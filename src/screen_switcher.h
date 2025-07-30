#pragma once

#include "urack_types.h"

class ScreenSwitcher {
  public:
    // Default constructor
    ScreenSwitcher();

    ScreenSwitcher(ScreenInterface** screens, size_t screen_count);

    // Get the current screen index
    size_t get_current() const;

    // Get the next screen index
    size_t get_next() const;

    // Set the current screen to the given index
    void set_screen(size_t index);

    // Update the current screen with the given event
    void update(Event* event);

    // Get the current screen pointer
    ScreenInterface* get_current_screen() const;

  private:
    ScreenInterface** screens;
    size_t screen_count;
    size_t current_index;
};
