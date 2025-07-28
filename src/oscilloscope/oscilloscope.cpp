#include "oscilloscope.h"
#include "../board.h"
#include <math.h>
#include <soc/adc_channel.h>


bool OscilloscopeRoot::is_rolling(size_t scale_index) {
    return time_scales[scale_index] > 100;
}

uint16_t OscilloscopeRoot::scale_to_rate(size_t scale_index) {
    return (uint16_t)(1.0 / (time_scales[scale_index] * 0.001 / TICK_SPACING));
}

OscilloscopeRoot::OscilloscopeRoot(Display* display) : ScreenInterface(display) {
    signal_config.channel_count = 1;
    signal_config.channels[0] = static_cast<adc_channel_t>(ADC1_GPIO36_CHANNEL);
    signal_config.trigger_mode = TriggerMode::FREE;
    signal_config.trigger_level = 0;
    signal_config.sampling_rate = 100;

    sigscoper.begin();
}

void OscilloscopeRoot::drawGraph() {
    /*
    int tickOffset = 0;
    const int height = display->height();
    const int fullWidth = display->width();
    // Fix the drawing width to 125 pixels
    const int width = 125;
    const int midY = height / 2;

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
    
    // Show current time scale in the label
    
    
    // Handle scales less than 1ms or greater than 1000ms
    if (time_scales[current_scale_index] < 1) {
        // Convert to μs
        time_label = String(time_scales[current_scale_index] * 1000, 0) + "us/div";
    } else if (time_scales[current_scale_index] >= 1000) {
        // Convert to seconds
        time_label = String(time_scales[current_scale_index] / 1000.0, 1) + "s/div";
    }
    
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
    display->print(time_label);
    
    // Reset text color to white for any future drawing
    display->setTextColor(SSD1306_WHITE);
    */
    const int TICK_SIZE = 6; // 6 pixels tall (3 above, 3 below)

    static uint32_t last_trigger_wait = millis();

    if(!sigscoper.is_ready() && last_trigger_wait == 0) {
        last_trigger_wait = millis();
    }

    if(millis() - last_trigger_wait > 1000 || sigscoper.is_ready() || true) {
        sigscoper.get_stats(0, &stats);
        size_t _pos = 0;
        sigscoper.get_buffer(0, SCREEN_WIDTH, signal_buffer, &_pos);
        sigscoper.restart();
        last_trigger_wait = 0;
    }

    display->setCursor(0, 0);
    display->printf("%.1f %.1f %.1f %.1f Hz", 
        std::min(std::max(-9.0, stats.min_value / 1000.0), 9.0),
        std::min(std::max(-9.0, stats.avg_value / 1000.0), 9.0),
        std::min(std::max(-9.0, stats.max_value / 1000.0), 9.0),
        stats.frequency
    );

    int graph_y = 40;
    for (int i = 1; i < SCREEN_WIDTH - 2; i++) {
        int y1 = map(
            signal_buffer[i], 400, 2400, SCREEN_HEIGHT - 10, 10);
        int y2 = map(
            signal_buffer[i + 1], 400, 2400, SCREEN_HEIGHT - 10, 10);
        
        if (y1 >= 0 && y1 < SCREEN_HEIGHT && y2 >= 0 && y2 < SCREEN_HEIGHT) {
            // Draw line with 2-pixel width
            display->drawLine(i, y1, i + 1, y2, SSD1306_WHITE);
            display->drawLine(i, y1 + 1, i + 1, y2 + 1, SSD1306_WHITE);
        }
    }

    // Draw trigger level using dotted line
    int trigger_level =
        map(sigscoper.get_trigger_threshold(), 400, 2400, 64, 10);
    for(int i = 0; i < SCREEN_WIDTH; i += 2) {
        display->drawPixel(i, trigger_level, SSD1306_WHITE);
    }

    // Draw trigger position using dotted line
    for(int i = 10; i < SCREEN_HEIGHT; i += 2) {
        display->drawPixel(SCREEN_WIDTH / 2, i, SSD1306_WHITE);
    }
    
    display->setCursor(0, SCREEN_HEIGHT - 10);
    display->printf("%.2f ms/div %d Hz",
        time_scales[current_scale_index],
        scale_to_rate(current_scale_index)
    );

    if(!is_rolling(current_scale_index)) {
        const int midY = SCREEN_HEIGHT / 2;
        for (int x = SCREEN_WIDTH - 2; x >= 0; x -= TICK_SPACING) {
            for (int y = midY - TICK_SIZE/2; y <= midY + TICK_SIZE/2; y++) {
                display->drawPixel(x, y, SSD1306_WHITE);
            }
        }
    }
}

void OscilloscopeRoot::enter() {

    if (!sigscoper.start(signal_config)) {
        Serial.println("Failed to start signal monitoring");
    }

    display->setTextSize(1);
    display->setTextColor(SSD1306_WHITE);

    display->clearDisplay();
    
    drawGraph();
    display->display();
}

void OscilloscopeRoot::exit() {
    sigscoper.stop();

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

        signal_config.sampling_rate = scale_to_rate(current_scale_index);
        signal_config.trigger_mode = is_rolling(current_scale_index)
            ? TriggerMode::FREE
            : TriggerMode::AUTO_RISE;
        sigscoper.stop();
        sigscoper.start(signal_config);
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