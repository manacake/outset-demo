// Outset Demo for client side

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <SPI.h>

// for feather32u4
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7
#define TFT_CS 12
#define TFT_DC 10
#define TFT_RST 11
#define TFT_LITE 13

#define CLIENT_ADDRESS 16
#define SERVER_ADDRESS 17

// The initial start cursor position for each bubble
// (right underneath the header)
#define LEFT_BUBBLE_X 7
#define LEFT_BUBBLE_Y 21
#define RIGHT_BUBBLE_X 58
#define RIGHT_BUBBLE_Y 21
#define LEFT_TEXT_X 10
#define RIGHT_TEXT_X 61
#define TEXT_Y 23
#define BUBBLE_H 19
#define LEFT_BUBBLE_W 130
#define RIGHT_BUBBLE_W 95

// Singleton instance of the radio driver
RH_RF95 driver(RFM95_CS, RFM95_INT);
RHReliableDatagram manager(driver, CLIENT_ADDRESS);
Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_RST);

// Default cursor positions
// top right corner of the left text bubble
uint8_t leftBubbleY = LEFT_BUBBLE_Y;
uint8_t rightBubbleY = RIGHT_BUBBLE_Y;
uint8_t textX = LEFT_TEXT_X;
uint8_t textY = TEXT_Y;
// Bubbles visible on screen, starts at 1
// The bubble that is being acted on
uint8_t bubbleIndex = 0;
// Draw left side bubble ? else draw the right side bubble
bool leftBubble = false;

uint8_t calendarIconX = 94;
uint8_t calendarIconY = 2;
uint8_t batteryIconX = 71;
uint8_t batteryIconY = 2;
uint8_t userIconX = 3;
uint8_t userIconY = 2;

void drawCalendarIcon(char* date) {
  // Outline
  tft.drawRect(calendarIconX +4, calendarIconY, 2, 2, ST7735_BLACK);
  tft.drawRect(calendarIconX +9, calendarIconY, 2, 2, ST7735_BLACK);
  tft.drawRect(calendarIconX, calendarIconY +2, 15, 12, ST7735_BLACK);
  tft.drawLine(calendarIconX +2, calendarIconY +5, calendarIconX +12, calendarIconY +5, ST7735_BLACK);
  tft.drawLine(calendarIconX +1, calendarIconY +3, calendarIconX +1, calendarIconY +12, ST7735_BLACK);
  tft.drawLine(calendarIconX +13, calendarIconY +3, calendarIconX +13, calendarIconY +12, ST7735_BLACK);
  tft.drawPixel(calendarIconX +2, calendarIconY +3, ST7735_BLACK);
  tft.drawPixel(calendarIconX +12, calendarIconY +4, ST7735_BLACK);
  // Calendar blocks
  tft.drawRect(calendarIconX +3, calendarIconY +7, 2, 2, ST7735_BLACK);
  tft.drawRect(calendarIconX +3, calendarIconY +10, 2, 2, ST7735_BLACK);
  tft.drawRect(calendarIconX +6, calendarIconY +7, 3, 2, ST7735_BLACK);
  tft.drawRect(calendarIconX +6, calendarIconY +10, 3, 2, ST7735_BLACK);
  tft.drawRect(calendarIconX +10, calendarIconY +7, 2, 2, ST7735_BLACK);
  tft.drawRect(calendarIconX +10, calendarIconY +10, 2, 2, ST7735_BLACK);
  // Date
  tft.setTextColor(ST7735_BLACK);
  // tft.setCursor(111, 1);
  // tft.print("05:10:36");
  tft.setCursor(111, 8);
  tft.print("07-31-18");
}

