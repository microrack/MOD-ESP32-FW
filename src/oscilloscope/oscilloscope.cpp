#include "oscilloscope.h"
#include <math.h>

void OscilloscopeRoot::initSineBuffer() {
    for (int i = 0; i < BUFFER_SIZE; i++) {
        // Generate sine wave values scaled to range -32 to +32
        float angle = (float)i / BUFFER_SIZE * 2.0 * PI;
        sine_buffer[i] = (int8_t)(sin(angle) * 32.0);
    }
}

void OscilloscopeRoot::drawGraph() {
    const int height = display->height();
    const int width = display->width();
    const int midY = height / 2;
    
    // Draw dotted line at the middle (y = 0)
    for (int x = 0; x < width; x += 4) {
        display->drawPixel(x, midY, SSD1306_WHITE);
    }
    
    // Draw tick marks on the x-axis every 32 pixels
    const int TICK_SPACING = 32;
    const int TICK_SIZE = 6; // 6 pixels tall (3 above, 3 below)
    for (int x = 0; x < width; x += TICK_SPACING) {
        for (int y = midY - TICK_SIZE/2; y <= midY + TICK_SIZE/2; y++) {
            display->drawPixel(x, y, SSD1306_WHITE);
        }
    }
    
    // Draw the graph
    for (int x = 0; x < min((int)BUFFER_SIZE, width); x++) {
        // Convert the sine value to display coordinates
        // Map from -32...32 to screen height with middle point as zero
        int y = midY - sine_buffer[x] * (height / 2) / 32;
        
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
    initSineBuffer();
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