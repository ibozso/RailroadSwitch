/*
 *
 */

#include "RailroadSwitchPeripheries.h"
#include "Arduino.h"
#include "Adafruit_NeoPixel.h"
#include "Ticker.h"

#define C_ELAPSED_TIME_LONG          2000u
#define C_ELAPSED_TIME_SHORT         100u
#define C_ELAPSED_TIME_SWITCH        500u

#define NEOPIXEL_COUNT               1u
#define NEOPIXEL_PIN                 12u
#define BUTTON_PIN                   14u
#define RELAY_CONTROL_PIN            13u
#define RELAY_SWITCH_PIN             15u

typedef enum
{
  BUTTON_STATE_HIGH,
  BUTTON_STATE_LOW,
} tButtonState;

Adafruit_NeoPixel NeoPixel = Adafruit_NeoPixel(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
Ticker NeoPixelTicker;
unsigned long PixelColor = COLOR_BLACK;
bool PixelBlink = false;
unsigned long ActiveTime = 0u;
tSwitchDirection SwitchDirection = SWITCH_DIRECTION_NEUTRAL;

static void ControlNeoPixel(void);

/*
 *
 */

void SwitchCommand(unsigned int Id, tSwitchDirection Direction)
{
  SwitchDirection = Direction;

  Serial.println("*** SwitchDirection()");
  Serial.print("Direction = ");
  Serial.println(Direction);

  if (Direction < SWITCH_DIRECTION_NEUTRAL)
  {
    ActiveTime = millis() + C_ELAPSED_TIME_SWITCH;
  }
  else
  {
    ActiveTime = 0u;
  }
}

/*
 *
 */

void InitSwitch(void)
{
  pinMode(BUTTON_PIN, INPUT);
  pinMode(RELAY_CONTROL_PIN, OUTPUT);
  pinMode(RELAY_SWITCH_PIN, OUTPUT);
}

/*
 *
 */

void SwitchControl(void)
{
  if (ActiveTime >= millis())
  {
    /* Keep the switch powered */
    digitalWrite(RELAY_CONTROL_PIN, HIGH);

    if (SwitchDirection == SWITCH_DIRECTION_LEFT)
    {
      Serial.println("*** SwitchControl() - RELAY_SWITCH_PIN, HIGH");
      digitalWrite(RELAY_SWITCH_PIN, HIGH);
    }
    else
    {
      Serial.println("*** SwitchControl() - RELAY_SWITCH_PIN, LOW");
      digitalWrite(RELAY_SWITCH_PIN, LOW);
    }
  }
  else
  {
    /* Cut off power */
    digitalWrite(RELAY_CONTROL_PIN, LOW);
  }
}

/*
 *
 */

void InitNeoPixel(void)
{
  NeoPixel.begin();
  NeoPixelTicker.attach(0.25, ControlNeoPixel);
  SetNeoPixel(false, COLOR_BLACK);
}

/*
 *
 */

static void ControlNeoPixel(void)
{
  static bool NeoPixelState = false;

  if ((NeoPixelState) || (!PixelBlink))
  {
    NeoPixel.setPixelColor(0u, PixelColor);
  }
  else
  {
    NeoPixel.setPixelColor(0u, COLOR_BLACK);
  }

  NeoPixel.show();
  NeoPixelState = !NeoPixelState;
}

/*
 *
 */

void SetNeoPixel(bool Blink, unsigned int Color)
{
  PixelBlink = Blink;
  PixelColor = Color;
}

/*
 *
 */

tButtonEvent ButtonMonitor(void)
{
  static tButtonState ButtonState = BUTTON_STATE_HIGH;
  static long StartTime;
  unsigned long ElapsedTime;
  tButtonEvent Result = BUTTON_EVENT_IDLE;

  if (digitalRead(BUTTON_PIN) == HIGH)
  {
    if (ButtonState == BUTTON_STATE_HIGH)
    {
      Serial.println("CheckButtonEvent(): ButtonState = BUTTON_STATE_LOW");
      ButtonState = BUTTON_STATE_LOW;
      StartTime = millis();
    }
  }
  else
  {
    if (ButtonState == BUTTON_STATE_LOW)
    {
      ButtonState = BUTTON_STATE_HIGH;
      ElapsedTime = millis() - StartTime;
      Serial.println("CheckButtonEvent(): ButtonState = BUTTON_STATE_HIGH");
      Serial.print("CheckButtonEvent(): ElapsedTime = ");
      Serial.println(ElapsedTime);

      if (ElapsedTime > C_ELAPSED_TIME_LONG)
      {
        Serial.println("CheckButtonEvent(): ButtonEvent = BUTTON_EVENT_LONGCLICK");
        Result = BUTTON_EVENT_LONGCLICK;
      }
      else
      {
        if (ElapsedTime > C_ELAPSED_TIME_SHORT)
        {
          Serial.println("CheckButton(): ButtonEvent = BUTTON_EVENT_SHORTCLICK");
          Result = BUTTON_EVENT_SHORTCLICK;
        }
      }
    }
  }

  return Result;
}
