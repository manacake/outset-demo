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
  startCursorVisible = false;
  startCursorEnabled = true;
  lastTrackpadState = LOW;
  lastTrackpadDebounce = 0;
  debounceDelay = 50;
  drawFromTop = true;
  sumOfBubbleHeights = 0;
  historyIndex = 0;
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
  tft.fillScreen(TFT_BLACK);

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
    tft.fillScreen(TFT_BLACK);
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
  tft.setTextColor(TFT_WHITE);
  drawTextContainer(36, 35, 90, 55, BLUE_MODE);
  drawMiniLogo((tft.width()-11)/2, (tft.height()-32)/2);
  tft.setCursor((tft.width()-55)/2, (tft.height()-7)/2);
  tft.println("GREYCAT.CO");
  tft.setCursor((tft.width()-35)/2, (tft.height()+8)/2);
  tft.println("DEMO v1");

  while (nextState == SPLASH_STATE) {
    blinkStartCursor((tft.width()-6)/2, 124);
    uint8_t trackpadReading = digitalRead(TP_BUTTON);

    if (trackpadReading != lastTrackpadState) {
      lastTrackpadDebounce = millis();
    }
    if (millis() - lastTrackpadDebounce > debounceDelay) {
      if (trackpadReading != trackpadState) {
        trackpadState = trackpadReading;
        // Trackpad button was pushed! Navigate to the next state
        if (trackpadState == HIGH) {
          startCursorEnabled = false;
          // Use transition
          for (uint8_t i = 0; i < 4; i++) {
            drawTextContainer(36, 35, 90, 55, PINK_MODE);
            delay(100);
            drawTextContainer(36, 35, 90, 55, BLUE_MODE);
            delay(100);
          }
          switchToState(TEXT_HISTORY_STATE, CONFIRM);
        }
      }
    }
    lastTrackpadState = trackpadReading;
  }
}

void Outset::testMessages() {
  // Simulate messaging
  // Only 5 messsages can fit on a screen (if they're single lined)
  pushMessage("AYYY MATTE!", "10:03:45", 1);
  drawTextHistory();
  delay(2000);
  pushMessage("HEY GREY", "10:03:48", deviceID);
  drawTextHistory();
  delay(2000);
  pushMessage("HOW ARE YOU?", "10:04:51", 1);
  drawTextHistory();
  delay(1000);
  pushMessage("LET'S DO LUNCH", "10:04:52", 1);
  drawTextHistory();
  delay(2000);
  pushMessage("PANCAKES ARE GOOD", "10:04:54", deviceID);
  drawTextHistory();
  delay(1000);
  pushMessage("YUM", "10:04:55", deviceID);
  drawTextHistory();
}

void Outset::pushMessage(char* message, char* timestamp, uint8_t createdBy) {
  Bubble b(message, timestamp, createdBy);
  // textHistory can only hold 6 messages
  if (historyIndex >= 6) { // We're at max capacity!
    // Shift all bubbles left
    for (uint8_t i = 1; i < 7; i++) {
      memcpy(&textHistory[i-1], &textHistory[i], sizeof(textHistory[i]));
    }
    // Add the new text bubble at the end
    textHistory[6] = b;
  }
  else {
    textHistory[historyIndex] = b;
    historyIndex++;
  }
}

void Outset::textHistoryState(uint8_t event) {
  drawHeader(currentState);

  testMessages();
}

void Outset::drawBubble(Bubble bubble) {
  Serial.println(F("drawBubble enter"));
  // Is this my own text or somebody else's?
  if (bubble.createdBy() == deviceID) { // Draw my text bubble with wispy tail on right side
    historyX = 160-(bubble.width()+6); // wispy tail is 5px + 1px gutter
    tft.fillRoundRect(historyX, historyY, bubble.width(), bubble.height(), 4, BLUE_DARK);
    drawWispyTail(154, historyY+(bubble.height()-5), TAIL_RIGHT_SIDE, BLUE_DARK);
  }
  else { // Draw other text bubble with wispy tail on the left side
    historyX = 6;
    tft.fillRoundRect(historyX, historyY, bubble.width(), bubble.height(), 4, BLUE_MID);
    drawWispyTail(1, historyY+(bubble.height()-5), TAIL_LEFT_SIDE, BLUE_MID);
  }
  tft.setCursor(historyX+2, historyY+2);
  tft.println(bubble.timestamp());
  tft.setCursor(historyX+2, historyY+10);
  tft.print(bubble.message());
  // Save the heights of the bubbles on the screen now for next pass
  sumOfBubbleHeights += (bubble.height() + 2); // 1px for margin
  // If we've received more messages that can fit, start drawing bottom up
  if (drawFromTop && sumOfBubbleHeights > 99) {
    Serial.print(F("Reached maximum bubbles from top down! Sum: "));
    Serial.println(sumOfBubbleHeights);
    Serial.println(F("Now drawing from bottom up!"));
    drawFromTop = false;
  }
}

