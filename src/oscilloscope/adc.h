#pragma once

#include <Arduino.h>
// Use newer ESP32 ADC APIs instead of deprecated ones
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_continuous.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

class Adc {
public:
    Adc();
    ~Adc();

    // Start ADC sampling with specified time between samples (in microseconds)
    bool start(int sample_us);
    
    // Stop ADC sampling
    void stop();
    
    // Read sample at current index
    int8_t read(size_t idx);
    
    // Get current write index
    size_t getIndex();
    
    static const size_t BUFFER_SIZE = 1024;

private:
    // Callback for ADC completion
    static void ARDUINO_ISR_ATTR adcCompleteCallback();
    
    // Setup ADC hardware
    void setupADC();
    
    // Static instance for the callback
    static Adc* instance;
    
    // ADC pin
    const uint8_t adc_pin = 36;  // Changed from 33 to 36 to match original code
    
    // ADC sampling rate in Hz
    static const uint32_t SAMPLING_FREQ_HZ = 50000;  // 50 kHz
    
    // Internal buffer size
    static const size_t INTERNAL_BUFFER_SIZE = 500;
    
    // Sample buffer (stores values from -32 to +32)
    int8_t buffer[BUFFER_SIZE];
    volatile size_t index;
    
    // Mutex for buffer protection
    SemaphoreHandle_t bufferMutex;
    
    // Running flag
    volatile bool isRunning;
    
    // Storage for the requested sample period in microseconds
    int samplePeriod;
    
    // Counter for skipping conversions when sample period > conversion time
    volatile int skipCounter;
}; 