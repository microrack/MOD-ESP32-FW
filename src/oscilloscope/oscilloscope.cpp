#include "oscilloscope.h"
#include <math.h>

// Initialize static members
volatile bool OscilloscopeRoot::adc_conversion_done = false;
OscilloscopeRoot* OscilloscopeRoot::instance = nullptr;

// ISR Callback function for ADC Continuous
void ARDUINO_ISR_ATTR OscilloscopeRoot::adcCompleteCallback() {
    // Set flag for debugging
    adc_conversion_done = true;
    
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (instance && instance->adcCompleteSemaphore) {
        xSemaphoreGiveFromISR(instance->adcCompleteSemaphore, &xHigherPriorityTaskWoken);
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

OscilloscopeRoot::OscilloscopeRoot(Display* display) : ScreenInterface(display) {
    // Store instance for callback
    instance = this;
    
    // Create synchronization primitives
    acquisitionSemaphore = xSemaphoreCreateBinary();
    adcCompleteSemaphore = xSemaphoreCreateBinary();
    bufferMutex = xSemaphoreCreateMutex();
    
    // Initialize buffer with zeros
    for (int i = 0; i < BUFFER_SIZE; i++) {
        signal_buffer[i] = 0;
        adc_readings[i] = 0;
    }
    
    // Initialize ADC
    setupADC();
    
    // Create ADC reading task
    xTaskCreate(
        adcTaskFunction,         // Function that implements the task
        "ADC_TASK",              // Text name for the task
        2048,                    // Stack size in words, not bytes
        this,                    // Parameter passed into the task
        1,                       // Priority at which the task is created
        &adcTaskHandle           // Used to pass out the created task's handle
    );
}

void OscilloscopeRoot::adcTaskFunction(void* pvParameters) {
    OscilloscopeRoot* self = static_cast<OscilloscopeRoot*>(pvParameters);
    
    while (true) {
        // Wait for signal to start acquisition
        xSemaphoreTake(self->acquisitionSemaphore, portMAX_DELAY);
        
        // Check if we should exit
        if (!self->isRunning) {
            break;
        }
        
        // Configure ADC Continuous mode for 16 samples per acquisition
        uint8_t pins[] = {self->adc_pin};
        // Use the callback and constant sampling frequency of 100kHz
        bool configResult = analogContinuous(pins, 1, 16, SAMPLING_FREQ_HZ, &adcCompleteCallback);
        
        if (!configResult) {
            continue;
        }
        
        // Start ADC conversions
        bool startResult = analogContinuousStart();
        
        if (!startResult) {
            continue;
        }
        
        // Decimation counter and factor
        uint16_t decimation_counter = 0;
        uint16_t decimation_factor = self->getDecimationFactor();
        uint8_t current_scale = self->current_scale_index;
        
        // Keep sampling while isRunning is true
        while (self->isRunning) {
            // Check if time scale has changed
            if (current_scale != self->current_scale_index) {
                // Update the decimation factor
                current_scale = self->current_scale_index;
                decimation_factor = self->getDecimationFactor();
                
                // Reset counters and buffer on scale change
                if (xSemaphoreTake(self->bufferMutex, 10 / portTICK_PERIOD_MS) == pdTRUE) {
                    self->adc_read_index = 0;
                    for (int i = 0; i < BUFFER_SIZE; i++) {
                        self->signal_buffer[i] = 0;
                        self->adc_readings[i] = 0;
                    }
                    xSemaphoreGive(self->bufferMutex);
                }
                
                decimation_counter = 0;
            }
            
            // Wait for ADC completion semaphore
            if (xSemaphoreTake(self->adcCompleteSemaphore, pdMS_TO_TICKS(100)) == pdTRUE) {
                // Poll for ADC data
                adc_continuous_data_t* result = NULL;
                
                // Try to read data
                if (analogContinuousRead(&result, 0)) {
                    // Increment decimation counter
                    decimation_counter++;
                    
                    // Only store samples when decimation counter hits the factor
                    if (decimation_counter >= decimation_factor) {
                        decimation_counter = 0;
                        
                        // Take mutex before modifying the buffer
                        if (xSemaphoreTake(self->bufferMutex, 10 / portTICK_PERIOD_MS) == pdTRUE) {
                            // Store raw reading
                            self->adc_readings[self->adc_read_index] = result[0].avg_read_raw;
                            
                            // Scale reading to range -32 to 32 for display
                            self->signal_buffer[self->adc_read_index] = map(result[0].avg_read_raw, 0, 4095, -32, 32);
                            
                            // Move to next position in circular buffer
                            self->adc_read_index = (self->adc_read_index + 1) % BUFFER_SIZE;
                            
                            // Release mutex
                            xSemaphoreGive(self->bufferMutex);
                        }
                    }
                }
            } else {
                // Timeout occurred, check if we should restart ADC
                if (self->isRunning) {
                    // Try to restart ADC
                    analogContinuousStop();
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

void OscilloscopeRoot::setupADC() {
    // Set the ADC resolution
    analogReadResolution(12);
    
    // Set the attenuation to allow for a wider input voltage range
    analogSetPinAttenuation(adc_pin, ADC_11db);
    
    // Initialize buffer with zeros
    for (int i = 0; i < BUFFER_SIZE; i++) {
        signal_buffer[i] = 0;
        adc_readings[i] = 0;
    }
    adc_read_index = 0;
}

// Calculate the decimation factor based on the current time scale
uint16_t OscilloscopeRoot::getDecimationFactor() {
    // Calculate time per sample in microseconds at 100kHz
    // 1 / 100000 = 10 microseconds per sample
    const float us_per_sample = 10.0;
    
    // Calculate how many microseconds each division represents
    float us_per_division = time_scales[current_scale_index] * 1000.0; // convert ms to us
    
    // Calculate samples per division
    float samples_per_division = us_per_division / us_per_sample;
    
    // Assuming we want about 32 pixels per division
    float target_pixels_per_division = 32.0;
    
    // Calculate decimation factor (how many samples to skip)
    uint16_t decimation = max((uint16_t)(samples_per_division / target_pixels_per_division), (uint16_t)1);
    
    return decimation;
}

// Calculate how many samples should be visible based on current time scale
uint16_t OscilloscopeRoot::getVisibleSamples() {
    // Get display width
    const int width = display->width();
    
    // With decimation in acquisition, we're already storing only the samples we need
    // So visible samples is just the display width, or the buffer size if smaller
    return min((uint16_t)width, (uint16_t)BUFFER_SIZE);
}

void OscilloscopeRoot::drawGraph() {
    const int height = display->height();
    const int width = display->width();
    const int midY = height / 2;
    
    // Take mutex before accessing the buffer for drawing
    if (xSemaphoreTake(bufferMutex, 10 / portTICK_PERIOD_MS) != pdTRUE) {
        // If we can't get the mutex, just return - we'll try again on next update
        return;
    }
    
    // Get the current read index (protected by mutex)
    uint16_t current_index = adc_read_index;
    
    // Release mutex as soon as possible
    xSemaphoreGive(bufferMutex);
    
    // Calculate the offset for scrolling based on read position
    int tickOffset = current_index % 4; // 4 is the spacing between dots
    
    // Draw scrolling dotted line at the middle (y = 0)
    for (int x = width - tickOffset; x >= 0; x -= 4) {
        display->drawPixel(x, midY, SSD1306_WHITE);
    }
    // Draw dots to the right of the starting point too
    for (int x = width - tickOffset + 4; x < width; x += 4) {
        display->drawPixel(x, midY, SSD1306_WHITE);
    }
    
    // Draw scrolling tick marks on the x-axis every 32 pixels
    const int TICK_SPACING = 32;
    const int TICK_SIZE = 6; // 6 pixels tall (3 above, 3 below)
    
    // Calculate the offset for scrolling ticks based on read position
    tickOffset = current_index % TICK_SPACING;
    
    // Draw ticks at positions that scroll with the signal
    for (int x = width - tickOffset; x >= 0; x -= TICK_SPACING) {
        for (int y = midY - TICK_SIZE/2; y <= midY + TICK_SIZE/2; y++) {
            display->drawPixel(x, y, SSD1306_WHITE);
        }
    }
    
    // Also draw ticks to the right of the starting point
    for (int x = width - tickOffset + TICK_SPACING; x < width; x += TICK_SPACING) {
        for (int y = midY - TICK_SIZE/2; y <= midY + TICK_SIZE/2; y++) {
            display->drawPixel(x, y, SSD1306_WHITE);
        }
    }
    
    // Calculate decimation factor for display
    uint16_t decimation = getDecimationFactor();
    
    // Take mutex again before reading the buffer for drawing the graph
    if (xSemaphoreTake(bufferMutex, 10 / portTICK_PERIOD_MS) != pdTRUE) {
        // If we can't get the mutex, just return
        return;
    }
    
    // Draw the graph using the already decimated data
    // Since we're decimating during acquisition, each sample in the buffer
    // corresponds directly to one pixel on screen
    for (int x = 0; x < min((int)width, (int)BUFFER_SIZE); x++) {
        // Calculate the buffer index for this screen position
        // Start from the most recent sample and go back in time
        int idx = (current_index - x - 1 + BUFFER_SIZE) % BUFFER_SIZE;
        
        // Convert the value to display coordinates
        // Map from -32...32 to screen height with middle point as zero
        int y = midY - signal_buffer[idx] * (height / 2) / 32;
        
        // Ensure y is within display bounds
        y = constrain(y, 0, height - 1);
        
        // Draw the point with the specified width
        for (int w = 0; w < GRAPH_TRACE_WIDTH; w++) {
            int drawY = y - (GRAPH_TRACE_WIDTH / 2) + w;
            if (drawY >= 0 && drawY < height) {
                display->drawPixel(x, drawY, SSD1306_WHITE);
            }
        }
    }
    
    // Release mutex
    xSemaphoreGive(bufferMutex);
    
    // Show current time scale in the label
    String time_label = String(time_scales[current_scale_index], 0) + "ms/div";
    
    // Handle scales less than 1ms or greater than 1000ms
    if (time_scales[current_scale_index] < 1) {
        // Convert to μs
        time_label = String(time_scales[current_scale_index] * 1000, 0) + "us/div";
    } else if (time_scales[current_scale_index] >= 1000) {
        // Convert to seconds
        time_label = String(time_scales[current_scale_index] / 1000.0, 1) + "s/div";
    }
    
    // Add decimation info to the label
    String full_label = time_label + " D:" + String(decimation);
    
    // Draw label with inverted colors (white background, black text)
    const int LABEL_WIDTH = 74;  // Adjusted width for decimation info
    const int LABEL_HEIGHT = 8;  // Standard text height
    const int LABEL_X = 2;       // Left margin
    const int LABEL_Y = 2;       // Top margin
    
    // Draw white background rectangle
    display->fillRect(LABEL_X, LABEL_Y, LABEL_WIDTH, LABEL_HEIGHT, SSD1306_WHITE);
    
    // Draw text in black
    display->setTextColor(SSD1306_BLACK);
    display->setTextSize(1);
    display->setCursor(LABEL_X + 2, LABEL_Y);
    display->print(full_label);
    
    // Reset text color to white for any future drawing
    display->setTextColor(SSD1306_WHITE);
}

void OscilloscopeRoot::enter() {
    display->clearDisplay();
    
    // Start the ADC acquisition task
    isRunning = true;
    xSemaphoreGive(acquisitionSemaphore);
    
    drawGraph();
    display->display();
}

void OscilloscopeRoot::exit() {
    // Stop the ADC acquisition task
    isRunning = false;
    
    display->clearDisplay();
    display->display();
}

void OscilloscopeRoot::update(Event* event) {
    if (event == nullptr) return;

    // Clear the display for redrawing
    display->clearDisplay();
    
    // Handle encoder changes to adjust time scale
    if (event->encoder != 0) {
        // Decrease index (faster time scale) when turned clockwise
        if (event->encoder > 0 && current_scale_index > 0) {
            current_scale_index--;
        }
        // Increase index (slower time scale) when turned counter-clockwise
        else if (event->encoder < 0 && current_scale_index < TIME_SCALE_COUNT - 1) {
            current_scale_index++;
        }
    }
    
    // Draw the graph on each update
    drawGraph();

    // Handle button events
    switch (event->button_a) {
        case ButtonPress:
            // Handle button A press
            break;
        case ButtonRelease:
            // Handle button A release
            break;
        default:
            break;
    }

    switch (event->button_sw) {
        case ButtonPress:
            // Handle encoder switch press
            break;
        case ButtonRelease:
            // Handle encoder switch release
            break;
        default:
            break;
    }
    
    // Update display after handling events
    display->display();
} 