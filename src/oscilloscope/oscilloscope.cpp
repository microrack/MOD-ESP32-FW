#include "oscilloscope.h"
#include <math.h>

OscilloscopeRoot::OscilloscopeRoot(Display* display) : ScreenInterface(display) {
    // Create synchronization primitives
    acquisitionSemaphore = xSemaphoreCreateBinary();
    bufferMutex = xSemaphoreCreateMutex();
    
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
        
        // Keep sampling while isRunning is true
        while (self->isRunning) {
            // Read from ADC
            uint32_t adc_reading = adc1_get_raw(self->adc_channel);
            
            // Take mutex before modifying the buffer
            if (xSemaphoreTake(self->bufferMutex, 10 / portTICK_PERIOD_MS) == pdTRUE) {
                // Store raw reading
                self->adc_readings[self->adc_read_index] = adc_reading;
                
                // Scale reading to range -32 to 32 for display
                self->signal_buffer[self->adc_read_index] = map(adc_reading, 0, 4095, -32, 32);
                
                // Move to next position in circular buffer
                self->adc_read_index = (self->adc_read_index + 1) % BUFFER_SIZE;
                
                // Release mutex
                xSemaphoreGive(self->bufferMutex);
            }
            
            // Delay for the appropriate sample interval
            vTaskDelay(pdMS_TO_TICKS(self->sample_interval_ms));
        }
    }
    
    // Task will delete itself
    vTaskDelete(NULL);
}

void OscilloscopeRoot::setupADC() {
    // Configure ADC
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(adc_channel, ADC_ATTEN_DB_12);
    
    // Characterize ADC
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_12, ADC_WIDTH_BIT_12, 1100, &adc_chars);
    
    // Initialize buffer with zeros
    for (int i = 0; i < BUFFER_SIZE; i++) {
        signal_buffer[i] = 0;
        adc_readings[i] = 0;
    }
    
    adc_read_index = 0;
    updateSampleInterval();
}

void OscilloscopeRoot::updateSampleInterval() {
    // With proper task delay, we can directly set the interval based on the time scale
    // A reasonable sampling rate would be about 8 samples per division
    sample_interval_ms = time_scales[current_scale_index] / 8.0;
    
    // Ensure we don't have a zero delay (minimum 1ms)
    if (sample_interval_ms < 1) {
        sample_interval_ms = 1;
    }

    Serial.printf("Sample interval: %dms\n", sample_interval_ms);
    
    // Clear the buffer when scale changes
    if (xSemaphoreTake(bufferMutex, 10 / portTICK_PERIOD_MS) == pdTRUE) {
        // Reset index and clear buffer
        adc_read_index = 0;
        
        // Initialize buffer with zeros
        for (int i = 0; i < BUFFER_SIZE; i++) {
            signal_buffer[i] = 0;
            adc_readings[i] = 0;
        }
        
        // Release mutex
        xSemaphoreGive(bufferMutex);
    }
}

void OscilloscopeRoot::readADC() {
    // This method is no longer needed as ADC reading happens in a separate thread
    // But we'll keep it for compatibility, it just doesn't do anything anymore
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
    uint8_t current_index = adc_read_index;
    
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
    
    // Take mutex again before reading the buffer for drawing the graph
    if (xSemaphoreTake(bufferMutex, 10 / portTICK_PERIOD_MS) != pdTRUE) {
        // If we can't get the mutex, just return
        return;
    }
    
    // Draw the graph
    for (int x = 0; x < min((int)BUFFER_SIZE, width); x++) {
        // Calculate the buffer index for this screen position
        int idx = (current_index - width + x + BUFFER_SIZE) % BUFFER_SIZE;
        
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
    
    // Draw label with inverted colors (white background, black text)
    const int LABEL_WIDTH = 58;  // Adjust width as needed
    const int LABEL_HEIGHT = 8;  // Standard text height
    const int LABEL_X = 2;       // Left margin
    const int LABEL_Y = 2;       // Top margin
    
    // Draw white background rectangle
    // display->fillRect(LABEL_X, LABEL_Y, LABEL_WIDTH, LABEL_HEIGHT, SSD1306_WHITE);
    
    // Draw text in black
    display->setTextColor(SSD1306_BLACK);
    display->setTextSize(1);
    display->setCursor(LABEL_X + 2, LABEL_Y);
    // display->print(time_label);
    
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
            updateSampleInterval();
        }
        // Increase index (slower time scale) when turned counter-clockwise
        else if (event->encoder < 0 && current_scale_index < TIME_SCALE_COUNT - 1) {
            current_scale_index++;
            updateSampleInterval();
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