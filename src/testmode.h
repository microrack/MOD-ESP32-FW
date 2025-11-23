#pragma once

#include <Adafruit_SSD1306.h>
#include "signal_processor/signal_processor.h"

class Input;

enum TestFlagIndex {
    TestFlagButtonA = 0,
    TestFlagButtonSw = 1,
    TestFlagEncoderCW = 2,
    TestFlagEncoderCCW = 3,
    TestFlagMidiRx = 4,
    TestFlagIN_0 = 5,
    TestFlagIN_1 = 6,
    TestFlagCount = 7
};

bool test_mode(Adafruit_SSD1306* display, Input* input, SignalProcessor* signal_processor);

