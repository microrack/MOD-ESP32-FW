#pragma once

#include "../urack_types.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
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
    
    // Timing variables
    // Time scales in milliseconds per division
    static const uint8_t TIME_SCALE_COUNT = 6;
    const float time_scales[TIME_SCALE_COUNT] = {10, 20, 50, 100, 200, 500};
    uint8_t current_scale_index = 4; // Default to 20ms/div
    float sample_interval_ms = 10.0;
    void updateSampleInterval();
    
    // Threading and synchronization
    static void adcTaskFunction(void* pvParameters);
    TaskHandle_t adcTaskHandle = NULL;
    SemaphoreHandle_t acquisitionSemaphore = NULL;
    SemaphoreHandle_t bufferMutex = NULL;
    volatile bool isRunning = false;
}; 
