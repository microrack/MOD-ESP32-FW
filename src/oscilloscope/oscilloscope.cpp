#include "oscilloscope.h"
#include <math.h>
#include <soc/adc_channel.h>
#include "../board.h"

bool OscilloscopeRoot::is_rolling(size_t scale_index) {
    return time_scales[scale_index] > 100;
}

uint16_t OscilloscopeRoot::scale_to_rate(size_t scale_index) {
    return (uint16_t)(1.0 / (time_scales[scale_index] * 0.001 / TICK_SPACING));
}

OscilloscopeRoot::OscilloscopeRoot(Display* display) : ScreenInterface(display) {
    signal_config.channel_count = 2;
    signal_config.channels[0] = static_cast<adc_channel_t>(ADC1_GPIO36_CHANNEL);
    signal_config.channels[1] = static_cast<adc_channel_t>(ADC1_GPIO37_CHANNEL);

    signal_config.trigger_mode = is_rolling(current_scale_index) ? TriggerMode::FREE : TriggerMode::AUTO_RISE;
    signal_config.trigger_level = 1000;
    signal_config.sampling_rate = scale_to_rate(current_scale_index);
    signal_config.auto_speed = 0.005f;  // Default auto_speed value
    signal_config.buffer_size = TICK_SPACING * (SCREEN_WIDTH / TICK_SPACING);

    sigscoper.begin();
}

