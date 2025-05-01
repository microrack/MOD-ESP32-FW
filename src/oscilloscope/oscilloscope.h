#pragma once

#include "../urack_types.h"

class OscilloscopeRoot : public ScreenInterface {
public:
    OscilloscopeRoot(Display* display) : ScreenInterface(display) {}
    void enter() override;
    void exit() override;
    void update(Event* event) override;

private:
    static const uint8_t BUFFER_SIZE = 128;
    static const uint8_t GRAPH_TRACE_WIDTH = 1; // Width of the graph trace in pixels
    int8_t sine_buffer[BUFFER_SIZE];
    void initSineBuffer();
    void drawGraph();
}; 