void Outset::drawTextHistory() {
  clearTextHistoryBody();
  // Draw each bubble in history
  if (drawFromTop) {
    for (uint8_t i = 0; i < 7; i++) {
      Bubble bubble = textHistory[i];
      if (sumOfBubbleHeights == 0) {
        // Header is 16px high
        historyY = 16;
      } else {
        historyY = 16 + sumOfBubbleHeights;
      }
      if (!bubble.isEmpty()) {
        Serial.print(F("call drawBubble: "));
        Serial.println(i);
        Serial.print(F("Y axis: "));
        Serial.println(historyY);
        drawBubble(bubble);
      }
    }
  }
  else { // Draw bubbles from the bottom up
    for (int8_t i = 6; i >= 0; i--) {
      Bubble bubble = textHistory[i];
      if (i == 6) {
        // Header is 16px high
        historyY = 108;
        sumOfBubbleHeights = 0; // Reset when drawing from beginning
      } else {
        historyY = 108 - sumOfBubbleHeights;
      }
      if (!bubble.isEmpty() && historyY >= 0) {
        Serial.print(F("call drawBubble: "));
        Serial.println(i);
        Serial.print(F("Y axis: "));
        Serial.println(historyY);
        drawBubble(bubble);
      }
    }
    // Draw the header again because we drew over it
    drawHeader(currentState);
  }
  // Reset at the end because we're drawing all history every loop
  sumOfBubbleHeights = 0;
}

void Outset::drawWispyTail(uint8_t x, uint8_t y, uint8_t side, uint16_t color) {
  switch (side) {
    case TAIL_LEFT_SIDE:
      // These were added to cover up the border radius of the text bubble
      tft.drawFastHLine(x+5, y+4, 3, color);
      tft.drawFastVLine(x+5, y+1, 3, color);
      // Right triangle
      tft.drawFastVLine(x+4, y, 5, color);
      tft.drawFastVLine(x+3, y+1, 4, color);
      tft.drawFastVLine(x+2, y+2, 3, color);
      tft.drawFastVLine(x+1, y+3, 2, color);
      tft.drawPixel(x, y+4, color);
      break;
    case TAIL_RIGHT_SIDE:
      // These were added to cover up the border radius of the text bubble
      tft.drawFastVLine(x-3, y+3, 2, color);
      tft.drawFastVLine(x-2, y+2, 3, color);
      tft.drawFastVLine(x-1, y+1, 4, color);
      // Right triangle
      tft.drawFastVLine(x, y, 5, color);
      tft.drawFastVLine(x+1, y+1, 4, color);
      tft.drawFastVLine(x+2, y+2, 3, color);
      tft.drawFastVLine(x+3, y+3, 2, color);
      tft.drawPixel(x+4, y+4, color);
      break;
  }
}

void Outset::textMessageState(uint8_t event) {

}

void Outset::panic() {
  Serial.println(F("PANIC"));
  for (;;);
}

void Outset::drawHeader(uint8_t state) {
  tft.fillRect(0, 0, 160, 13, BLUE_MID);
  tft.drawFastHLine(0, 13, 160, BLUE_DARK);
  tft.drawFastHLine(0, 14, 160, BLUE_DARK);
  tft.setCursor(16, 3);

  switch (state) {
    case TEXT_HISTORY_STATE:
      drawChatIcon(1, 1);
      drawBatteryIcon(136, 1);

      tft.print("CHATTING WITH ");
      if (deviceID == 7) {
        tft.print("GREY");
      }
      else {
        tft.print("MATTE");
      }
      break;
    case TEXT_MESSAGE_STATE:
      tft.print("WRITE A MESSAGE");
      break;
    default:
      Serial.println(F("CAN'T DRAW HEADER FOR INVALID STATE"));
      panic();
  }
}

void Outset::clearTextHistoryBody() {
  Serial.println(F("clearTextHistoryBody"));
  tft.fillRect(0, 15, 160, 113, TFT_BLACK);
}

void Outset::drawChatIcon(uint8_t x, uint8_t y) {
  tft.fillRect(x+1, x+1, 10, 9, BLUE_LIGHTEST);
  tft.drawFastHLine(x+1, y, 10, BLUE_DARK);
  tft.drawFastHLine(x+1, y+10, 10, BLUE_DARK);
  tft.drawFastVLine(x, y+1, 9, BLUE_DARK);
  tft.drawFastVLine(x+11, y+1, 9, BLUE_DARK);
  tft.drawPixel(x+2, y+4, BLUE_DARK);
  tft.drawPixel(x+3, y+3, BLUE_DARK);
  tft.drawPixel(x+4, y+4, BLUE_DARK);
  tft.drawPixel(x+7, y+4, BLUE_DARK);
  tft.drawPixel(x+8, y+3, BLUE_DARK);
  tft.drawPixel(x+9, y+4, BLUE_DARK);
  tft.fillRect(x+3, y+6, 6, 2, BLUE_DARK);
  tft.drawPixel(x+4, y+8, BLUE_DARK);
  tft.drawPixel(x+7, y+8, BLUE_DARK);
  tft.drawFastHLine(x+5, y+8, 2, PINK_LIGHT);
}

