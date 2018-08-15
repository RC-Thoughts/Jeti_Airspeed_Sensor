/*
   -----------------------------------------------------------
                   Jeti AirSpeed Sensor v 1.1
   -----------------------------------------------------------

    Tero Salminen RC-Thoughts.com (c) 2017 www.rc-thoughts.com

  -----------------------------------------------------------

    Airspeed sensor for Jeti EX telemetry with cheap
    MPXV7002DP breakout-board and Arduino Pro Mini

    Jeti sensor values:
    - Airspeed display
    - User selectable format km/h or mph
    - Speedrange ~20 to 225km/h (~20 to 140mph)

  -----------------------------------------------------------
    Shared under MIT-license by Tero Salminen (c) 2017-2018
  -----------------------------------------------------------
*/

// No settings to be defined by user - Use Jetibox
#include <JetiExSerial.h>
#include <JetiExProtocol.h>
#include <EEPROM.h>
#include "SpeedPressure.h"
#define sensor_pin A3

JetiExProtocol jetiEx;
Pressure speedSensor(sensor_pin);

int airSpeed = 0;
int baseval = 0;
int units;
static char _Speed[16];

const int numReadings = 20;
int readings[numReadings];
int readIndex = 0;
int total = 0;
int average = 0;

static int  _nMenu = 0;
static bool _bSetDisplay = true;

enum
{
  ID_AIRSPEED = 1,
};

JETISENSOR_CONST sensorsEU[] PROGMEM =
{
  { ID_AIRSPEED,    "Airspeed",   "km/h",       JetiSensor::TYPE_14b, 0 },
  { 0 }
};

JETISENSOR_CONST sensorsUS[] PROGMEM =
{
  { ID_AIRSPEED,    "Airspeed",   "mph",        JetiSensor::TYPE_14b, 0 },
  { 0 }
};

void setup() {
  units = EEPROM.read(0);
  if (units == 255) {
    units = 0;
  }
  if (units == 1) {
    jetiEx.Start( "RCT", sensorsUS, JetiExProtocol::SERIAL2 );
  } else {
    jetiEx.Start( "RCT", sensorsEU, JetiExProtocol::SERIAL2 );
  }
  speedSensor.Init();
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }
  jetiEx.SetDeviceId( 0x76, 0x33 );
}

void loop() {

  // Spiiiiiiiiiid! <- copyright Jeremy Clarkson
  total = total - readings[readIndex];
  readings[readIndex] = speedSensor.GetAirSpeed();
  total = total + readings[readIndex];
  readIndex = readIndex + 1;

  if (readIndex >= numReadings) {
    readIndex = 0;
  }
  airSpeed = total / numReadings;
  if (units == 1) airSpeed = airSpeed * 0.621371;
  if (airSpeed < 20) airSpeed = 0;

  jetiEx.SetSensorValue( ID_AIRSPEED, airSpeed );

  if(_nMenu == 1) {
    if (units == 1) {
      jetiEx.SetJetiboxText( JetiExProtocol::LINE1, "Airspeed (mph)" );
     } else {
      jetiEx.SetJetiboxText( JetiExProtocol::LINE1, "Airspeed (km/h)" );
     }
     sprintf( _Speed, ": %d", airSpeed );
     jetiEx.SetJetiboxText( JetiExProtocol::LINE2, _Speed );
  }

  HandleMenu();
  jetiEx.DoJetiSend();
}

void HandleMenu()
{
  uint8_t c = jetiEx.GetJetiboxKey();

  // 224 0xe0 : // RIGHT
  // 112 0x70 : // LEFT
  // 208 0xd0 : // UP
  // 176 0xb0 : // DOWN
  // 144 0x90 : // UP+DOWN
  //  96 0x60 : // LEFT+RIGHT

  // Right
  if ( c == 0xe0 && _nMenu < 4 )
  {
    _nMenu++;
    _bSetDisplay = true;
  }

  // Left
  if ( c == 0x70 &&  _nMenu > 0 )
  {
    _nMenu--;
    _bSetDisplay = true;
  }

  // Down
  if ( c == 0xb0 )
  {
    if ( _nMenu == 2 ) {
      units = 0;
      EEPROM.write(0, units);
      _nMenu = 5;
      _bSetDisplay = true;
    }
    if ( _nMenu == 3 ) {
      units = 1;
      EEPROM.write(0, units);
      _nMenu = 5;
      _bSetDisplay = true;
    }
    if ( _nMenu == 4 ) {
      units = 0;
      EEPROM.write(0, units);
      _nMenu = 5;
      _bSetDisplay = true;
    }
  }

  if ( !_bSetDisplay )
    return;

  switch ( _nMenu )
  {
    case 0:
      jetiEx.SetJetiboxText( JetiExProtocol::LINE1, " RCT Jeti Tools" );
      jetiEx.SetJetiboxText( JetiExProtocol::LINE2, "  RCT Airspeed" );
      _bSetDisplay = false;
      break;
    case 1:
      if (units == 1) {
       jetiEx.SetJetiboxText( JetiExProtocol::LINE1, "Airspeed (mph)" );
      } else {
       jetiEx.SetJetiboxText( JetiExProtocol::LINE1, "Airspeed (km/h)" );
      }
      sprintf( _Speed, ": %d", airSpeed );
      jetiEx.SetJetiboxText( JetiExProtocol::LINE2, _Speed );
      _bSetDisplay = false;
      break;
    case 2:
      jetiEx.SetJetiboxText( JetiExProtocol::LINE1, "  Unit: km/h" );
      jetiEx.SetJetiboxText( JetiExProtocol::LINE2, "  Store: DOWN" );
      _bSetDisplay = false;
      break;
    case 3:
      jetiEx.SetJetiboxText( JetiExProtocol::LINE1, "  Unit: mph" );
      jetiEx.SetJetiboxText( JetiExProtocol::LINE2, "  Store: DOWN" );
      _bSetDisplay = false;
      break;
    case 4:
      jetiEx.SetJetiboxText( JetiExProtocol::LINE1, " Reset defaults" );
      jetiEx.SetJetiboxText( JetiExProtocol::LINE2, "  Store: DOWN" );
      _bSetDisplay = false;
      break;
    case 5:
      jetiEx.SetJetiboxText( JetiExProtocol::LINE1, "Settings stored!" );
      jetiEx.SetJetiboxText( JetiExProtocol::LINE2, " Search sensors!" );
      _bSetDisplay = false;
      break;
      if (_nMenu == 5) {
        delay(1500);
        _nMenu = 0;
        _bSetDisplay = true;
      }
  }
}
