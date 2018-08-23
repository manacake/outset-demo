// outset.h
#ifndef Outset_h
#define Outset_h

#include <TFT_ST7735.h>
#include <SPI.h>
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
    TFT_ST7735 tft;

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
    // Unique board ID
    uint8_t deviceID;

    // Watch the clock
    unsigned long currentMillis;
    // Keep state of blinking cursor on splash state
    unsigned long startCursorLastDrawn; // in ms
    bool startCursorVisible;
    bool startCursorEnabled;

    uint8_t trackpadState;
    uint8_t lastTrackpadState;
    unsigned long lastTrackpadDebounce;
    unsigned long debounceDelay;

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

    // Drawing Handlers
    void drawMiniLogo(uint8_t x, uint8_t y);
    void drawTextContainer(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color);
    void blinkStartCursor(uint8_t x, uint8_t y);
    void drawChatIcon(uint8_t x, uint8_t y);
    void drawBatteryIcon(uint8_t x, uint8_t y);

    // Gets called when state doesn't exist or event is invalid.
    void panic();
};

#endif
