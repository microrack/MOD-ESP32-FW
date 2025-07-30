#pragma once

#include "../urack_types.h"
#include "sigscoper.h"

enum class DisplayMode {
    SINGLE,  // Only one channel shows
    JOINED,  // Two channels on single graph
    SPLIT    // Two channels on separate graphs
};

class OscilloscopeRoot : public ScreenInterface {
public:
    OscilloscopeRoot(Display* display);
    void enter() override;
    void exit() override;
    void update(Event* event) override;

private:
    // Buffer size for drawing on screen
    static const uint16_t BUFFER_SIZE = 128;
    static const uint8_t GRAPH_TRACE_WIDTH =
        2;  // Width of the graph trace in pixels
    uint16_t signal_buffer[BUFFER_SIZE];
    uint16_t signal_buffer2[BUFFER_SIZE];  // Buffer for second channel

    const int TICK_SPACING = 25;
    size_t tickOffset = 0;

    void drawGraph();
    bool is_rolling(size_t scale_index);
    uint16_t scale_to_rate(size_t scale_index);

    // Timing variables
    // Time scales in milliseconds per division
    static const uint8_t TIME_SCALE_COUNT = 11;
    const float time_scales[TIME_SCALE_COUNT] = {
        0.25, 0.5, 1, 2.5, 5, 10, 25, 50, 100, 250, 500};
    uint8_t current_scale_index = 5;  // Default to 10ms/div

    // Crosshair scrolling variables
    uint32_t crosshair_offset = 0;
    uint32_t last_crosshair_update = 0;
    static const uint32_t CROSSHAIR_UPDATE_RATE =
        50;  // Update every 50ms

    Sigscoper sigscoper;
    SigscoperConfig signal_config;
    SigscoperStats stats;
    DisplayMode display_mode = DisplayMode::JOINED;  // Default mode
};
