#include "adc.h"

// Initialize static members
Adc* Adc::instance = nullptr;

// ISR Callback function for ADC Continuous
void ARDUINO_ISR_ATTR Adc::adcCompleteCallback() {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (instance && instance->adcCompleteSemaphore) {
        xSemaphoreGiveFromISR(instance->adcCompleteSemaphore, &xHigherPriorityTaskWoken);
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

Adc::Adc() : index(0), isRunning(false), samplePeriod(0), skipCounter(0) {
    // Store instance for callback
    instance = this;
    
    // Create synchronization primitives
    adcCompleteSemaphore = xSemaphoreCreateBinary();
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
    if (adcCompleteSemaphore) {
        vSemaphoreDelete(adcCompleteSemaphore);
    }
    
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
    
    // Set running flag
    isRunning = true;
    
    // Create ADC reading task
    xTaskCreate(
        adcTaskFunction,         // Function that implements the task
        "ADC_TASK",              // Text name for the task
        2048,                    // Stack size in words, not bytes
        this,                    // Parameter passed into the task
        1,                       // Priority at which the task is created
        &adcTaskHandle           // Used to pass out the created task's handle
    );
    
    return true;
}

void Adc::stop() {
    // Stop task
    isRunning = false;
    
    // Wait for task to end
    if (adcTaskHandle != nullptr) {
        // Give some time for the task to clean up
        vTaskDelay(100 / portTICK_PERIOD_MS);
        
        // Delete the task handle
        adcTaskHandle = nullptr;
    }
}

int8_t Adc::read(size_t idx) {
    // Take mutex before reading
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

void Adc::adcTaskFunction(void* pvParameters) {
    Adc* self = static_cast<Adc*>(pvParameters);
    
    while (self->isRunning) {
        // Calculate ADC conversion time in microseconds (same as in start method)
        const float conversion_time_us = (INTERNAL_BUFFER_SIZE * 1000000.0f) / SAMPLING_FREQ_HZ;
        
        // Calculate how many conversions to skip if sample_us > conversion_time_us
        int skip_count = 0;
        if (self->samplePeriod > conversion_time_us) {
            skip_count = (self->samplePeriod / conversion_time_us) - 1;
        }
        
        // Configure ADC Continuous mode for internal buffer size
        uint8_t pins[] = {self->adc_pin};
        // Use the callback and specified sampling frequency
        bool configResult = analogContinuous(pins, 1, INTERNAL_BUFFER_SIZE, SAMPLING_FREQ_HZ, &adcCompleteCallback);
        
        if (!configResult) {
            vTaskDelay(100 / portTICK_PERIOD_MS);
            continue;
        }
        
        // Start ADC conversions
        bool startResult = analogContinuousStart();
        
        if (!startResult) {
            vTaskDelay(100 / portTICK_PERIOD_MS);
            continue;
        }
        
        // Keep sampling while isRunning is true
        while (self->isRunning) {
            // Wait for ADC completion semaphore
            if (xSemaphoreTake(self->adcCompleteSemaphore, pdMS_TO_TICKS(100)) == pdTRUE) {
                // If sample_us > conversion_time_us, we need to skip some conversions
                if (skip_count > 0) {
                    // Use a local variable instead of directly incrementing volatile
                    int currentSkipCount = self->skipCounter;
                    currentSkipCount++;
                    self->skipCounter = currentSkipCount;
                    
                    // Only process every (skip_count+1)th conversion
                    if (self->skipCounter < skip_count) {
                        continue; // Skip this conversion
                    }
                    
                    // Reset counter
                    self->skipCounter = 0;
                }
                
                // Poll for ADC data
                adc_continuous_data_t* result = NULL;
                
                // Try to read data
                if (analogContinuousRead(&result, 0)) {
                    // Take mutex before modifying the buffer
                    if (xSemaphoreTake(self->bufferMutex, 10 / portTICK_PERIOD_MS) == pdTRUE) {
                        // Calculate decimation based on sample period
                        if (self->samplePeriod > conversion_time_us) {
                            // We're already skipping conversions, so just use the first value
                            
                            // Scale reading to fit in range -32 to +32 (for screen display)
                            self->buffer[self->index] = map(result[0].avg_read_raw, 0, 4095, -32, 32);
                            
                            // Move to next position in circular buffer
                            self->index = (self->index + 1) % BUFFER_SIZE;
                        } else {
                            // If sample_us < conversion_time_us, decimate internal buffer
                            // Calculate how many samples to take from internal buffer (same calculation as in start)
                            int samples_to_take = conversion_time_us / self->samplePeriod;
                            samples_to_take = constrain(samples_to_take, 1, INTERNAL_BUFFER_SIZE);
                            
                            // Stride through the internal buffer
                            int stride = INTERNAL_BUFFER_SIZE / samples_to_take;
                            stride = max(stride, 1);
                            
                            for (int i = 0; i < samples_to_take; i++) {
                                int internal_idx = i * stride;
                                if (internal_idx < INTERNAL_BUFFER_SIZE) {
                                    // Scale reading to fit in range -32 to +32 (for screen display)
                                    self->buffer[self->index] = map(result[internal_idx].avg_read_raw, 0, 4095, -32, 32);
                                    
                                    // Move to next position in circular buffer
                                    self->index = (self->index + 1) % BUFFER_SIZE;
                                }
                            }
                        }
                        
                        // Release mutex
                        xSemaphoreGive(self->bufferMutex);
                    }
                }
            } else {
                // Timeout occurred, check if we should restart ADC
                if (self->isRunning) {
                    // Try to restart ADC
                    analogContinuousStop();
                    vTaskDelay(10 / portTICK_PERIOD_MS); // Add a delay to let things settle
                    analogContinuousStart();
                }
            }
        }
        
        // Stop and deinitialize ADC when exiting the loop
        analogContinuousStop();
        analogContinuousDeinit();
    }
    
    // Task will delete itself
    vTaskDelete(NULL);
} 