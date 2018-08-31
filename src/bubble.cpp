/*
  bubble.cpp - Outset texting demo
  author: @manacake
*/
#include <math.h>
#include <string.h>
#include <SPI.h>
#include "bubble.h"

Bubble::Bubble() {}
Bubble::Bubble(const char* message, const char* timestamp, uint8_t createdBy) {
  _createdBy = createdBy;
  // Message length is dynamic
  uint8_t messageLen = strlen(message);
  strncpy(_message, message, messageLen);
  _message[messageLen] = '\0';

  // Timestamp length is static
  strncpy(_timestamp, timestamp, 8);
  _timestamp[8] = '\0';

  // The height of the bubble if the message fits on 1 row
  uint8_t baseHeight = 19;
  // Calculate and store the height of the bubble
  // messageLen/25 where 25 is the max length of text in 1 row
  uint8_t rows = ceil(messageLen/25.0) + 1; // add 1 extra row for timestamp
  if (rows <= 2) {
    _height = baseHeight;
  }
  else if (rows > 2 && rows <= 10) {
    // Add in the baseHeight and then any remaining rows is an extra 8px
    // 8 is the increment when adding another row of text
    _height = baseHeight + (rows-2) * 8;
  }
  else {
    Serial.println(F("WARNING: Trouble calculating bubble height."));
    Serial.print(F("messageLen: "));
    Serial.print(messageLen);
    Serial.print(F(", rows: "));
    Serial.print(rows);
  }
  // Calculate and store the width of the bubble
  // width of a character is 5px
  // padding on the sides is 4px
  // gutter is 1px
  if (messageLen <= 25) {
    // 51px is the min width of the timestamp (8*5px)
    _width = max(51, messageLen*5 + 4 + (messageLen-1)*1);
  } else if (messageLen > 25) { // Max width bubble
    _width = (25*5) + 4 + 24;
  }
}

bool Bubble::isEmpty() {
  if (message()[0] == 0 && timestamp()[0] == 0) {
    return true;
  }
  return false;
}

uint8_t Bubble::createdBy() {
  return _createdBy;
}

uint8_t Bubble::height() {
  return _height;
}

uint8_t Bubble::width() {
  return _width;
}

char* Bubble::timestamp() {
  return _timestamp;
}

char* Bubble::message() {
  return _message;
}
