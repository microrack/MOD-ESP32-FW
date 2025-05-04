#pragma once

#include "../urack_types.h"
#include "adc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

class OscilloscopeRoot : public ScreenInterface {
public:
    OscilloscopeRoot(Display* display);
    void enter() override;
    void exit() override;
    void update(Event* event) override;

private:
    // Buffer size for drawing on screen
    static const uint16_t BUFFER_SIZE = 1024;
    static const uint8_t GRAPH_TRACE_WIDTH = 2; // Width of the graph trace in pixels
    int8_t signal_buffer[BUFFER_SIZE];
    void drawGraph();
    
    // Timing variables
    // Time scales in milliseconds per division
    static const uint8_t TIME_SCALE_COUNT = 11;
    const float time_scales[TIME_SCALE_COUNT] = {0.25, 0.5, 1, 2.5, 5, 10, 25, 50, 100, 250, 500};
    uint8_t current_scale_index = 5; // Default to 10ms/div
    
    // Decimation parameters for different time scales
    uint16_t getDecimationFactor();
    uint16_t getVisibleSamples();
    
    // ADC handler
    Adc adc;
    
    // Mutex for buffer protection
    SemaphoreHandle_t bufferMutex = NULL;
    
    // State control
    volatile bool isRunning = false;
    
    // Sample period calculation for time scale
    int calculateSamplePeriod();
    
    // Last write position in the buffer
    volatile uint16_t buffer_pos = 0;
}; 