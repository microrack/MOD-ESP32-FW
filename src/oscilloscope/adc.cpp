#include "adc.h"

// Initialize static members
Adc* Adc::instance = nullptr;

// ISR Callback function for ADC Continuous
void ARDUINO_ISR_ATTR Adc::adcCompleteCallback() {
    // Ensure instance exists
    if (!instance || !instance->isRunning) {
        return;
    }
    
    // Poll for ADC data
    adc_continuous_data_t* result = NULL;
    
    // Try to read data
    if (analogContinuousRead(&result, 0)) {
        // Calculate conversion time in microseconds
        const float conversion_time_us = (INTERNAL_BUFFER_SIZE * 1000000.0f) / SAMPLING_FREQ_HZ;
        
        // Handle skipping if needed
        if (instance->samplePeriod > conversion_time_us) {
            // Use a local variable instead of directly incrementing volatile
            int currentSkipCount = instance->skipCounter;
            currentSkipCount++;
            instance->skipCounter = currentSkipCount;
            
            // Calculate skip count (same as in start method)
            int skip_count = (instance->samplePeriod / conversion_time_us) - 1;
            
            // Only process every (skip_count+1)th conversion
            if (instance->skipCounter < skip_count) {
                return; // Skip this conversion
            }
            
            // Reset counter
            instance->skipCounter = 0;
        }
        
        // We don't want to use a mutex in an ISR, so we'll use direct buffer access
        // This is safe because we're the only writer to the buffer
        
        // Calculate decimation based on sample period
        if (instance->samplePeriod > conversion_time_us) {
            // We're already skipping conversions, so just use the first value
            
            // Scale reading to fit in range -32 to +32 (for screen display)
            instance->buffer[instance->index] = map(result[0].avg_read_raw, 0, 4095, -32, 32);
            
            // Move to next position in circular buffer
            instance->index = (instance->index + 1) % BUFFER_SIZE;
        } else {
            // If sample_us < conversion_time_us, decimate internal buffer
            // Calculate how many samples to take from internal buffer (same calculation as in start)
            int samples_to_take = conversion_time_us / instance->samplePeriod;
            samples_to_take = constrain(samples_to_take, 1, INTERNAL_BUFFER_SIZE);
            
            // Stride through the internal buffer
            int stride = INTERNAL_BUFFER_SIZE / samples_to_take;
            stride = max(stride, 1);
            
            for (int i = 0; i < samples_to_take; i++) {
                int internal_idx = i * stride;
                if (internal_idx < INTERNAL_BUFFER_SIZE) {
                    // Scale reading to fit in range -32 to +32 (for screen display)
                    instance->buffer[instance->index] = map(result[internal_idx].avg_read_raw, 0, 4095, -32, 32);
                    
                    // Move to next position in circular buffer
                    instance->index = (instance->index + 1) % BUFFER_SIZE;
                }
            }
        }
    }
    
    // We don't need to yield because this is already an ISR
}

Adc::Adc() : index(0), isRunning(false), samplePeriod(0), skipCounter(0) {
    // Store instance for callback
    instance = this;
    
    // Create synchronization primitives for safe reading from the main task
    bufferMutex = xSemaphoreCreateMutex();
    
    // Initialize buffer with zeros
    for (int i = 0; i < BUFFER_SIZE; i++) {
        buffer[i] = 0;
    }
    
    // Initialize ADC
    setupADC();
}

Adc::~Adc() {
    // Stop ADC if running
    stop();
    
    // Clean up resources
    if (bufferMutex) {
        vSemaphoreDelete(bufferMutex);
    }
}

void Adc::setupADC() {
    // Set the ADC resolution
    analogReadResolution(12);
    
    // Set the attenuation to allow for a wider input voltage range
    analogSetPinAttenuation(adc_pin, ADC_11db);
}

bool Adc::start(int sample_us) {
    // If already running, stop first
    if (isRunning) {
        stop();
    }
    
    // Store the sample period
    samplePeriod = sample_us;
    skipCounter = 0;
    
    // Calculate conversion time in microseconds
    const float conversion_time_us = (INTERNAL_BUFFER_SIZE * 1000000.0f) / SAMPLING_FREQ_HZ;
    
    // Print mode information
    Serial.println("--------- ADC Configuration ---------");
    Serial.print("Sample period (us): ");
    Serial.println(sample_us);
    Serial.print("Conversion time (us): ");
    Serial.println(conversion_time_us);
    
    if (sample_us > conversion_time_us) {
        // Skip samples mode
        int skip_count = (sample_us / conversion_time_us) - 1;
        Serial.println("Mode: Skip samples");
        Serial.print("Samples to skip: ");
        Serial.println(skip_count);
    } else {
        // Decimation mode
        int samples_to_take = conversion_time_us / sample_us;
        samples_to_take = constrain(samples_to_take, 1, INTERNAL_BUFFER_SIZE);
        int stride = INTERNAL_BUFFER_SIZE / samples_to_take;
        stride = max(stride, 1);
        
        Serial.println("Mode: Decimation");
        Serial.print("Samples to take: ");
        Serial.println(samples_to_take);
        Serial.print("Stride factor: ");
        Serial.println(stride);
    }
    Serial.println("-----------------------------------");
    
    // Configure ADC Continuous mode for internal buffer size
    uint8_t pins[] = {adc_pin};
    // Use the callback and specified sampling frequency
    bool configResult = analogContinuous(pins, 1, INTERNAL_BUFFER_SIZE, SAMPLING_FREQ_HZ, &adcCompleteCallback);
    
    if (!configResult) {
        return false;
    }
    
    // Set running flag
    isRunning = true;
    
    // Start ADC conversions
    bool startResult = analogContinuousStart();
    
    if (!startResult) {
        isRunning = false;
        return false;
    }
    
    return true;
}

void Adc::stop() {
    // Stop ADC
    isRunning = false;
    
    // Deinitialize ADC
    analogContinuousStop();
    analogContinuousDeinit();
}

int8_t Adc::read(size_t idx) {
    // Take mutex before reading to ensure we don't read during an update
    if (xSemaphoreTake(bufferMutex, 10 / portTICK_PERIOD_MS) != pdTRUE) {
        return 0; // Return zero if can't acquire mutex
    }
    
    // Read value
    int8_t value = buffer[idx % BUFFER_SIZE];
    
    // Release mutex
    xSemaphoreGive(bufferMutex);
    
    return value;
}

size_t Adc::getIndex() {
    // Take mutex before reading
    if (xSemaphoreTake(bufferMutex, 10 / portTICK_PERIOD_MS) != pdTRUE) {
        return 0; // Return zero if can't acquire mutex
    }
    
    // Get current index
    size_t current_index = index;
    
    // Release mutex
    xSemaphoreGive(bufferMutex);
    
    return current_index;
} 