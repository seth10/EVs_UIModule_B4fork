/** \file EVs_UIModule.h
  UIModule.h defines interfaces used by the EVShield UIModule.
 \mainpage  EVShield UIModule Library Reference
 \section intro_sec Introduction
 This library provides interface implementaiton to use EVShield UIModule<br>
 For more information about the UIModule, please visit:<br>
 http://www.openelectrons.com/index.php?module=pagemaster&PAGE_user_op=view_page&PAGE_id=99
*/

/*
 * Copyright (C) 2015 OpenElectrons.com
 *
 * This file is part of EVShield interface library.
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef EVs_UIModule_H
#define EVs_UIModule_H

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9340.h>

#define EVs_UIM_BLACK   0x0000
#define EVs_UIM_BLUE    0x001F
#define EVs_UIM_RED     0xF800
#define EVs_UIM_GREEN   0x07E0
#define EVs_UIM_CYAN    0x07FF
#define EVs_UIM_MAGENTA 0xF81F
#define EVs_UIM_YELLOW  0xFFE0  
#define EVs_UIM_WHITE   0xFFFF

#define EVs_BTN_LEFT A3
#define EVs_BTN_RIGHT A0
#define EVs_BTN_UP A1
#define EVs_BTN_DOWN A2
#define EVs_BTN_CLICK  2


/**
  @brief This class defines methods for the UIModule for EVShield
  */
class EVs_UIModule : public Adafruit_ILI9340
{
  public:
    /**  Constructor for UI Module, 
    this constructor takes input as three SPI pin assignments.
    */
    EVs_UIModule(uint8_t CS = 7, uint8_t RS = 8, uint8_t RST = 9);

  /** initialize the library interface, its default values and performs hardware setup required for operation.
  */
    void begin();

  /** Set the rotation (orientation) of the screen 
    @param r orientation value: <br>
        1 for landscape  <br>
        0 for portrait
   */
  void  setRotation(uint8_t r);

  /** Clear the screen of any previous text
  */
  void clearScreen();

  /**
  Get the button state of the specific button on UI Module
  @param btn      Button to get state for following: <br>
            EVs_BTN_LEFT <br>
            EVs_BTN_RIGHT <br>
            EVs_BTN_UP <br>
            EVs_BTN_DOWN <br>
            EVs_BTN_CLICK
  @return true if button is pressed (false otherwise)
  */
  bool getButtonState(uint8_t btn);

  /** 
  Wait inside function until specified button is pressed on UIModule
  @param btn      Button to get state for: <br>
            EVs_BTN_LEFT <br>
            EVs_BTN_RIGHT <br>
            EVs_BTN_UP <br>
            EVs_BTN_DOWN <br>
            EVs_BTN_CLICK
  */
  void waitForButtonPress(uint8_t btn);

  /**
  Clears the specified line.
  @param lineNo the line number to be cleared.
  */
  void clearLine(uint8_t lineNo);

  /**
  Clears the specified line.
  @param x      X position where to begin writing
  @param lineNo   Line Number (along Y) to write to
  @param outText  The output text
  @param clearLine clear the line before writing (true/false)
  @param color Color of the output text:<br>
            EVs_UIM_BLACK <br>
            EVs_UIM_BLUE <br>
            EVs_UIM_RED <br>
            EVs_UIM_GREEN <br>
            EVs_UIM_CYAN <br>
            EVs_UIM_MAGENTA <br>
            EVs_UIM_YELLOW <br>
            EVs_UIM_WHITE
  */
  void writeLine(uint16_t x, uint8_t lineNo, const char *outText, bool clearLine, uint16_t color);

};

#endif