void drawBatteryIcon() {
  // Left half
  tft.fillRect(batteryIconX +1, batteryIconY +2, 4, 2, ST7735_BLACK);
  tft.fillRect(batteryIconX, batteryIconY +3, 2, 8, ST7735_BLACK);
  tft.fillRect(batteryIconX +1, batteryIconY +10, 7, 2, ST7735_BLACK);
  // Lightning bolt
  tft.fillRect(batteryIconX +7, batteryIconY, 5, 4, ST7735_BLACK);
  tft.fillRect(batteryIconX +6, batteryIconY +4, 5, 4, ST7735_BLACK);
  tft.fillRect(batteryIconX +11, batteryIconY +5, 3, 3, ST7735_BLACK);
  tft.drawLine(batteryIconX +14, batteryIconY +5, batteryIconX +14, batteryIconY +6, ST7735_BLACK);
  tft.fillRect(batteryIconX +10, batteryIconY +8, 3, 2, ST7735_BLACK);
  tft.fillRect(batteryIconX +10, batteryIconY +10, 2, 2, ST7735_BLACK);
  tft.fillRect(batteryIconX +9, batteryIconY +11, 2, 3, ST7735_BLACK);
  tft.drawPixel(batteryIconX +9, batteryIconY +14, ST7735_BLACK);
  // Right half
  tft.fillRect(batteryIconX +13, batteryIconY +2, 6, 2, ST7735_BLACK);
  tft.fillRect(batteryIconX +18, batteryIconY +3, 2, 3, ST7735_BLACK);
  tft.fillRect(batteryIconX +19, batteryIconY +4, 2, 6, ST7735_BLACK);
  tft.fillRect(batteryIconX +18, batteryIconY +8, 2, 4, ST7735_BLACK);
  tft.fillRect(batteryIconX +14, batteryIconY +10, 6, 2, ST7735_BLACK);
  tft.drawPixel(batteryIconX +13, batteryIconY +11, ST7735_BLACK);
}

void drawUserIcon() {
  tft.fillRect(userIconX, userIconY, 2, 4, ST7735_BLUE);
  tft.fillRect(userIconX +12, userIconY, 2, 4, ST7735_BLUE);
  tft.drawLine(userIconX +2, userIconY +1, userIconX +2, userIconY +3, ST7735_BLUE);
  tft.drawLine(userIconX +3, userIconY +2, userIconX +3, userIconY +3, ST7735_BLUE);
  tft.drawLine(userIconX +4, userIconY +3, userIconX +9, userIconY +3, ST7735_BLUE);
  tft.drawLine(userIconX +10, userIconY +2, userIconX +10, userIconY +3, ST7735_BLUE);
  tft.drawLine(userIconX +11, userIconY +1, userIconX +11, userIconY +3, ST7735_BLUE);

  tft.fillRect(userIconX +1, userIconY +4, 12, 6, ST7735_BLUE);
  tft.drawLine(userIconX +2, userIconY +10, userIconX +11, userIconY +10, ST7735_BLUE);
  tft.drawLine(userIconX +3, userIconY +11, userIconX +10, userIconY +11, ST7735_BLUE);
  tft.drawLine(userIconX +4, userIconY +12, userIconX +9, userIconY +12, ST7735_BLUE);

  tft.drawLine(userIconX +3, userIconY +4, userIconX +4, userIconY +4, ST7735_CYAN);
  tft.drawLine(userIconX +9, userIconY +4, userIconX +10, userIconY +4, ST7735_CYAN);
  tft.fillRect(userIconX +3, userIconY +6, 2, 2, ST7735_CYAN); // eye
  tft.fillRect(userIconX +9, userIconY +6, 2, 2, ST7735_CYAN); // eye
  tft.fillRect(userIconX +6, userIconY +8, 2, 2, ST7735_CYAN); // nose
  tft.drawPixel(userIconX +5, userIconY +10, ST7735_CYAN);
  tft.drawPixel(userIconX +8, userIconY +10, ST7735_CYAN);
}

void drawHeader() {
  tft.fillRect(0, 0, 160, 17, ST7735_YELLOW);
  drawCalendarIcon("07-31-18");
  drawBatteryIcon();
  drawUserIcon();
}

