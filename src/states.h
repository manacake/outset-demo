/*
  states.h - Outset texting demo
  author: @manacake
*/
#ifndef States_h
#define States_h

// State machine states
#define INVALID_STATE           0
#define SPLASH_STATE            1
#define TEXT_HISTORY_STATE      2
#define TEXT_MESSAGE_STATE      3
#define TEXT_SEND_STATE         4
#define NUM_STATES              5

// State machine events
// 0 can also be passed as an event...
// That just means there's no event
#define GO_BACK              1
#define CANCEL               2
#define CONFIRM              3
#define TIMEOUT              4
#define INIT_BOOT            5

// Other misc dev defines
// Note: This can be set in platformio.ini as a build flag if you use Platformio
// #define DEVICE_ID ?
#define RF95_FREQ 915.0

#endif
