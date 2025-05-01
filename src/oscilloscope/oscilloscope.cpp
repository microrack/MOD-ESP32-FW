#include "oscilloscope.h"
#include <math.h>

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
}

void OscilloscopeRoot::readADC() {
    // Read from ADC
    uint32_t adc_reading = adc1_get_raw(adc_channel);
    
    // Store raw reading
    adc_readings[adc_read_index] = adc_reading;
    
    // Scale reading to range -32 to 32 for display
    signal_buffer[adc_read_index] = map(adc_reading, 0, 4095, -32, 32);
    
    // Move to next position in circular buffer
    adc_read_index = (adc_read_index + 1) % BUFFER_SIZE;
}

void OscilloscopeRoot::drawGraph() {
    const int height = display->height();
    const int width = display->width();
    const int midY = height / 2;
    
    // Calculate the offset for scrolling based on read position
    int tickOffset = adc_read_index % 4; // 4 is the spacing between dots
    
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
    tickOffset = adc_read_index % TICK_SPACING;
    
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
    
    // Draw the graph
    for (int x = 0; x < min((int)BUFFER_SIZE, width); x++) {
        // Calculate the buffer index for this screen position
        int idx = (adc_read_index - width + x + BUFFER_SIZE) % BUFFER_SIZE;
        
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
}

void OscilloscopeRoot::enter() {
    display->clearDisplay();
    setupADC();
    drawGraph();
    display->display();
}

void OscilloscopeRoot::exit() {
    display->clearDisplay();
    display->display();
}

void OscilloscopeRoot::update(Event* event) {
    if (event == nullptr) return;

    // Clear the display for redrawing
    display->clearDisplay();
    
    // Read ADC value
    readADC();
    
    // Draw the graph on each update
    drawGraph();

    // Handle encoder changes
    if (event->encoder != 0) {
        // Process encoder movement
    }

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