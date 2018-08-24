// bubble.h
#ifndef Bubble_h
#define Bubble_h

// Represents a text bubble in our messaging system
class Bubble {
  public:
    Bubble(char* timestamp, char* message);
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

  private:
    char _timestamp[9];
    char _message[250];
    uint8_t height;
    uint8_t width;
};

#endif