void setup() {
  Serial.begin(9600);

  pinMode(TFT_LITE, OUTPUT);
  digitalWrite(TFT_LITE, HIGH);
  
  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(ST7735_BLACK);
  tft.setTextSize(1);
  tft.setRotation(3);
  tft.setCursor(0,0);
  
  if (!manager.init()) {
    Serial.println(F("init failed"));
    tft.println("Radio init failed");
    while(1);
  }
  else if (!driver.setFrequency(915.0)) {
    Serial.println(F("set freq failed"));
    tft.println("Set frequency failed");
    while(1);
  }
  else {
    driver.setTxPower(20, false);
    manager.setTimeout(500);
    Serial.println(F("radio init complete"));
    tft.println("Radio init complete");
  }

  tft.fillScreen(ST7735_BLACK);
  drawHeader();
}

uint8_t data[] = "Hello World!";
// Dont put this on the stack:
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

void drawNextBubble(char* timestamp, bool sleep = false) {
  // Reset bubble to act on. This is the max amount of bubbles on the screen
  if (bubbleIndex >= 5) {
    bubbleIndex = 0;
    tft.fillRect(0, 17, 160, 111, ST7735_BLACK);
  }
  bubbleIndex ++;
  // Set Y cursor for text print
  if (bubbleIndex > 1) {
    textY += (7 + 6);
  } else { // First bubble
    textY = TEXT_Y;
  }
  
  if (leftBubble) { // Draw left side bubble
    // Set Y cursor for bubbles
    if (bubbleIndex == 1) {
      leftBubbleY = LEFT_BUBBLE_Y;
    } else if (bubbleIndex == 2) {
      leftBubbleY = LEFT_BUBBLE_Y;
      leftBubbleY += 21;
    } else {
      leftBubbleY += 42;
    }
    tft.fillRoundRect(LEFT_BUBBLE_X, leftBubbleY, LEFT_BUBBLE_W, BUBBLE_H, 4, ST7735_BLUE);
    tft.fillTriangle(LEFT_BUBBLE_X -5, BUBBLE_H + leftBubbleY -1, 
      LEFT_BUBBLE_X, (BUBBLE_H + leftBubbleY) -5 -1, 
      LEFT_BUBBLE_X +5, BUBBLE_H + leftBubbleY -1, 
      ST7735_BLUE);
    tft.setTextColor(ST7735_WHITE);
    textX = LEFT_TEXT_X;
    tft.setCursor(textX, textY);
    tft.println(timestamp);
    textY += 8;
    tft.setCursor(textX, textY);
    tft.println("RPLY FROM OUT: HELLO!");
    leftBubble = false;
    if (sleep) delay(500);
  }
  else { // Draw right side bubble
    // Set Y cursor for bubbles
    if (bubbleIndex == 1) {
      rightBubbleY = RIGHT_BUBBLE_Y;
    } else if (bubbleIndex == 2) {
      rightBubbleY = RIGHT_BUBBLE_Y;
      rightBubbleY += 21;
    } else {
      rightBubbleY += 42;
    }
    tft.fillRoundRect(RIGHT_BUBBLE_X, rightBubbleY, RIGHT_BUBBLE_W, BUBBLE_H, 4, ST7735_CYAN);
    tft.fillTriangle(149, rightBubbleY,
      153, rightBubbleY + 4,
      157, rightBubbleY,
      ST7735_CYAN);
    tft.setTextColor(ST7735_BLACK);
    textX = RIGHT_TEXT_X;
    tft.setCursor(textX, textY);
    tft.println(timestamp);
    textY += 8;
    tft.setCursor(textX, textY);
    tft.println("SENT MSG.....OK");
    leftBubble = true;
    if (sleep) delay(500);
  }
}

void loop() {
  Serial.println(F("Sending to server"));
   
  // Send a message to manager_server
  if (manager.sendtoWait(data, sizeof(data), SERVER_ADDRESS)) {
    drawNextBubble("5:10:36", true);
    drawNextBubble("5:10:37", true);
    
    // Now wait for a reply from the server
    uint8_t len = sizeof(buf);
    uint8_t from;   
    if (manager.recvfromAckTimeout(buf, &len, 2000, &from)) {
      Serial.print(F("got reply from: "));
      Serial.print(from, DEC);
      Serial.print(F(": "));
      Serial.println((char*)buf);
    } else {
      Serial.println(F("No reply, is server running?"));
    }
  } else {
    Serial.println(F("sendtoWait failed"));
  }
  delay(3000);
}

