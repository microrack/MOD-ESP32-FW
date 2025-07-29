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
    signal_config.trigger_mode = is_rolling(current_scale_index)
        ? TriggerMode::FREE
        : TriggerMode::AUTO_RISE;
    signal_config.trigger_level = 1000;
    signal_config.sampling_rate = scale_to_rate(current_scale_index);
    signal_config.auto_speed = 0.005f;  // Default auto_speed value

    sigscoper.begin();
}

void OscilloscopeRoot::drawGraph() {
    const int TICK_SIZE = 6; // 6 pixels tall (3 above, 3 below)

    static uint32_t last_trigger_wait = millis();

    if(!sigscoper.is_ready() && last_trigger_wait == 0) {
        last_trigger_wait = millis();
    }

    if(millis() - last_trigger_wait > 1000
        || sigscoper.is_ready()
        || is_rolling(current_scale_index)) {
        sigscoper.get_stats(0, &stats);
        size_t _pos = 0;
        sigscoper.get_buffer(0, SCREEN_WIDTH, signal_buffer, &_pos);
        sigscoper.restart();
        last_trigger_wait = 0;
    }

    display->setCursor(0, 0);
    display->printf("%.1f %.1f ", 
        std::min(std::max(-9.0, stats.min_value / 1000.0), 9.0),
        std::min(std::max(-9.0, stats.max_value / 1000.0), 9.0)
    );

    if(stats.frequency >= 1000) {
        display->printf("%.1f kHz ", stats.frequency / 1000.0);
    } else {
        display->printf("%.0f Hz ", stats.frequency);
    }

    display->printf("%.0f %s/d",
        time_scales[current_scale_index] >= 1.0
            ? time_scales[current_scale_index]
            : time_scales[current_scale_index] * 1000.0,
        time_scales[current_scale_index] >= 1.0 ? "ms" : "us"
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

    /*
    // Draw trigger position using dotted line
    for(int i = 10; i < SCREEN_HEIGHT; i += 2) {
        display->drawPixel(SCREEN_WIDTH / 2, i, SSD1306_WHITE);
    }
    */

    const int midY = SCREEN_HEIGHT / 2;

    if(!is_rolling(current_scale_index)) {
        for (int x = SCREEN_WIDTH - 2; x >= 0; x -= TICK_SPACING) {
            for (int y = midY - TICK_SIZE/2; y <= midY + TICK_SIZE/2; y++) {
                display->drawPixel(x, y, SSD1306_WHITE);
            }
        }
    }

    for (int x = SCREEN_WIDTH - tickOffset; x >= 0; x -= 4) {
        display->drawPixel(x, midY, SSD1306_WHITE);
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
        
        // save last trigger level
        signal_config.trigger_level = sigscoper.get_trigger_threshold();

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