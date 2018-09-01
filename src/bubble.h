/*
  bubble.cpp - Outset texting demo
  author: Tina Zhao
  github: @manacake
*/
#ifndef Bubble_h
#define Bubble_h

#include <stdint.h>

// Used as arguments for drawing helper function
#define TAIL_LEFT_SIDE  0
#define TAIL_RIGHT_SIDE 1

// Represents a text bubble in our messaging system
class Bubble {
  public:
    Bubble();
    Bubble(const char* message, const char* timestamp, uint8_t createdBy);
    // Get the timestamp of a bubble instance
    char* timestamp();
    // Get the message of a bubble instance
    char* message();
    // Get the height (pixels) of a bubble instance
    // This includes the padding and text
    uint8_t height();
    // Get the width (pixels) of a bubble instance
    // Does NOT include the wispy talking trail -- only the square bubble itself
    // [the triangle piece that sticks on the border of the bubble and indicates
    // the direction in which the bubble came from]
    uint8_t width();
    // Get the id associated with the bubble instance
    // id can be a form of deviceID or identifier of person who wrote the text
    uint8_t createdBy();
    // Used to test if the bubble has a timestamp and message.
    bool isEmpty();

  private:
    char _timestamp[9];
    char _message[250];
    uint8_t _height;
    uint8_t _width;
    uint8_t _createdBy; // Can use deviceID here
};

#endif
