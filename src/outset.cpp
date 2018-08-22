// outset.cpp
#include "outset.h"

Outset::Outset()
:
tft(),
stateInfo{
  { F("INVALID_STATE"), NULL },
  { F("SPLASH_STATE"), &Outset::splashState },
  { F("TEXT_HISTORY_STATE"), &Outset::textHistoryState },
  { F("TEXT_MESSAGE_STATE"), &Outset::textMessageState }
}
{
  initialState = SPLASH_STATE;
  currentState = INVALID_STATE;
  deviceID = DEVICE_ID;

  startCursorLastDrawn = 0;
  cursorVisible = false;
}

void Outset::init() {
  // Enable level shifter and screen
  pinMode(LVL_SHIFT_EN, OUTPUT);
  digitalWrite(LVL_SHIFT_EN, LOW);
  pinMode(TFT_PWR_EN, OUTPUT);
  digitalWrite(TFT_PWR_EN, HIGH);
  delay(250);
  digitalWrite(LVL_SHIFT_EN, HIGH);
  digitalWrite(TFT_PWR_EN, LOW);
  pinMode(TP_BUTTON, INPUT);

  tft.init();
  tft.setRotation(3);
  tft.fillScreen(ST7735_BLACK);

  // Set up the initial state transition.
  switchToState(initialState, INIT_BOOT);
}

void Outset::runFSM() {
  if ((nextState <= INVALID_STATE) || (nextState >= NUM_STATES)) {
    // TODO clear nextState? state 0 is an invalid state!
    Serial.print(F("cannot switch to state "));
    Serial.println(nextState);
    panic();
  }
  else if (currentState != nextState) {
    currentState = nextState;
    const __FlashStringHelper *curStateName = stateInfo[currentState].name;

    Serial.print(F("ENTER "));
    Serial.print(curStateName);
    Serial.print(F(", event "));
    Serial.println(nextEvent);

    // Call the event handler to enter the state.
    (this->*stateInfo[currentState].handler)(nextEvent);
  }
}

void Outset::switchToState(uint8_t newState, uint8_t event) {
  if ((newState <= INVALID_STATE) || (newState >= NUM_STATES)) {
    Serial.print(F("cannot switch to state "));
    Serial.println(newState);
    panic();
  }

  // Tina Q: if this is a const, wouldn't this keep stacking every time
  // switchToState is called?
  const __FlashStringHelper *curStateName = stateInfo[currentState].name;
  const __FlashStringHelper *newStateName = stateInfo[newState].name;

  Serial.print(F("STATE "));
  Serial.print(curStateName);
  Serial.print(F(": prep for "));
  Serial.println(newStateName);

  nextState = newState;
  nextEvent = event;
}

void Outset::splashState(uint8_t event) {
  tft.setTextColor(ST7735_WHITE);
  drawTextContainer(36, 35, 90, 55);
  drawMiniLogo((tft.width()-11)/2, (tft.height()-32)/2);
  tft.setCursor((tft.width()-55)/2, (tft.height()-7)/2);
  tft.println("GREYCAT.CO");
  tft.setCursor((tft.width()-35)/2, (tft.height()+8)/2);
  tft.println("DEMO v1");

  while (currentState == SPLASH_STATE) {
    blinkStartCursor((tft.width()-6)/2, 124);
    if (digitalRead(TP_BUTTON) == HIGH) {
      Serial.println(F("button pushed!"));
      Serial.println(deviceID);
    }
  }
}

void Outset::textHistoryState(uint8_t event) {

}

void Outset::textMessageState(uint8_t event) {

}

void Outset::panic() {
  Serial.println(F("PANIC"));
  for (;;);
}

