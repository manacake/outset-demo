// outset.h
#ifndef Outset_h
#define Outset_h

#include <RH_RF95.h>
#include <TFT_ST7735.h>
#include <SPI.h>
#include <Keypad.h>
#include "bubble.h"
#include "states.h"
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
    TFT_ST7735 tft;
    Keypad keypad;

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
    // Comes from build flags
    uint8_t deviceID;

    // Watch the clock
    unsigned long currentMillis;
    // Keep state of blinking cursor on splash state
    unsigned long startCursorLastDrawn; // in ms
    bool startCursorVisible;

    const char keypadKeys[KEYPAD_ROWS][KEYPAD_COLS];
    const uint8_t keypadRowPins[KEYPAD_ROWS];
    const uint8_t keypadColPins[KEYPAD_COLS];
    bool shiftPressed;
    bool symPressed;

    uint8_t trackpadState;
    uint8_t lastTrackpadState;
    unsigned long lastTrackpadDebounce;
    unsigned long debounceDelay;

    // TEXT_HISTORY_STATE helper values
    // Draws text bubbles underneath the header until there's too many bubbles
    // on screen
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
    void splashState(uint8_t event = 0);
    void textHistoryState(uint8_t event = 0);
    void textMessageState(uint8_t event = 0);
    void textSendState(uint8_t event = 0);
    void clearBody();

    void keypadEvent();
    void addCharToMessage(char key);
    char getShiftedKey(char key);
    char getSymKey(char key);
    char outgoingMessage[250];
    uint8_t outgoingMessageLen;
    uint8_t lastOutgoingMessageLen;
    bool outgoingMessageUpdate; // Indicates whether screen needs update
    uint8_t messageX; // The cursor on textMessageState
    uint8_t messageY;
    uint8_t messageWidth;
    char incomingMessage[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t incomingMessageLen;
    bool textHistoryNeedsUpdate;

    // Drawing Handlers
    void drawHeader(uint8_t state);
    void drawMiniLogo(uint8_t x, uint8_t y);
    void drawTextContainer(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color);
    void blinkStartCursor(uint8_t x, uint8_t y);
    void drawChatIcon(uint8_t x, uint8_t y);
    void drawBatteryIcon(uint8_t x, uint8_t y);
    void drawTextHistory();
    void drawWispyTail(uint8_t x, uint8_t y, uint8_t side, uint16_t color);
    void drawBubble(Bubble bubble);

    // Message Handlers
    void pushMessage(const char* message, const char* timestamp, uint8_t createdBy);
    void listenForIncomingMessages();
    void testMessages();

    // Keyboard event handler
    void keypadEvent(KeypadEvent key);
    void blinkTextCursor(uint8_t x, uint8_t y);

    // Gets called when state doesn't exist or event is invalid.
    void panic();
};

#endif
