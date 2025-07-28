#pragma once

#include "sigscoper.h"
#include "../urack_types.h"

class OscilloscopeRoot : public ScreenInterface {
public:
    OscilloscopeRoot(Display* display);
    void enter() override;
    void exit() override;
    void update(Event* event) override;

private:
    // Buffer size for drawing on screen
    static const uint16_t BUFFER_SIZE = 128;
    static const uint8_t GRAPH_TRACE_WIDTH = 2; // Width of the graph trace in pixels
    uint16_t signal_buffer[BUFFER_SIZE];

    void drawGraph();
    
    // Timing variables
    // Time scales in milliseconds per division
    static const uint8_t TIME_SCALE_COUNT = 11;
    const float time_scales[TIME_SCALE_COUNT] = {0.25, 0.5, 1, 2.5, 5, 10, 25, 50, 100, 250, 500};
    uint8_t current_scale_index = 5; // Default to 10ms/div
    
    // Crosshair scrolling variables
    uint32_t crosshair_offset = 0;
    uint32_t last_crosshair_update = 0;
    static const uint32_t CROSSHAIR_UPDATE_RATE = 50; // Update every 50ms

    Sigscoper sigscoper;
    SigscoperConfig signal_config;
    SigscoperStats stats;
}; 