/*
  bubble.cpp - Outset texting demo
  author: Tina Zhao
  github: @manacake
*/
#ifndef Outset_h
#define Outset_h

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <Keypad.h>
#include <RH_RF95.h>
#include <SPI.h>
#include <Wire.h>
#include "RTClib.h"
#include "bubble.h"
#include "states.h"
#include "pins.h"
#include "colors.h"

class Outset {
  public:
    Outset();
    // Set the datagram driver and any dev pins
    void init();
    // Run the FSM through one state transition.
    void runFSM();

  private:
    RH_RF95 radio;
    Adafruit_ST7735 tft;
    Keypad keypad;
    RTC_DS1307 rtc;

    // Array of function pointers modeling outset's state (view)
    // index represents the state
    // value at index is the handler/render function for that state
    typedef void (Outset::*stateHandlerPtr)(uint8_t);
    struct {
      const __FlashStringHelper *name; // The type returned by F()
      stateHandlerPtr handler;
    } stateInfo[NUM_STATES];

    // The first state outset is in when outset starts
    uint8_t initialState;

    // The current state of the deivce
    uint8_t currentState;

    // The state we'll switch to for the next pass through loop()
    uint8_t nextState;
    uint8_t nextEvent;

    // Comes from build flags (ideally, would grab serial # from hardware)
    uint8_t deviceID;

    // For use in timing blinking cursors and other animated objects
    unsigned long currentMillis;

    // Keep state of blinking cursor on splash state
    unsigned long startCursorLastDrawn; // in ms

    // The state of the blinking cursor on the start view
    bool startCursorVisible;

    // Used to initialize the Keypad class
    const char keypadKeys[KEYPAD_ROWS][KEYPAD_COLS];
    const uint8_t keypadRowPins[KEYPAD_ROWS];
    const uint8_t keypadColPins[KEYPAD_COLS];

    // The state of the shift key on the keypad. Returns true if pressed else false
    bool shiftPressed;

    // The state of the sym key on the keypad. Returns true if pressed else false
    bool symPressed;

    // Last time a keypress or other event (i.e. new incoming data) happens in order to wake up screen
   unsigned long lastWakeupEventMills;

    // Used for keeping the debounce state of the trackpad
    uint8_t trackpadState;
    uint8_t lastTrackpadState;
    unsigned long lastTrackpadDebounce;
    unsigned long debounceDelay;

    // TEXT_HISTORY_STATE helper values
    // Draws text bubbles underneath the header until there's too many bubbles
    // on screen. The screen can only fit 6 one line message bubbles for now.
    bool drawFromTop;
    int sumOfBubbleHeights;
    uint8_t historyIndex;
    Bubble textHistory[7]; // Only keep 6 messages for now...

    // Keep track of the next place to draw text bubbles
    int historyX;
    int historyY;

    // Sets the new state for the FSM to look at.
    // \param newState: The next state to swtich to. See list in "states.h"
    // \param event: The event that caused the state switch. See list in "states.h"
    void switchToState(uint8_t newState, uint8_t event);

    // State Handlers
    // Event handlers can take as long as they like, including waiting for
    // external things, but they MUST return after a call to switchToState
    // (which could go back to its own state, of course).

    // The start/splash view
    void splashState(uint8_t event = 0);
    // The view where messages are displayed (a text history)
    void textHistoryState(uint8_t event = 0);
    // The view where the user can type out a message
    void textMessageState(uint8_t event = 0);
    // The view in which a message is being sent
    void textSendState(uint8_t event = 0);
    // Clears the space underneath the header bar
    void clearBody();

    // Listens to the keypad for the currentState and responds accordingly
    void keypadEvent();

    // Message Handlers
    // Adds the given message as a Bubble struct at the end of the textHistory array
    // \param message: The message `string`. Max length is 250 bytes
    // \param timestamp: The timestamp of the message. Format is hh:mm:ss
    // \param createdBy: The id (I'm using deviceID) of the message writer
    void pushMessage(const char* message, const char* timestamp, uint8_t createdBy);

    // Checks the radio module to see if there's a message for us. If so, use
    // pushMessage to add that message to textHistory array.
    void listenForIncomingMessages();

    // Used as a UI test for messages
    void testMessages();

    // TODO: There's something funky going on with the level shifter and
    // it interferes with the message receiving. Needs more debugging.
    void turnOnLevelShifter();
    void turnOffLevelShifter();

    // Used as a helper to add/delete chars from outgoingMessage
    void addCharToMessage(char key);

    // Returns the [shift] key + the given input key (e.g. t => T)
    // \param key: the key the user has pressed on the keyboard
    char getShiftedKey(char key);

    // Returns the [sym] key + the given input key (e.g. b => !)
    // \param key: the key the user has pressed on the keyboard
    char getSymKey(char key);

    // Stores the outgoing message when the user is typing in TEXT_HISTORY_STATE
    char outgoingMessage[250];

    // The following 2 stores the length of the message being typed by the user
    uint8_t outgoingMessageLen;
    uint8_t lastOutgoingMessageLen; // Used to check if we need to delete

    // Indicates whether screen needs update
    bool outgoingMessageUpdate;

    // The cursors on textMessageState
    uint8_t messageX;
    uint8_t messageY;
    uint8_t messageWidth; // TODO: Do I need this?

    // Used to store the incoming message from the radio module
    char incomingMessage[RH_RF95_MAX_MESSAGE_LEN];

    // The length of the incoming message
    uint8_t incomingMessageLen;

    // When messages get added to textHistory, we need to update the TEXT_HISTORY_STATE
    bool textHistoryNeedsUpdate;

    // Drawing Handlers
    void drawHeader(uint8_t state);
    void drawMiniLogo(uint8_t x, uint8_t y);
    void drawTextContainer(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color);
    void drawChatIcon(uint8_t x, uint8_t y);
    void drawBatteryIcon(uint8_t x, uint8_t y);
    void drawTextHistory();
    // This is part of the text bubble that "holds" a message
    void drawWispyTail(uint8_t x, uint8_t y, uint8_t side, uint16_t color);
    void drawBubble(Bubble bubble);
    void formatTime(char* buff, uint8_t hour, uint8_t min, uint8_t sec);
    // Blinks the cusor during SPLASH_STATE
    void blinkStartCursor(uint8_t x, uint8_t y);
    // Blinks the cusor during TEXT_MESSAGE_STATE
    void blinkTextCursor(uint8_t x, uint8_t y);

    // Gets called when state doesn't exist or event is invalid.
    void panic();
};

#endif