void OscilloscopeRoot::drawGraph() {
    const int TICK_SIZE = 6;  // 6 pixels tall (3 above, 3 below)

    static uint32_t last_trigger_wait = millis();

    if (!sigscoper.is_ready() && last_trigger_wait == 0) {
        last_trigger_wait = millis();
    }

    if (millis() - last_trigger_wait > 1000 || sigscoper.is_ready() || is_rolling(current_scale_index)) {
        sigscoper.get_stats(0, &stats);
        size_t _pos = 0;
        sigscoper.get_buffer(0, SCREEN_WIDTH, signal_buffer, &_pos);
        sigscoper.get_buffer(1, SCREEN_WIDTH, signal_buffer2, &_pos);
        sigscoper.restart();
        last_trigger_wait = 0;
    }

    display->setCursor(0, 0);

    display->printf("%.0f %s/d ",
                    time_scales[current_scale_index] >= 1.0 ? time_scales[current_scale_index]
                                                            : time_scales[current_scale_index] * 1000.0,
                    time_scales[current_scale_index] >= 1.0 ? "ms" : "us");

    if (display_mode == DisplayMode::SINGLE) {
        display->printf("| %.1f | %.1f ", std::min(std::max(-9.0, stats.min_value / 1000.0), 9.0),
                        std::min(std::max(-9.0, stats.max_value / 1000.0), 9.0));
    }

    int graph_y = 40;

    switch (display_mode) {
        case DisplayMode::SINGLE:
            // Draw only first channel (thick line - 3 pixels)
            for (int i = 1; i < SCREEN_WIDTH - 2; i++) {
                if (signal_buffer[i] == 0)
                    continue;
                if (signal_buffer[i + 1] == 0)
                    continue;

                int y1 = map(signal_buffer[i], 400, 2400, SCREEN_HEIGHT, 10);
                int y2 = map(signal_buffer[i + 1], 400, 2400, SCREEN_HEIGHT, 10);

                if (y1 >= 0 && y1 < SCREEN_HEIGHT && y2 >= 0 && y2 < SCREEN_HEIGHT) {
                    // Draw line with 3-pixel width
                    display->drawLine(i, y1, i + 1, y2, SSD1306_WHITE);
                    display->drawLine(i, y1 + 1, i + 1, y2 + 1, SSD1306_WHITE);
                    display->drawLine(i, y1 + 2, i + 2, y2 + 2, SSD1306_WHITE);
                }
            }
            break;

        case DisplayMode::JOINED:
            // Draw only first channel (thick line - 3 pixels)
            for (int i = 1; i < SCREEN_WIDTH - 2; i++) {
                if (signal_buffer[i] == 0)
                    continue;
                if (signal_buffer[i + 1] == 0)
                    continue;

                int y1 = map(signal_buffer[i], 400, 2400, SCREEN_HEIGHT, 10);
                int y2 = map(signal_buffer[i + 1], 400, 2400, SCREEN_HEIGHT, 10);

                if (y1 >= 0 && y1 < SCREEN_HEIGHT && y2 >= 0 && y2 < SCREEN_HEIGHT) {
                    // Draw line with 3-pixel width
                    display->drawLine(i, y1, i + 1, y2, SSD1306_WHITE);
                    display->drawLine(i, y1 + 1, i + 1, y2 + 1, SSD1306_WHITE);
                    display->drawLine(i, y1 + 2, i + 2, y2 + 2, SSD1306_WHITE);
                }
            }

            // Draw second channel (thin line - 1 pixel)
            for (int i = 1; i < SCREEN_WIDTH - 2; i++) {
                if (signal_buffer2[i] == 0)
                    continue;
                if (signal_buffer2[i + 1] == 0)
                    continue;

                int y1 = map(signal_buffer2[i], 400, 2400, SCREEN_HEIGHT, 10);
                int y2 = map(signal_buffer2[i + 1], 400, 2400, SCREEN_HEIGHT, 10);

                if (y1 >= 0 && y1 < SCREEN_HEIGHT && y2 >= 0 && y2 < SCREEN_HEIGHT) {
                    // Draw line with 1-pixel width
                    display->drawLine(i, y1, i + 1, y2, SSD1306_WHITE);
                }
            }
            break;

        case DisplayMode::SPLIT:
            // Draw first channel in upper half (thick line - 3 pixels)
            for (int i = 1; i < SCREEN_WIDTH - 2; i++) {
                if (signal_buffer[i] == 0)
                    continue;
                if (signal_buffer[i + 1] == 0)
                    continue;

                int y1 = map(signal_buffer[i], 400, 2400, SCREEN_HEIGHT / 2, 10);
                int y2 = map(signal_buffer[i + 1], 400, 2400, SCREEN_HEIGHT / 2, 10);

                if (y1 >= 0 && y1 < SCREEN_HEIGHT / 2 && y2 >= 0 && y2 < SCREEN_HEIGHT / 2) {
                    // Draw line with 3-pixel width
                    display->drawLine(i, y1, i + 1, y2, SSD1306_WHITE);
                    display->drawLine(i, y1 + 1, i + 1, y2 + 1, SSD1306_WHITE);
                    display->drawLine(i, y1 + 2, i + 2, y2 + 2, SSD1306_WHITE);
                }
            }

            // Draw second channel in lower half (thick line - 3 pixels)
            for (int i = 1; i < SCREEN_WIDTH - 2; i++) {
                if (signal_buffer2[i] == 0)
                    continue;
                if (signal_buffer2[i + 1] == 0)
                    continue;

                int y1 = map(signal_buffer2[i], 400, 2400, SCREEN_HEIGHT, SCREEN_HEIGHT / 2);
                int y2 = map(signal_buffer2[i + 1], 400, 2400, SCREEN_HEIGHT, SCREEN_HEIGHT / 2);

                if (y1 >= SCREEN_HEIGHT / 2 && y1 < SCREEN_HEIGHT && y2 >= SCREEN_HEIGHT / 2 && y2 < SCREEN_HEIGHT) {
                    // Draw line with 3-pixel width
                    display->drawLine(i, y1, i + 1, y2, SSD1306_WHITE);
                    display->drawLine(i, y1 + 1, i + 1, y2 + 1, SSD1306_WHITE);
                    display->drawLine(i, y1 + 2, i + 2, y2 + 2, SSD1306_WHITE);
                }
            }
            break;
    }

    /*
    // Draw trigger level using dotted line
    int trigger_level =
        map(sigscoper.get_trigger_threshold(), 400, 2400, 64, 10);
    for(int i = 0; i < SCREEN_WIDTH; i += 2) {
        display->drawPixel(i, trigger_level, SSD1306_WHITE);
    }
    // */

    /*
    // Draw trigger position using dotted line
    for(int i = 10; i < SCREEN_HEIGHT; i += 2) {
        display->drawPixel(SCREEN_WIDTH / 2, i, SSD1306_WHITE);
    }
    // */

    if (!is_rolling(current_scale_index)) {
        if (display_mode == DisplayMode::SPLIT) {
            // Draw separate ticks and midY lines for split mode
            const int midY1 = SCREEN_HEIGHT / 4;      // Middle of upper half
            const int midY2 = 3 * SCREEN_HEIGHT / 4;  // Middle of lower half

            // Draw ticks for upper half (first channel)
            for (int x = SCREEN_WIDTH - 2; x >= 0; x -= TICK_SPACING) {
                for (int y = midY1 - TICK_SIZE / 2; y <= midY1 + TICK_SIZE / 2; y++) {
                    display->drawPixel(x, y, SSD1306_WHITE);
                }
            }

            // Draw ticks for lower half (second channel)
            for (int x = SCREEN_WIDTH - 2; x >= 0; x -= TICK_SPACING) {
                for (int y = midY2 - TICK_SIZE / 2; y <= midY2 + TICK_SIZE / 2; y++) {
                    display->drawPixel(x, y, SSD1306_WHITE);
                }
            }

            // Draw dotted midY lines for both halves
            for (int x = SCREEN_WIDTH - tickOffset; x >= 0; x -= 4) {
                display->drawPixel(x, midY1, SSD1306_WHITE);
                display->drawPixel(x, midY2, SSD1306_WHITE);
            }
        } else {
            // Draw single midY line and ticks for other modes
            const int midY = SCREEN_HEIGHT / 2;

            for (int x = SCREEN_WIDTH - 2; x >= 0; x -= TICK_SPACING) {
                for (int y = midY - TICK_SIZE / 2; y <= midY + TICK_SIZE / 2; y++) {
                    display->drawPixel(x, y, SSD1306_WHITE);
                }
            }

            for (int x = SCREEN_WIDTH - tickOffset; x >= 0; x -= 4) {
                display->drawPixel(x, midY, SSD1306_WHITE);
            }
        }
    }

    // display->setCursor(0, SCREEN_HEIGHT - 10);
    // display->printf("%d", trigger_count);
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
    if (event == nullptr)
        return;

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
        signal_config.trigger_mode = is_rolling(current_scale_index) ? TriggerMode::FREE : TriggerMode::AUTO_RISE;

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
            // Switch display mode
            switch (display_mode) {
                case DisplayMode::SINGLE:
                    display_mode = DisplayMode::JOINED;
                    break;
                case DisplayMode::JOINED:
                    display_mode = DisplayMode::SPLIT;
                    break;
                case DisplayMode::SPLIT:
                    display_mode = DisplayMode::SINGLE;
                    break;
            }
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