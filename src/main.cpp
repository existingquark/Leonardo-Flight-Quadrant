// -----------------------------------------------------------------------------
// MoonDog Throttle Quadrant Firmware
// Arduino Leonardo (ATmega32u4) USB HID Game Controller
// - Reads 7 analog axes (Throttle 1–6, Axis 7 optional)
// - Reads 32 buttons via two MCP23017 I2C expanders
// - Implements rolling average smoothing for analog noise reduction
// - Includes real-time serial monitor output with live updating
// - Adds adaptive deadband logic for Throttle L/R
// - Adds virtual trim accumulator to simulate multi-turn trim wheel
// -----------------------------------------------------------------------------

#include <Wire.h>
#include <Adafruit_MCP23X17.h>
#include <Joystick.h>
#include <PluggableUSB.h>

#ifndef _USING_DYNAMIC_HID
#define _USING_DYNAMIC_HID
#endif

#if defined(_USING_DYNAMIC_HID)
#pragma message("✅ Dynamic HID is ENABLED")
#else
#pragma message("❌ Dynamic HID is NOT enabled")
#endif

// -----------------------------------------------------------------------------
// Joystick HID Interface
// -----------------------------------------------------------------------------
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID,
                   JOYSTICK_TYPE_MULTI_AXIS, 32, 2,
                   true, true, true, // X, Y, Z
                   true, true, true, // Rx, Ry, Rz
                   false, false, false,
                   false, false);

// -----------------------------------------------------------------------------
// I/O Expanders (MCP23017) for 32 Button Inputs
// -----------------------------------------------------------------------------
Adafruit_MCP23X17 mcp1;
Adafruit_MCP23X17 mcp2;

// -----------------------------------------------------------------------------
// Axis Configuration and Smoothing Buffers
// -----------------------------------------------------------------------------
const int NUM_AXES = 7;
const int axisPins[NUM_AXES] = {A0, A1, A2, A3, A4, A5, A6};

const int AXIS_RAW_MIN[NUM_AXES] = {196, 196, 196, 196, 196, 196, 196};
const int AXIS_RAW_MAX[NUM_AXES] = {1023, 1023, 1023, 1023, 1023, 1023, 1023};

const int filterWindowSize = 10;
int axisBuffers[NUM_AXES][filterWindowSize] = {{0}};
int axisSums[NUM_AXES] = {0};
int axisIndices[NUM_AXES] = {0};

const char *axisLabels[NUM_AXES] = {
    "Throttle L", "Throttle R", "Trim", "Mixture 1",
    "Mixture 2", "TBD Axis", "TBD Axis"};

// -----------------------------------------------------------------------------
// Adaptive Deadband & Virtual Trim Accumulation
// -----------------------------------------------------------------------------
int lastStableOutput[NUM_AXES] = {0};
const int DEADZONE_THRESHOLDS[NUM_AXES] = {
    1, 1, 0, 0, 0, 0, 0};

// Virtual trim state (Z axis): accumulate relative movement for scaled response
float accumulatedTrim = 512.0f; // Start at midpoint
float lastTrimAvg = 0;
const float TRIM_INCREMENT_SCALE = 0.5f;
const int TRIM_AXIS_INDEX = 2; // Trim is axis 2

int applyDeadband(int axisIndex, int currentMapped)
{
  int delta = abs(currentMapped - lastStableOutput[axisIndex]);
  if (delta >= DEADZONE_THRESHOLDS[axisIndex])
  {
    lastStableOutput[axisIndex] = currentMapped;
  }
  return lastStableOutput[axisIndex];
}

// -----------------------------------------------------------------------------
// Setup Routine
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
  while (!Serial)
  {
  }
  Serial.println("Throttle Debug Initialized");

  for (int a = 0; a < NUM_AXES; a++)
  {
    int initVal = analogRead(axisPins[a]);
    for (int i = 0; i < filterWindowSize; i++)
    {
      axisBuffers[a][i] = initVal;
      axisSums[a] += initVal;
    }
    lastStableOutput[a] = map(initVal, AXIS_RAW_MIN[a], AXIS_RAW_MAX[a], 0, 1023);
  }
  lastTrimAvg = axisSums[TRIM_AXIS_INDEX] / filterWindowSize;
}

