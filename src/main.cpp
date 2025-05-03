// -----------------------------------------------------------------------------
// MoonDog Throttle Quadrant Firmware
// Embedded firmware for Arduino Leonardo (ATmega32u4) as a USB HID Game Device
// Reads analog throttle axis + 32 button inputs over I2C via two MCP23017 expanders
// Implements smoothing and dynamic calibration for unstable potentiometers
// -----------------------------------------------------------------------------

#include <Wire.h>
#include <Adafruit_MCP23X17.h>
#include <Joystick.h>
#include <PluggableUSB.h>

// -----------------------------------------------------------------------------
// USB HID Descriptor Check
// -----------------------------------------------------------------------------
#if defined(_USING_DYNAMIC_HID)
#pragma message("✅ Dynamic HID is ENABLED")
#else
#pragma message("❌ Dynamic HID is NOT enabled")
#endif

// -----------------------------------------------------------------------------
// Function Prototypes
// -----------------------------------------------------------------------------
void readButtons();
void readAxes();
int getSmoothedThrottle();
void updateThrottleCalibration(int raw);

// -----------------------------------------------------------------------------
// Joystick HID Report Definition
// -----------------------------------------------------------------------------
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID,
                   JOYSTICK_TYPE_MULTI_AXIS, 32, 2,
                   true, true, true,     // X, Y, Z
                   true, true, true,     // Rx, Ry, Rz
                   false, false, false,  // rudder, throttle, accelerator
                   false, false);

// -----------------------------------------------------------------------------
// I2C Expanders (Button Banks)
// -----------------------------------------------------------------------------
Adafruit_MCP23X17 mcp1;
Adafruit_MCP23X17 mcp2;

// -----------------------------------------------------------------------------
// Throttle Axes
// -----------------------------------------------------------------------------
const int axisPins[7] = {A0, A1, A2, A3, A4, A5, A6};

// -----------------------------------------------------------------------------
// Smoothing Filter for Throttle 1
// -----------------------------------------------------------------------------
const int filterWindowSize = 15;
int throttleBuffer[filterWindowSize] = {0};
int throttleIndex = 0;
long throttleSum = 0;

// -----------------------------------------------------------------------------
// Dynamic Calibration Bounds for Throttle 1
// -----------------------------------------------------------------------------
int throttleRawMin = 1023;
int throttleRawMax = 0;

// -----------------------------------------------------------------------------
// Setup Function - Called Once at Startup
// -----------------------------------------------------------------------------
void setup()
{
  Wire.begin();

  mcp1.begin_I2C(0x20);
  mcp2.begin_I2C(0x21);

  for (int i = 0; i < 16; i++)
  {
    mcp1.pinMode(i, INPUT_PULLUP);
    mcp2.pinMode(i, INPUT_PULLUP);
  }

  Joystick.begin();

  Serial.begin(9600);
  while (!Serial) {}
  Serial.println("[MoonDog] Throttle Debug Initialized");

  // Initialize Smoothing Buffer
  int initial = analogRead(axisPins[0]);
  for (int i = 0; i < filterWindowSize; i++)
  {
    throttleBuffer[i] = initial;
    throttleSum += initial;
  }

  // Initialize Calibration Range
  throttleRawMin = initial;
  throttleRawMax = initial;
}

// -----------------------------------------------------------------------------
// Read Digital Button Inputs from Two MCP23017 Expanders
// -----------------------------------------------------------------------------
void readButtons()
{
  for (int i = 0; i < 16; i++)
  {
    Joystick.setButton(i, !mcp1.digitalRead(i));
  }
  for (int i = 0; i < 16; i++)
  {
    Joystick.setButton(i + 16, !mcp2.digitalRead(i));
  }
}

// -----------------------------------------------------------------------------
// Apply Rolling Average Smoothing and Dynamic Calibration to Throttle 1
// -----------------------------------------------------------------------------
int getSmoothedThrottle()
{
  int raw = analogRead(axisPins[0]);

  // Update calibration range
  updateThrottleCalibration(raw);

  // Update moving average window
  throttleSum -= throttleBuffer[throttleIndex];
  throttleBuffer[throttleIndex] = raw;
  throttleSum += raw;
  throttleIndex = (throttleIndex + 1) % filterWindowSize;

  int average = throttleSum / filterWindowSize;

  // Map and constrain to 0–1023 range
  int mapped = map(average, throttleRawMin - 10, throttleRawMax + 10, 0, 1023);
  mapped = constrain(mapped, 0, 1023);

  // Debug output
  Serial.print("Raw: ");
  Serial.print(raw);
  Serial.print(" | Smoothed: ");
  Serial.print(average);
  Serial.print(" | Mapped: ");
  Serial.print(mapped);
  Serial.print(" | Min: ");
  Serial.print(throttleRawMin);
  Serial.print(" | Max: ");
  Serial.println(throttleRawMax);

  return mapped;
}

// -----------------------------------------------------------------------------
// Dynamically Expand Calibration Range to Fit Observed Values
// -----------------------------------------------------------------------------
void updateThrottleCalibration(int raw)
{
  if (raw < throttleRawMin)
    throttleRawMin = raw;
  if (raw > throttleRawMax)
    throttleRawMax = raw;
}

// -----------------------------------------------------------------------------
// Read Analog Axes (Throttle 1 Only for Now)
// -----------------------------------------------------------------------------
void readAxes()
{
  int throttle = getSmoothedThrottle();
  Joystick.setXAxis(throttle);
}

// -----------------------------------------------------------------------------
// Main Loop - Continuously Poll Inputs and Send HID Report
// -----------------------------------------------------------------------------
void loop()
{
  readButtons();
  readAxes();
  delay(10); // Allow USB HID time to update
}