// Draw mini logo starting at given x,y coords
void Outset::drawMiniLogo(uint8_t x, uint8_t y) {
  // Face outline
  tft.drawFastHLine(x+1, y, 2, BLUE_LIGHTEST);
  tft.drawFastHLine(x+9, y, 2, BLUE_LIGHTEST);
  tft.drawFastHLine(x+4, y+2, 4, BLUE_LIGHTEST);
  tft.drawFastVLine(x+1, y+1, 3, BLUE_LIGHTEST);
  tft.drawFastVLine(x+10, y+1, 3, BLUE_LIGHTEST);
  tft.drawPixel(x+3, y+1, BLUE_LIGHTEST);
  tft.drawPixel(x+8, y+1, BLUE_LIGHTEST);
  tft.drawFastVLine(x, y+4, 5, BLUE_LIGHTEST);
  tft.drawFastVLine(x+11, y+4, 5, BLUE_LIGHTEST);
  tft.drawPixel(x+1, y+9, BLUE_LIGHTEST);
  tft.drawPixel(x+10, y+9, BLUE_LIGHTEST);
  tft.drawFastHLine(x+2, y+10, 8, BLUE_LIGHTEST);
  // Eye (left)
  tft.drawPixel(x+2, y+5, BLUE_MID);
  tft.drawPixel(x+4, y+5, BLUE_MID);
  tft.drawPixel(x+4, y+7, BLUE_MID);
  tft.drawPixel(x+2, y+7, BLUE_DARK);
  tft.drawPixel(x+2, y+6, BLUE_LIGHT);
  tft.drawPixel(x+4, y+6, BLUE_LIGHT);
  tft.drawPixel(x+3, y+7, BLUE_LIGHT);
  // Eye (right)
  tft.drawPixel(x+7, y+5, BLUE_MID);
  tft.drawPixel(x+9, y+5, BLUE_MID);
  tft.drawPixel(x+7, y+7, BLUE_MID);
  tft.drawPixel(x+9, y+7, BLUE_DARK);
  tft.drawPixel(x+7, y+6, BLUE_LIGHT);
  tft.drawPixel(x+9, y+6, BLUE_LIGHT);
  tft.drawPixel(x+8, y+7, BLUE_LIGHT);
}

// Draws a text container given x,y coords and width,height
void Outset::drawTextContainer(uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
  // tft.fillRect(0, 0, 160, 55, BLUE_DARK);
  tft.drawRect(x, y, w, h, BLUE_MID); // main border
  tft.drawRect(x+1, y+1, w-2, h-2, BLUE_MID);
  tft.fillRect(x, y, 2, 2, BLUE_DARK); // top left corner
  tft.fillRect(x+2, y+2, 3, 3, BLUE_MID);
  tft.drawPixel(x+2, y+2, BLUE_LIGHT);
  tft.drawPixel(x+4, y+4, BLUE_LIGHT);
  tft.fillRect(x+(w-2), y, 2, 2, BLUE_DARK); // top right corner
  tft.fillRect(x+(w-5), y+2, 3, 3, BLUE_MID);
  tft.drawPixel(x+(w-5), y+4, BLUE_LIGHT);
  tft.drawPixel(x+(w-3), y+2, BLUE_LIGHT);

  tft.fillRect(x, y+(h-2), 2, 2, BLUE_DARK); // bottom left corner
  tft.fillRect(x+2, y+(h-5), 3, 3, BLUE_MID);
  tft.drawPixel(x+2, y+(h-3), BLUE_LIGHT);
  tft.drawPixel(x+4, y+(h-5), BLUE_LIGHT);
  tft.fillRect(x+(w-2), y+(h-2), 2, 2, BLUE_DARK); // bottom right corner
  tft.fillRect(x+(w-5), y+(h-5), 3, 3, BLUE_MID);
  tft.drawPixel(x+(w-5), y+(h-5), BLUE_LIGHT);
  tft.drawPixel(x+(w-3), y+(h-3), BLUE_LIGHT);

  tft.fillRect(x+5, y+2, w-10, 2, BLUE_DARK); // shadow
  tft.fillRect(x+2, y+5, 3, 2, BLUE_DARK);
  tft.fillRect(x+(w-5), y+5, 3, 2, BLUE_DARK);
}

void Outset::blinkStartCursor(uint8_t x, uint8_t y) {
  currentMillis = millis();
  if(currentMillis - startCursorLastDrawn > 1000) {
    // Save the last time cursor was updated
    startCursorLastDrawn = currentMillis;
    if (cursorVisible) {
      // Clear cursor
      tft.fillRect(x, y, 6, 4, ST7735_BLACK);
      cursorVisible = false;
    } else { // Draw cursor
      tft.fillRect(x, y, 6, 2, ST7735_WHITE);
      tft.drawFastHLine(x+1, y+2, 4, ST7735_WHITE);
      tft.drawFastHLine(x+2, y+3, 2, ST7735_WHITE);
      cursorVisible = true;
    }
  }
}
