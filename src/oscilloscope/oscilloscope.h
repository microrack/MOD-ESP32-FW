#pragma once

#include "../urack_types.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

class OscilloscopeRoot : public ScreenInterface {
public:
    OscilloscopeRoot(Display* display) : ScreenInterface(display) {}
    void enter() override;
    void exit() override;
    void update(Event* event) override;

private:
    static const uint8_t BUFFER_SIZE = 128;
    static const uint8_t GRAPH_TRACE_WIDTH = 2; // Width of the graph trace in pixels
    int8_t signal_buffer[BUFFER_SIZE];
    void setupADC();
    void readADC();
    void drawGraph();
    
    // ADC configuration
    adc1_channel_t adc_channel = ADC1_CHANNEL_0; // ADC_0
    esp_adc_cal_characteristics_t adc_chars;
    uint16_t adc_readings[BUFFER_SIZE];
    uint8_t adc_read_index = 0;
}; 