// -----------------------------------------------------------------------------
// Read Button States from MCP23017 Expanders
// -----------------------------------------------------------------------------
void readButtons()
{
  for (int i = 0; i < 16; i++)
  {
    Joystick.setButton(i, !mcp1.digitalRead(i));
    Joystick.setButton(i + 16, !mcp2.digitalRead(i));
  }
}

// -----------------------------------------------------------------------------
// Return Smoothed and Mapped Value for a Given Axis
// -----------------------------------------------------------------------------
int getSmoothedAxis(int axisIndex, int &rawOut, int &averageOut)
{
  rawOut = analogRead(axisPins[axisIndex]);

  axisSums[axisIndex] -= axisBuffers[axisIndex][axisIndices[axisIndex]];
  axisBuffers[axisIndex][axisIndices[axisIndex]] = rawOut;
  axisSums[axisIndex] += rawOut;
  axisIndices[axisIndex] = (axisIndices[axisIndex] + 1) % filterWindowSize;

  averageOut = axisSums[axisIndex] / filterWindowSize;

  int mapped = map(averageOut,
                   AXIS_RAW_MIN[axisIndex], AXIS_RAW_MAX[axisIndex],
                   0, 1023);
  return constrain(mapped, 0, 1023);
}

// -----------------------------------------------------------------------------
// Read and Transmit Axis Values (Throttle 1–6 Only)
// -----------------------------------------------------------------------------
void readAxes()
{
  for (int i = 0; i < 6; i++)
  {
    int raw, avg;
    int mapped = getSmoothedAxis(i, raw, avg);

    // Handle trim axis separately for slow accumulation
    if (i == TRIM_AXIS_INDEX)
    {
      float delta = avg - lastTrimAvg;
      lastTrimAvg = avg;
      accumulatedTrim += delta * TRIM_INCREMENT_SCALE;
      accumulatedTrim = constrain(accumulatedTrim, 0, 1023);
      Joystick.setZAxis((int)accumulatedTrim);
      continue;
    }

    int stable = applyDeadband(i, mapped);

    switch (i)
    {
    case 0:
      Joystick.setXAxis(stable);
      break;
    case 1:
      Joystick.setYAxis(stable);
      break;
    case 3:
      Joystick.setRxAxis(stable);
      break;
    case 4:
      Joystick.setRyAxis(stable);
      break;
    case 5:
      Joystick.setRzAxis(stable);
      break;
    }
  }
}

// -----------------------------------------------------------------------------
// Debug Output for Serial Monitor — Live Table View
// -----------------------------------------------------------------------------
void printAxisDebug()
{
  Serial.print("\033[2J\033[H");
  Serial.println("─────────────────────────────────────────────────────────────────────────────");
  Serial.println("  Axis         Raw    Smoothed    Mapped     ΔMapped");
  Serial.println("─────────────────────────────────────────────────────────────────────────────");

  for (int i = 0; i < 6; i++)
  {
    int raw, avg;
    int mapped = getSmoothedAxis(i, raw, avg);
    int delta = abs(mapped - lastStableOutput[i]);
    int stable = applyDeadband(i, mapped);

    Serial.print("  ");
    Serial.print(axisLabels[i]);
    Serial.print("  |  ");
    Serial.print(raw);
    Serial.print("  |    ");
    Serial.print(avg);
    Serial.print("     |   ");
    Serial.print(mapped);
    Serial.print("     |     ");
    Serial.println(delta);
  }

  Serial.println("─────────────────────────────────────────────────────────────────────────────");
}

// -----------------------------------------------------------------------------
// Main Loop — Read Inputs and Print Debug Output
// -----------------------------------------------------------------------------
void loop()
{
  readButtons();
  readAxes();
  printAxisDebug();
  delay(100);
}
