/*
  bubble.cpp - Outset texting demo
  author: Tina Zhao
  github: @manacake
*/
#include "outset.h"

Outset::Outset()
:
radio(RFM_CS, RFM_INT),
tft(TFT_CS, TFT_DC, TFT_RESET),
keypadKeys{ // '`' denotes an unmapped character
  {'`', '`', '{', '[', '`', '`', ']', '}', '`', '`'},
  {'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p'},
  {'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', '~'},
  {'>', 'z', 'x', 'c', 'v', 'b', 'n', 'm', '$', '<'},
  {'`', '`', '`', '^', '0', ' ', '=', '^', '`', '`'}
},
keypadRowPins{ROW_1, ROW_2, ROW_3, ROW_4, ROW_5},
keypadColPins{COL_1, COL_2, COL_3, COL_4, COL_5, COL_6, COL_7, COL_8, COL_9, COL_10},
keypad(makeKeymap(keypadKeys), keypadRowPins, keypadColPins, KEYPAD_ROWS, KEYPAD_COLS),
stateInfo{
  { F("INVALID_STATE"), NULL },
  { F("SPLASH_STATE"), &Outset::splashState },
  { F("TEXT_HISTORY_STATE"), &Outset::textHistoryState },
  { F("TEXT_MESSAGE_STATE"), &Outset::textMessageState },
  { F("TEXT_SEND_STATE"), &Outset::textSendState },
}
{
  initialState = SPLASH_STATE;
  currentState = INVALID_STATE;
  deviceID = DEVICE_ID;
  // Splash screen state defaults
  startCursorLastDrawn = 0;
  startCursorVisible = false;
  lastTrackpadState = LOW;
  lastTrackpadDebounce = 0;
  debounceDelay = 50;
  // Text history defaults
  drawFromTop = true;
  sumOfBubbleHeights = 0;
  historyIndex = 0;
  textHistoryNeedsUpdate = false;
  // Keyboard defaults
  outgoingMessageLen = 0;
  shiftPressed = false;
  symPressed = false;
  messageX = 2;
  messageY = 17;
  // Text message defaults
  outgoingMessageUpdate = false;
  messageWidth = 0;
  incomingMessageLen = RH_RF95_MAX_MESSAGE_LEN;
}

