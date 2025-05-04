#include "oscilloscope.h"
#include <math.h>

OscilloscopeRoot::OscilloscopeRoot(Display* display) : ScreenInterface(display) {
    // Create buffer mutex
    bufferMutex = xSemaphoreCreateMutex();
    
    // Initialize buffer with zeros
    for (int i = 0; i < BUFFER_SIZE; i++) {
        signal_buffer[i] = 0;
    }
    
    buffer_pos = 0;
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
    
    // Assuming we want about 25 pixels per division (changed from 32)
    float target_pixels_per_division = 25.0;
    
    // Calculate decimation factor (how many samples to skip)
    uint16_t decimation = max((uint16_t)(samples_per_division / target_pixels_per_division), (uint16_t)1);
    
    return decimation;
}

// Calculate how many samples should be visible based on current time scale
uint16_t OscilloscopeRoot::getVisibleSamples() {
    // Fix the drawing width to 125 pixels
    const int width = 125;
    
    // Return the display width or buffer size, whichever is smaller
    return min((uint16_t)width, (uint16_t)BUFFER_SIZE);
}

// Calculate the sample period in microseconds based on the current time scale
int OscilloscopeRoot::calculateSamplePeriod() {
    // Calculate sample period from the current time scale
    // Time scale is in ms/div, convert to microseconds
    float us_per_division = time_scales[current_scale_index] * 1000.0;
    
    // Calculate appropriate sample period for the current time scale
    // We want approximately 25 samples per division (changed from 32)
    const float target_samples_per_division = 25.0;
    
    // Sample period = time per division / target samples per division
    int sample_period_us = (int)(us_per_division / target_samples_per_division);
    
    // Ensure minimum sample period is 10 microseconds (100kHz max)
    return max(sample_period_us, 10);
}

void OscilloscopeRoot::drawGraph() {
    const int height = display->height();
    const int fullWidth = display->width();
    // Fix the drawing width to 125 pixels
    const int width = 125;
    const int midY = height / 2;
    
    // Get current ADC index
    size_t current_index = adc.getIndex();
    
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
    
    // Draw scrolling tick marks on the x-axis every 25 pixels (changed from 32)
    const int TICK_SPACING = 25;
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
    
    // Draw the graph using data from the ADC buffer
    for (int x = 0; x < min((int)width, (int)Adc::BUFFER_SIZE); x++) {
        // Calculate the buffer index for this screen position
        // Start from the most recent sample and go back in time
        int idx = (current_index - x - 1 + Adc::BUFFER_SIZE) % Adc::BUFFER_SIZE;
        
        // Read value from ADC buffer
        int8_t value = adc.read(idx);
        
        // Convert the value to display coordinates
        // The ADC class already scales to int8_t range (-32...32)
        // Scale to screen height
        int y = midY - value * (height / 2) / 32;
        
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
    
    // Show current time scale in the label
    String time_label = String(time_scales[current_scale_index], 1) + "ms/div";
    
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
    
    // Start the ADC with appropriate sample period
    int sample_period_us = calculateSamplePeriod();
    adc.start(sample_period_us);
    
    isRunning = true;
    
    drawGraph();
    display->display();
}

void OscilloscopeRoot::exit() {
    // Stop the ADC
    adc.stop();
    
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
            
            // Update ADC sample rate based on new time scale
            int sample_period_us = calculateSamplePeriod();
            adc.stop();
            adc.start(sample_period_us);
        }
        // Increase index (slower time scale) when turned counter-clockwise
        else if (event->encoder < 0 && current_scale_index < TIME_SCALE_COUNT - 1) {
            current_scale_index++;
            
            // Update ADC sample rate based on new time scale
            int sample_period_us = calculateSamplePeriod();
            adc.stop();
            adc.start(sample_period_us);
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