void Outset::drawBatteryIcon(uint8_t x, uint8_t y) {
  tft.drawRect(x+1, y, 19, 11, BLUE_DARK);
  tft.drawFastVLine(x, y+1, 9, BLUE_DARK);
  tft.drawFastVLine(x+20, y+1, 9, BLUE_DARK);
  tft.drawFastVLine(x+21, y+2, 7, BLUE_DARK);
  tft.drawFastVLine(x+22, y+3, 5, BLUE_DARK);

  tft.fillRect(x+2, y+1, 17, 4, GREEN_LIGHT);
  tft.drawFastVLine(x+1, y+2, 3, GREEN_MID);
  tft.drawFastVLine(x+7, y+1, 4, GREEN_MID);
  tft.drawFastVLine(x+13, y+1, 4, GREEN_MID);
  tft.drawFastVLine(x+19, y+2, 3, GREEN_MID);

  tft.fillRect(x+2, y+5, 17, 5, GREEN_MID);
  tft.drawFastVLine(x+1, y+5, 4, GREEN_DARK);
  tft.drawFastVLine(x+7, y+5, 5, GREEN_DARK);
  tft.drawFastVLine(x+13, y+5, 5, GREEN_DARK);
  tft.drawFastVLine(x+19, y+5, 4, GREEN_DARK);
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
  tft.drawPixel(x-1, y+6, BLUE_LIGHTEST);
  tft.drawPixel(x-1, y+8, BLUE_LIGHTEST);
  tft.drawPixel(x+12, y+6, BLUE_LIGHTEST);
  tft.drawPixel(x+12, y+8, BLUE_LIGHTEST);
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
void Outset::drawTextContainer(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color) {
  if (color == BLUE_MODE) {
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
  else if (color == PINK_MODE) {
    tft.drawRect(x, y, w, h, PINK_MID); // main border
    tft.drawRect(x+1, y+1, w-2, h-2, PINK_MID);
    tft.fillRect(x, y, 2, 2, PINK_DARK); // top left corner
    tft.fillRect(x+2, y+2, 3, 3, PINK_MID);
    tft.drawPixel(x+2, y+2, PINK_LIGHT);
    tft.drawPixel(x+4, y+4, PINK_LIGHT);
    tft.fillRect(x+(w-2), y, 2, 2, PINK_DARK); // top right corner
    tft.fillRect(x+(w-5), y+2, 3, 3, PINK_MID);
    tft.drawPixel(x+(w-5), y+4, PINK_LIGHT);
    tft.drawPixel(x+(w-3), y+2, PINK_LIGHT);

    tft.fillRect(x, y+(h-2), 2, 2, PINK_DARK); // bottom left corner
    tft.fillRect(x+2, y+(h-5), 3, 3, PINK_MID);
    tft.drawPixel(x+2, y+(h-3), PINK_LIGHT);
    tft.drawPixel(x+4, y+(h-5), PINK_LIGHT);
    tft.fillRect(x+(w-2), y+(h-2), 2, 2, PINK_DARK); // bottom right corner
    tft.fillRect(x+(w-5), y+(h-5), 3, 3, PINK_MID);
    tft.drawPixel(x+(w-5), y+(h-5), PINK_LIGHT);
    tft.drawPixel(x+(w-3), y+(h-3), PINK_LIGHT);

    tft.fillRect(x+5, y+2, w-10, 2, PINK_DARK); // shadow
    tft.fillRect(x+2, y+5, 3, 2, PINK_DARK);
    tft.fillRect(x+(w-5), y+5, 3, 2, PINK_DARK);
  }
}

void Outset::blinkStartCursor(uint8_t x, uint8_t y) {
  if (!startCursorEnabled) return;

  currentMillis = millis();
  if (currentMillis - startCursorLastDrawn > 1000) {
    // Save the last time cursor was updated
    startCursorLastDrawn = currentMillis;
    if (startCursorVisible) {
      // Clear cursor
      tft.fillRect(x, y, 6, 4, TFT_BLACK);
      startCursorVisible = false;
    } else { // Draw cursor
      tft.fillRect(x, y, 6, 2, TFT_WHITE);
      tft.drawFastHLine(x+1, y+2, 4, TFT_WHITE);
      tft.drawFastHLine(x+2, y+3, 2, TFT_WHITE);
      startCursorVisible = true;
    }
  }
}
