// bubble.cpp
#include "bubble.h"

Bubble::Bubble(char* message, char* timestamp) {
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
    height = baseHeight;
  }
  else if (rows > 2 && rows <= 10) {
    // 8 is the increment when adding another row of text
    height = rows * 8;
  }
  else {
    Serial.println(F("WARNING: Trouble calculating bubble height."));
    Serial.print(F("messageLen: "));
    Serial.print(messageLen);
    Serial.print(F(", rows: "));
    Serial.print(rows);
    panic();
  }
  // Calculate and store the width of the bubble
  // width of a character is 5px
  // padding on the sides is 4px
  // gutter is 1px
  if (messageLen <= 25) {
    width = messageLen*5 + 4 + (messageLen-1)*1;
  } else if (messageLen > 25) { // Max width bubble
    width = (25*5) + 4 + 24;
  }
}

uint8_t Bubble::height() {
  return height;
}

uint8_t Bubble::width() {
  return width;
}

char* Bubble::timestamp() {
  return _timestamp;
}

char* Bubble::message() {
  return _message;
}
