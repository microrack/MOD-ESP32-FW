#pragma once

#include <Adafruit_SSD1306.h>

class Input;

enum TestFlagIndex {
    TestFlagButtonA = 0,
    TestFlagButtonSw = 1,
    TestFlagEncoderCW = 2,
    TestFlagEncoderCCW = 3,
    TestFlagCount = 4
};

bool test_mode(Adafruit_SSD1306* display, Input* input);

