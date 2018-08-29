// states.h
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

// Navigation links per state
// #define INVALID_CURSOR 255
// #define MAIN_NAV_ROUTE_CONTACTS 0
// #define MAIN_NAV_ROUTE_PAIR 1
// #define MAIN_NAV_ROUTE_SETTINGS 2

// Other misc dev defines
#define RF95_FREQ 915.0
#define DEFAULT_CLIENT_ADDRESS 7
#define DEFAULT_SERVER_ADDRESS 8

#endif