// OS
void Outset::init() {
  pinMode(RFM_CS, OUTPUT);
  pinMode(TFT_CS, OUTPUT);

  RH_RF95::ModemConfig modemConfig = {
    0x78, // Reg 0x1D: BW=125kHz, Coding=4/8, Header=explicit
    0xc4, // Reg 0x1E: Spread=4096chips/symbol, CRC=enable
    0x0c  // Reg 0x26: LowDataRate=On, Agc=ON (Automatic gain control)
  };
  // Give power to the radio module
  pinMode(RFM_PWR_EN, OUTPUT);
  digitalWrite(RFM_PWR_EN, LOW);
  // Init radio
  if (!radio.init()) {
    Serial.println(F("init failed"));
    panic();
  }
  else if (!radio.setFrequency(915.0)) {
    Serial.println(F("set freq failed"));
    panic();
  }
  else {
    radio.setTxPower(20, false);
    radio.setModemRegisters(&modemConfig);
    Serial.println(F("radio init complete"));
  }

  // Enable level shifter and screen
  pinMode(LVL_SHIFT_EN, OUTPUT);
  digitalWrite(LVL_SHIFT_EN, LOW); // off
  pinMode(TFT_PWR_EN, OUTPUT);
  digitalWrite(TFT_PWR_EN, HIGH); // off
  digitalWrite(LVL_SHIFT_EN, HIGH); // on
  digitalWrite(TFT_PWR_EN, LOW); // on
  pinMode(TP_BUTTON, INPUT);

  tft.initR(INITR_GREENTAB);
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  keypad.setDebounceTime(10);

  // Init clock
  rtc.begin();
  // rtc not running!
  if (!rtc.isrunning()) {
    // Sets the rtc to the date & time this file was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

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

void Outset::panic() {
  Serial.println(F("PANIC"));
  for (;;);
}

void Outset::keypadEvent() {
  switch (currentState) {
    case TEXT_HISTORY_STATE: {
      char key = keypad.getKey();
      if (key && key == '{') {
        Serial.print(F("pressed: "));
        Serial.println(key);
        switchToState(TEXT_MESSAGE_STATE, CONFIRM);
      }
      break;
    }
    case TEXT_MESSAGE_STATE: {
      // We've reached the end, draw cursor on the next line
      if (messageX+6 > 158) {
        blinkTextCursor(2, messageY + 8);
        // tft.fillRect(2, messageY+8, 6, 8, BLUE_LIGHT);
      }
      else if (outgoingMessageLen == 0) {
        blinkTextCursor(messageX, messageY);
        // tft.fillRect(messageX, messageY, 6, 8, BLUE_LIGHT);
      }
      else {
        blinkTextCursor(messageX+6, messageY);
        // tft.fillRect(messageX+6, messageY, 6, 8, BLUE_LIGHT);
      }

      // Scan for keys and draw them on the screen
      if (keypad.getKeys()) {
        for (uint8_t i = 0; i < LIST_MAX; i++) {
          if (keypad.key[i].stateChanged) {
            switch (keypad.key[i].kstate) {
              case PRESSED:
                if (keypad.key[i].kchar == '[') {
                  switchToState(TEXT_SEND_STATE, CONFIRM);
                } else if (keypad.key[i].kchar == '^') {
                  shiftPressed = true;
                } else if (keypad.key[i].kchar == '=') {
                  symPressed = true;
                } else {
                  addCharToMessage(keypad.key[i].kchar);
                }
                break;
              case RELEASED:
                if (keypad.key[i].kchar == '^') {
                  shiftPressed = false;
                } else if (keypad.key[i].kchar == '=') {
                  symPressed = false;
                }
                break;
            }
          }
        }
      }
      // Only draw to screen when there is a change
      if (outgoingMessageUpdate) {
        if (lastOutgoingMessageLen > outgoingMessageLen) {
          // clearBody();
          tft.fillRect(messageX+6, messageY, 14, 8, TFT_BLACK);
        }
        else {
          tft.setCursor(messageX, messageY);
          // Print the last char entered
          tft.print(outgoingMessage[outgoingMessageLen-1]);
        }
        outgoingMessageUpdate = false;
      }
      break; // break for textMessageState
    }
  }
}

char Outset::getShiftedKey(char key) {
  return toupper(key);
}

char Outset::getSymKey(char key) {
  if (key == 'q') return '#';
  else if (key == 'w') return '1';
  else if (key == 'e') return '2';
  else if (key == 'r') return '3';
  else if (key == 't') return '(';
  else if (key == 'y') return ')';
  else if (key == 'u') return '_';
  else if (key == 'i') return '-';
  else if (key == 'o') return '+';
  else if (key == 'p') return '@';
  else if (key == 'a') return '*';
  else if (key == 's') return '4';
  else if (key == 'd') return '5';
  else if (key == 'f') return '6';
  else if (key == 'g') return '/';
  else if (key == 'h') return ':';
  else if (key == 'j') return ';';
  else if (key == 'k') return '\'';
  else if (key == 'l') return '"';
  else if (key == 'z') return '7';
  else if (key == 'x') return '8';
  else if (key == 'c') return '9';
  else if (key == 'v') return '?';
  else if (key == 'b') return '!';
  else if (key == 'n') return ',';
  else if (key == 'm') return '.';
}

void Outset::addCharToMessage(char key) {
  if (shiftPressed) {
    key = getShiftedKey(key);
  }
  else if (symPressed) {
    key = getSymKey(key);
  }
  Serial.print(F("trying char: "));
  Serial.println(key);
  lastOutgoingMessageLen = outgoingMessageLen;
  // Delete last char
  if (key == '~' && outgoingMessageLen != 0) {
    Serial.println(F("deleting last char"));
    outgoingMessageLen--;
    outgoingMessage[outgoingMessageLen] = '\0';
    messageX -= 6;
    if (messageX == 2 && outgoingMessageLen > 1)  {
      messageX = 152;
      messageY -= 8;
    }
    Serial.print(F("messageX: "));
    Serial.println(messageX);
  }
  // We still have space for 2 chars! (char + null)
  else if (outgoingMessageLen < 248) {
    // We have a valid printable key
    if (key != '{' && key != '}' && key != '[' && key != ']'
      && key != '>' && key != '<') {
        // Figure out where this char will be printed
        if (outgoingMessageLen != 0) messageX += 6;
        if (messageX+6 > 158) { // We've reached the end of the screen
          messageX = 2; // Reset to next row
          messageY += 8;
        }
        outgoingMessage[outgoingMessageLen] = key;
        outgoingMessageLen++;
        outgoingMessage[outgoingMessageLen] = '\0';
    }
  }
  outgoingMessageUpdate = true;
  Serial.print(F("message now: "));
  Serial.println(outgoingMessage);
  Serial.print(F("len: "));
  Serial.println(outgoingMessageLen);
}

void Outset::listenForIncomingMessages() {
  if (currentState != TEXT_HISTORY_STATE) return;
  if (radio.available()) {
    Serial.println(F("Message available!"));
    if (radio.recv(incomingMessage, &incomingMessageLen)) {
      Serial.print(F("got message: "));
      Serial.println(incomingMessage);

      DateTime now = rtc.now();
      char timestamp[8];
      formatTime(timestamp, now.hour(), now.minute(), now.second());
      // TODO: use reliableDatagram for addressed sources
      pushMessage(incomingMessage, timestamp, 1);
      textHistoryNeedsUpdate = true;
    }
    else { // Received Failed
      Serial.println(F("failed to receive incoming message"));
    }
  }
}

void Outset::switchToState(uint8_t newState, uint8_t event) {
  if ((newState <= INVALID_STATE) || (newState >= NUM_STATES)) {
    Serial.print(F("cannot switch to state "));
    Serial.println(newState);
    panic();
  }

  const __FlashStringHelper *curStateName = stateInfo[currentState].name;
  const __FlashStringHelper *newStateName = stateInfo[newState].name;

  Serial.print(F("STATE "));
  Serial.print(curStateName);
  Serial.print(F(": prep for "));
  Serial.println(newStateName);

  nextState = newState;
  nextEvent = event;
}

// State handlers ============================================================
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
  Serial.println(F("Exiting splashState"));
}

void Outset::blinkStartCursor(uint8_t x, uint8_t y) {
  if (currentState != SPLASH_STATE) return;

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

// TODO: refactor this (is still using start cursor vars)
void Outset::blinkTextCursor(uint8_t x, uint8_t y) {
  if (currentState != TEXT_MESSAGE_STATE) return;

  currentMillis = millis();
  if (currentMillis - startCursorLastDrawn > 1000) {
    // Save the last time cursor was updated
    startCursorLastDrawn = currentMillis;
    if (startCursorVisible) {
      // Clear cursor
      tft.fillRect(x, y, 6, 8, TFT_BLACK);
      startCursorVisible = false;
    } else { // Draw cursor
      tft.fillRect(x, y, 6, 8, TFT_WHITE);
      startCursorVisible = true;
    }
  }
}

void Outset::textHistoryState(uint8_t event) {
  drawHeader(currentState);
  if (textHistory[0].isEmpty()) {
    tft.setCursor(35, 60);
    tft.print("PRESS GREEN TO TEXT");
  }
  else {
    drawTextHistory();
  }

  while (nextState == TEXT_HISTORY_STATE) {
    keypadEvent();
    listenForIncomingMessages();
    if (textHistoryNeedsUpdate) {
      drawTextHistory();
      textHistoryNeedsUpdate = false;
    }
  }
}

void Outset::textMessageState(uint8_t event) {
  clearBody();
  drawHeader(currentState);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  // Reset all the message vars
  outgoingMessage[0] = '\0';
  outgoingMessageLen = 0;
  lastOutgoingMessageLen = 0;
  outgoingMessageUpdate = false;
  messageX = 2;
  messageY = 17;
  messageWidth = 0;

  while (nextState == TEXT_MESSAGE_STATE) {
    keypadEvent();
  }
}

void Outset::textSendState(uint8_t event) {
  char timestamp[8];
  tft.setTextColor(TFT_WHITE);
  drawHeader(currentState);
  // Send the message in outgoingMessage
  Serial.print(F("Sending message out: "));
  Serial.println(outgoingMessage);
  Serial.print(F("msg len + null terminator: "));
  Serial.println(outgoingMessageLen+1);
  radio.send(outgoingMessage, outgoingMessageLen+1);
  radio.waitPacketSent();

  DateTime now = rtc.now();
  formatTime(timestamp, now.hour(), now.minute(), now.second());
  pushMessage(outgoingMessage, timestamp, deviceID);
  textHistoryNeedsUpdate = true;
  tft.setCursor(2, 17);
  tft.print("MESSAGE SENT!");
  delay(1000);
  switchToState(TEXT_HISTORY_STATE, CONFIRM);
}

void Outset::pushMessage(const char* message, const char* timestamp, uint8_t createdBy) {
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

// Tests =====================================================================
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

// Drawing helpers ===========================================================
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
  clearBody();
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
    case TEXT_SEND_STATE:
      tft.print("SENDING MESSAGE");
      break;
    default:
      Serial.println(F("CAN'T DRAW HEADER FOR INVALID STATE"));
      panic();
  }
}

void Outset::clearBody() {
  Serial.println(F("clearBody"));
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

void Outset::formatTime(char* buff, uint8_t hour, uint8_t min, uint8_t sec) {
  char temp[3];
   buff[0] = '\0';
  itoa(hour, temp, 10);
  if (hour < 10) {
    strcpy(buff, "0");
  }
  strcat(buff, temp);
  strcat(buff, ":");
   if (min < 10) {
    strcat(buff, "0");
  }
  itoa(min, temp, 10);
  strcat(buff, temp);
  strcat(buff, ":");
   if (sec < 10) {
    strcat(buff, "0");
  }
  itoa(sec, temp, 10);
  strcat(buff, temp);
}
