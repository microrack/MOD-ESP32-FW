#include <driver/dac.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Encoder.h>
#include <Adafruit_NeoPixel.h>

Adafruit_NeoPixel strip = Adafruit_NeoPixel(4, 23, NEO_GRB + NEO_KHZ800);

#include <MIDI.h>

// Create a 'MIDI' object using MySettings bound to Serial2.
MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial2, MIDI, midi::DefaultSerialSettings);

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const int BUTTON_A = 38;
const int ENCODER_SW = 39;
const int PWM = 33;
const int DAC_1 = 25;
const int DAC_2 = 26;
const int ADC_0 = 36;
const int ADC_1 = 37;
const int SYNC_IN = 18;
const int SYNC_OUT = 19;

ESP32Encoder encoder;

uint32_t period = 0;
void IRAM_ATTR sync_handle() {
  static uint32_t last_sync = 0;

  period = millis() - last_sync;
  last_sync = millis();
}

uint8_t last_note = 0;
void handle_note_on(byte channel, byte note, byte velocity) {
  last_note = note;
}

void setup() {
  strip.begin();
  strip.setBrightness(255);
  strip.clear();

  encoder.attachHalfQuad (34, 35);
  encoder.setCount(0);

  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(ENCODER_SW, INPUT_PULLUP);
  pinMode(34, INPUT_PULLUP);
  pinMode(35, INPUT_PULLUP);
  pinMode(SYNC_IN, INPUT);
  pinMode(SYNC_OUT, OUTPUT);

  ledcSetup(0, 16000, 12);
  ledcAttachPin(PWM, 0);
  ledcSetup(1, 16000, 12);
  ledcAttachPin(DAC_1, 1);
  ledcSetup(2, 16000, 12);
  ledcAttachPin(DAC_2, 2);

  randomSeed(analogRead(0));

  Serial.begin(9600);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.cp437(true);         // Use full 256 char 'Code Page 437' font
  display.setRotation(2);

  attachInterrupt(SYNC_IN, sync_handle, RISING);

  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.setHandleNoteOn(handle_note_on);
  MIDI.turnThruOff();

  // Serial2.begin(31250);
}

void draw_inputs(uint8_t state) {
  // display.setCursor(0, 1);
  for(uint8_t i = 0; i < 8; i++) {
    if(state & 1 << i) {
      display.write('+');
    } else {
      display.write('-');
    }
  }
}

void loop() {
  MIDI.read();

  static uint32_t t_display = millis();
  if(millis() - t_display > 20) {
    display.clearDisplay();
    display.setCursor(0, 0);

    display.printf("%+.1f %+.1f",
      -5. + 10. * analogRead(ADC_0)/4096,
      -5. + 10. * analogRead(ADC_1)/4096
    );
    display.write('\n');
    draw_inputs(
      digitalRead(BUTTON_A) << 4 |
      digitalRead(ENCODER_SW) << 6
    );

    display.printf("  %d\n", encoder.getCount());
    display.printf("  note: %d", last_note);

    display.write('\n');
    if(period > 0) {
      display.printf("sync: %d ms, %d BPM", period, 30000/period);
    }

    display.display();
  }

  static uint32_t t_dac = millis();
  if(millis() - t_dac > 10) {
    ledcWrite(0, encoder.getCount() * 16);
    ledcWrite(1, encoder.getCount() * 16);
    ledcWrite(2, encoder.getCount() * 16);

    strip.setPixelColor(0, random(0, 255), random(0, 255), random(0, 255));
    strip.setPixelColor(1, random(0, 255), random(0, 255), random(0, 255));
    strip.setPixelColor(2, random(0, 255), random(0, 255), random(0, 255));
    strip.setPixelColor(3, random(0, 255), random(0, 255), random(0, 255));
    strip.show();

    uint8_t value = map(analogRead(ADC_0), 0, 4096, 0, 127);
    MIDI.sendNoteOn(value, 127, 1);
    delay(50);
    MIDI.sendNoteOff(value, 127, 1);

    t_dac = millis();
  }

  static uint32_t t_sync = millis();
  if(millis() - t_sync > encoder.getCount() * 2)
  {
    digitalWrite(SYNC_OUT, HIGH);
    delay(1);
    digitalWrite(SYNC_OUT, LOW);

    t_sync = millis();
  }
  delay(2);
}
