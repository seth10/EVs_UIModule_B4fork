
// EVs_UIModule.cpp

/*
  Copyright (C) 2015 OpenElectrons.com

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "EVs_UIModule.h"


EVs_UIModule::EVs_UIModule(uint8_t CS, uint8_t RS, uint8_t RST):Adafruit_ILI9340(CS, RS, RST)
{
    #if defined(ESP8266)
    // read touchscreen calibration values from PiStorms

    Wire.begin(D2,D3);

    // set BAS1 type to none so it doesn't interfere with the following i2c communicaiton
    Wire.beginTransmission(0x34 >> 1); // SH_Bank_A
    Wire.write(0x6F); // SH_S1_MODE
    Wire.write(0x00); // SH_Type_NONE
    Wire.endTransmission();

    // copy from permanent memory to temporary memory
    Wire.beginTransmission(0x34 >> 1); // SH_Bank_A
    Wire.write(0x41); // SH_COMMAND
    Wire.write(0x6C); // SH_PS_TS_LOAD
    Wire.endTransmission();

    delay(2); // normally it only takes 2 milliseconds or so to load the values
    unsigned long timeout = millis() + 1000; // wait for up to a second
    uint8_t calibrationDataReadyStatus = 0;
    while (calibrationDataReadyStatus != 1) {
        Wire.beginTransmission(0x34 >> 1); // SH_Bank_A
        Wire.write(0x70); // SH_PS_TS_CALIBRATION_DATA_READY
        Wire.endTransmission();
        Wire.requestFrom(0x70, 1); // SH_PS_TS_CALIBRATION_DATA_READY
        calibrationDataReadyStatus = Wire.read();
        Wire.endTransmission();
        delay(10);
        if (millis() > timeout) break; // TODO: exception handling, ts_cal doesn't have values
    }
    Serial.begin(115200);
    for (int i = 0; i < 8; i++) { // SH_PS_TS_CALIBRATION_DATA
        uint8_t reg = 0x71 + i*0x02;
        Wire.beginTransmission(0x34 >> 1); // SH_Bank_A
        Wire.write(reg); 
        Wire.endTransmission();
        Wire.requestFrom(reg, (uint8_t)2);
        ts_cal[i] = Wire.read() | (Wire.read() << 8);
        Wire.endTransmission();
        Serial.println(ts_cal[i]);
    }
    #endif
}

void EVs_UIModule::setRotation(uint8_t r)
{
    Adafruit_ILI9340::setRotation(r);
}

void EVs_UIModule::clearScreen()
{
    Adafruit_ILI9340::fillScreen(EVs_UIM_BLACK);
}

void EVs_UIModule::begin()
{
    Adafruit_ILI9340::begin();
    #if defined(ESP8266)
    setRotation(3);
    #else
    setRotation(1);
    #endif
    setTextSize(2);
    setTextColor(EVs_UIM_WHITE);
    #if !defined(ESP8266)
    pinMode(EVs_BTN_LEFT, INPUT);
    pinMode(EVs_BTN_RIGHT, INPUT);
    pinMode(EVs_BTN_UP, INPUT);
    pinMode(EVs_BTN_DOWN, INPUT);
    pinMode(EVs_BTN_CLICK, INPUT);
    #endif
}

bool EVs_UIModule::getButtonState(uint8_t btn)
{
    #if defined(ESP8266)
    #warning from EVs_UIModule: bool getButtonState(uint8_t btn) is not supported
    return false;
    #else
    return (!digitalRead(btn));
    #endif
}

void EVs_UIModule::waitForButtonPress(uint8_t btn)
{
    #if defined(ESP8266)
    #warning from EVs_UIModule: void waitForButtonPress(uint8_t btn) is not supported
    return;
    #else
    while (digitalRead(btn))
        ;
    #endif
}

void EVs_UIModule::clearLine(uint8_t lineNo)
{
    uint16_t y = (lineNo - 1) * 16;
    Adafruit_ILI9340::fillRect(0,y,320,16,EVs_UIM_BLACK);
}

void EVs_UIModule::writeLine(uint16_t x, uint8_t lineNo, const char *outText, bool clearLine, uint16_t color)
{
    if(clearLine==true){
        EVs_UIModule::clearLine(lineNo);
    }
    setTextSize(2);
    setTextColor(color);
    Adafruit_GFX::setCursor(x,((lineNo - 1) * 16));
    println(outText);
    
}

// read the raw x-coordinate of the touchscreen press
uint16_t RAW_X()
{
    Wire.beginTransmission(0x34 >> 1); // SH_Bank_A
    Wire.write(0xE7); // SH_PS_TS_RAWX
    Wire.endTransmission();
    Wire.requestFrom(0xE7, 2); // SH_PS_TS_RAWX
    uint16_t rawx = Wire.read() | (Wire.read() << 8);
    Wire.endTransmission();
    return rawx;
}

// read the raw y-coordinate of the touchscreen press
uint16_t RAW_Y()
{
    Wire.beginTransmission(0x34 >> 1); // SH_Bank_A
    Wire.write(0xE9); // SH_PS_TS_RAWY
    Wire.endTransmission();
    Wire.requestFrom(0xE9, 2); // SH_PS_TS_RAWY
    uint16_t rawy = Wire.read() | (Wire.read() << 8);
    Wire.endTransmission();
    return rawy;
}

// helper function to getReading
double distanceToLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) // some point and two points forming the line
{
    // multiply by 1.0 to avoid integer truncation, don't need parentheses because the multiplication operator has left-to-right associativity
    return 1.0 * abs( (y2-y1)*x0 - (x2-x1)*y0 + x2*y1 - y2*x1 ) / sqrt( pow((y2-y1),2) + pow((x2-x1),2) );
}

// (private) get raw touchscreen values, do some math using the calibration values, and write to the output parameters
void EVs_UIModule::getReading(uint16_t *retx, uint16_t *rety) // returnX, returnY to avoid shadowing local x, y
{
    uint16_t x = RAW_X();
    uint16_t y = RAW_Y();
    
    uint16_t x1 = ts_cal[0],
             y1 = ts_cal[1],
             x2 = ts_cal[2],
             y2 = ts_cal[3],
             x3 = ts_cal[4],
             y3 = ts_cal[5],
             x4 = ts_cal[6],
             y4 = ts_cal[7];

    if ( x < min(x1,min(x2,min(x3,x4))) \
      || x > max(x1,max(x2,max(x3,x4))) \
      || y < min(y1,min(y2,min(y3,y4))) \
      || y > max(y1,max(y2,max(y3,y4))) ) {
        *retx = 0;
        *rety = 0;
        return;
    }

    // careful not to divide by 0
    if ( y2-y1 == 0 \
      || x4-x1 == 0 \
      || y3-y4 == 0 \
      || x3-x2 == 0 ) {
        *retx = 0;
        *rety = 0;
        return;
    }

    // http://math.stackexchange.com/a/104595/363240
    uint16_t dU0 = distanceToLine(x, y, x1, y1, x2, y2) / (y2-y1) * 320;
    uint16_t dV0 = distanceToLine(x, y, x1, y1, x4, y4) / (x4-x1) * 240;

    uint16_t dU1 = distanceToLine(x, y, x4, y4, x3, y3) / (y3-y4) * 320;
    uint16_t dV1 = distanceToLine(x, y, x2, y2, x3, y3) / (x3-x2) * 240;

    // careful not to divide by 0
    if ( dU0+dU1 == 0 \
      || dV0+dV1 == 0 ) {
        *retx = 0;
        *rety = 0;
        return;
    }

    *retx = 320 * dU0/(dU0+dU1);
    *rety = 240 * dV0/(dV0+dV1);
}

void EVs_UIModule::getTouchscreenValues(uint16_t *x, uint16_t *y)
{
    #if defined(ESP8266)
    uint16_t x1, y1;
    getReading(&x1, &y1);
    uint16_t x2, y2;
    getReading(&x2, &y2);

    const uint8_t tolerance = 5;
    if (abs(x2-x1) < tolerance and abs(y2-y1) < tolerance) {
        *x = x2;
        *y = y2;
    } else {
        *x = 0;
        *y = 0;
    }
    #endif
}

uint16_t EVs_UIModule::TS_X()
{
    #if defined(ESP8266)
    uint16_t x, y;
    getTouchscreenValues(&x, &y);
    return x;
    #else
    return 0;
    #endif
}

uint16_t EVs_UIModule::TS_Y()
{
    #if defined(ESP8266)
    uint16_t x, y;
    getTouchscreenValues(&x, &y);
    return y;
    #else
    return 0;
    #endif
}

bool EVs_UIModule::isTouched()
{
    #if defined(ESP8266)
    uint16_t x, y;
    getTouchscreenValues(&x, &y);
    return !(x==0 && y==0);
    #else
    return false;
    #endif
}

bool EVs_UIModule::checkButton(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
    #if defined(ESP8266)
    uint16_t tsx, tsy; // touch screen x, touch screen y
    getTouchscreenValues(&tsx, &tsy);

    if (tsx==0 && tsy==0) return false;

    // 0,0 is top-left corner
    // if left of right edge, right of left edge, above bottom edge, and below top edge
    return tsx<=x+width && tsx>=x && tsy<=y+height && tsy>=y;
    #else
    return false;
    #endif
}

uint8_t EVs_UIModule::getFunctionButton()
{
    #if defined(ESP8266)
    uint16_t x = RAW_X();
    
    uint16_t x1 = ts_cal[0],
             y1 = ts_cal[1],
             x2 = ts_cal[2],
             y2 = ts_cal[3],
             x3 = ts_cal[4],
             y3 = ts_cal[5],
             x4 = ts_cal[6],
             y4 = ts_cal[7];
             
    uint16_t xborder;
    if (x4 > x1)  { // lower values left
        xborder = max(x1, x2); // where the touchscreen ends and the software buttons begin
        if (!(x < xborder && x > xborder-200)) return 0;
    } else { // greater values left
        xborder = min(x1, x2);
        if (!(x > xborder && x < xborder+200)) return 0;
    }

    uint16_t y = RAW_Y(),
             ymin = min(y1, y2),
             ymax = max(y1, y2),
             yQuarter = (ymax-ymin)/4; // a quarter of the distance between the two y extremes

    if (y < ymin + 0 * yQuarter) return 0; // too low
    if (y < ymin + 1 * yQuarter) return 1;
    if (y < ymin + 2 * yQuarter) return 2;
    if (y < ymin + 3 * yQuarter) return 3;
    if (y < ymin + 4 * yQuarter) return 4;
    if (y >= ymin + 4 * yQuarter) return 0; // too high

    return 0; // some other weird error occured, execution should not reach this point
    #else
    return 0;
    #endif
}