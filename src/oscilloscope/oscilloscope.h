#pragma once

#include "../urack_types.h"
// Suppress the deprecation warnings but keep the original includes
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcpp"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#pragma GCC diagnostic pop
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
    // Increase buffer size to 1024 for higher resolution
    static const uint16_t BUFFER_SIZE = 1024;
    static const uint8_t GRAPH_TRACE_WIDTH = 2; // Width of the graph trace in pixels
    int8_t signal_buffer[BUFFER_SIZE];
    void setupADC();
    void drawGraph();
    
    // ADC configuration
    uint8_t adc_pin = 36; // Change to your specific ADC pin
    esp_adc_cal_characteristics_t adc_chars;
    uint16_t adc_readings[BUFFER_SIZE];
    uint16_t adc_read_index = 0;  // Changed to uint16_t to handle larger buffer
    
    // Timing variables
    // Time scales in milliseconds per division
    static const uint8_t TIME_SCALE_COUNT = 9;
    const float time_scales[TIME_SCALE_COUNT] = {1, 2, 5, 10, 20, 50, 100, 200, 500};
    uint8_t current_scale_index = 4; // Default to 20ms/div
    
    // Constant sampling frequency 100kHz
    static const uint32_t SAMPLING_FREQ_HZ = 100000;
    
    // Decimation parameters for different time scales
    uint16_t getDecimationFactor();
    uint16_t getVisibleSamples();
    
    // ADC Continuous mode with task
    static void ARDUINO_ISR_ATTR adcCompleteCallback();
    static void adcTaskFunction(void* pvParameters);
    TaskHandle_t adcTaskHandle = NULL;
    SemaphoreHandle_t acquisitionSemaphore = NULL;
    SemaphoreHandle_t adcCompleteSemaphore = NULL;
    SemaphoreHandle_t bufferMutex = NULL;
    static volatile bool adc_conversion_done;
    static OscilloscopeRoot* instance; // Static pointer to access from callback
    adc_continuous_data_t* adc_result = NULL;
    
    // State control
    volatile bool isRunning = false;
}; 