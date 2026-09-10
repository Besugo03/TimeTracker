#include <Arduino.h>

const int PIN_A = 5; // CONFIRM this matches your current wiring
const int PIN_B = 4;

const unsigned long DEBOUNCE_US = 4000; // 4ms per-pin debounce, in microseconds

volatile unsigned long lastAcceptedA = 0;
volatile unsigned long lastAcceptedB = 0;

// simple circular log buffer, filled in ISR, printed in loop
#define LOG_SIZE 100
volatile char logPin[LOG_SIZE];
volatile int logLevel[LOG_SIZE];
volatile unsigned long logTime[LOG_SIZE];
volatile int logHead = 0;
volatile int logCount = 0;

void IRAM_ATTR logEvent(char pin, int level, unsigned long t)
{
  logPin[logHead] = pin;
  logLevel[logHead] = level;
  logTime[logHead] = t;
  logHead = (logHead + 1) % LOG_SIZE;
  if (logCount < LOG_SIZE)
    logCount++;
}

void IRAM_ATTR ISR_A()
{
  unsigned long now = micros();
  if (now - lastAcceptedA < DEBOUNCE_US)
    return; // ignore bounce on THIS pin only
  lastAcceptedA = now;

  logEvent('A', digitalRead(PIN_A), now);
}

void IRAM_ATTR ISR_B()
{
  unsigned long now = micros();
  if (now - lastAcceptedB < DEBOUNCE_US)
    return; // ignore bounce on THIS pin only
  lastAcceptedB = now;

  logEvent('B', digitalRead(PIN_B), now);
}

void setup()
{
  Serial.begin(115200);
  delay(2000);

  pinMode(PIN_A, INPUT_PULLUP);
  pinMode(PIN_B, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(PIN_A), ISR_A, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_B), ISR_B, CHANGE);

  Serial.println("Encoder Order Test Started (debounced). Turn the wheel slowly one click at a time.");
}

void loop()
{
  static int printedIndex = 0;

  noInterrupts();
  int currentCount = logCount;
  int currentHead = logHead;
  interrupts();

  while (printedIndex < currentCount)
  {
    int idx = (currentHead - currentCount + printedIndex + LOG_SIZE) % LOG_SIZE;
    Serial.print(logTime[idx]);
    Serial.print(" us | pin ");
    Serial.print(logPin[idx]);
    Serial.print(" -> ");
    Serial.println(logLevel[idx]);
    printedIndex++;
  }

  if (printedIndex >= LOG_SIZE)
  {
    noInterrupts();
    logHead = 0;
    logCount = 0;
    interrupts();
    printedIndex = 0;
  }

  delay(20);
}