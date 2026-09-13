#include <Arduino.h>
#include <ESP32Encoder.h>

ESP32Encoder encoder;

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <OneButton.h>
#include <SPI.h>

// Pin configuration for ESP32
#define TFT_CS 26
#define TFT_DC 25
#define TFT_RST 33
#define TFT_MOSI 23
#define TFT_SCK 22
#define BUTTON_PIN 32

int encodercount;
int prev_encodercount;

int menuScreenSelector = 0;

// Used to know whether the encoder has moved from the last measurement
// +1 : Encoder has moved in positive direction
// -1 : Encoder has moved in negative direction
// 0  : No movement detected
int encoder_delta;

OneButton btnSelect;

// Create display object (pass pins in constructor)
Adafruit_ST7789 tft =
    Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCK, TFT_RST);

// Enum to keep track of the current screen
enum Screen
{
  MENU,
  SETTINGS,
  TIMER
};
Screen currentScreen = MENU;

void setScreen(Screen screen)
{
  tft.fillScreen(ST77XX_BLACK);
  currentScreen = screen;
}

void handleSelectButtonPress() { Serial.println("Button pressed"); }

void setup()
{

  Serial.begin(115200);

  btnSelect.setup(BUTTON_PIN, INPUT_PULLUP, true);
  btnSelect.setClickMs(200);
  btnSelect.attachClick(handleSelectButtonPress);

  // Enable the weak pull down resistors

  // Initialize display
  tft.init(240, 320); // Set resolution (adjust if needed)
  tft.setRotation(1);

  // Vedere quanto posso abbassarlo prima che dia problemi
  // tft.setSPISpeed(100000000);

  // Test
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  // tft.setCursor(10, 10);
  // tft.println("ST7789V Works!");
  // ESP32Encoder::useInternalWeakPullResistors = puType::down;

  //  Enable the weak pull up resistors
  ESP32Encoder::useInternalWeakPullResistors = puType::up;

  // encoder setup
  encoder.attachHalfQuad(4, 5);
  encoder.setCount(0);
}

void handleMenuScreen(int encoder_delta)
{

  menuScreenSelector += encoder_delta;

  tft.setCursor(10, 10);
  tft.println("Menu Screen");

  tft.setCursor(10, 30);
  tft.fillRect(10, 30, 50, 50, ST77XX_BLACK);
  tft.println(menuScreenSelector);

  if (menuScreenSelector < 0)
  {
    setScreen(TIMER);
  }
}

void handleTimerScreen(int encoder_delta)
{
  tft.setCursor(10, 30);
  tft.println("Timer Screen");
  esp_deep_sleep_start();

  tft.setCursor(10, 30);
  tft.fillRect(10, 30, 50, 50, ST77XX_BLACK);
  tft.println(menuScreenSelector);

  if (menuScreenSelector < 0)
  {
    setScreen(MENU);
  }
}

void loop()
{
  // tick buttons otherwise they won't work
  btnSelect.tick();

  encodercount = encoder.getCount();
  if (prev_encodercount != encodercount)
  {
    encoder_delta = encoder.getCount() - prev_encodercount;
    prev_encodercount = encodercount;
  }
  else
  {
    encoder_delta = 0;
  }

  // delay(100);

  switch (currentScreen)
  {
  case MENU:
    handleMenuScreen(encoder_delta);
    break;
  case TIMER:
    handleTimerScreen(encoder_delta);
    break;
  }
}
