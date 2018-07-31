// Outset Demo for client side

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
// #include <RHReliableDatagram.h>
// #include <RH_RF95.h>
#include <SPI.h>

// for feather32u4
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7
#define TFT_CS 12
#define TFT_DC 10
#define TFT_RST 11
#define TFT_LITE 13

#define CLIENT_ADDRESS 1
#define SERVER_ADDRESS 2

#define LEFT_BUBBLE_X 7
#define LEFT_BUBBLE_Y 2
#define RIGHT_BUBBLE_X 58
#define RIGHT_BUBBLE_Y 2
#define LEFT_TEXT_X 10
#define RIGHT_TEXT_X 61
#define TEXT_Y 4
#define BUBBLE_H 19
#define LEFT_BUBBLE_W 130
#define RIGHT_BUBBLE_W 95

// Singleton instance of the radio driver
//RH_RF95 driver(RFM95_CS, RFM95_INT);
//RHReliableDatagram manager(driver, CLIENT_ADDRESS);
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
bool leftBubble = true;

void setup() 
{
  Serial.begin(9600);

  pinMode(TFT_LITE, OUTPUT);
  digitalWrite(TFT_LITE, HIGH);
  
//  if (!manager.init()) {
//    Serial.println(F("init failed"));
//    while(1);
//  }
//  else if (!driver.setFrequency(915.0)) {
//    Serial.println(F("set freq failed"));
//    while(1);
//  }
//  else {
//    driver.setTxPower(20, false);
//    manager.setTimeout(500);
//    Serial.println(F("radio init complete"));
//  }

  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(ST7735_BLACK);
  tft.setTextSize(1);
  tft.setRotation(3);
}

uint8_t data[] = "Hello World!";
// Dont put this on the stack:
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

void loop() {
  // Reset bubble to act on. This is the max amount of bubbles on the screen
  if (bubbleIndex >= 6) {
    bubbleIndex = 0;
    tft.fillScreen(ST7735_BLACK);
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
      leftBubbleY += 21;
    } else {
      leftBubbleY += 42;
    }
    tft.fillRect(LEFT_BUBBLE_X, leftBubbleY, LEFT_BUBBLE_W, BUBBLE_H, ST7735_BLUE);
    tft.fillTriangle(LEFT_BUBBLE_Y, BUBBLE_H + leftBubbleY -1, 
      LEFT_BUBBLE_Y + 5, (BUBBLE_H + leftBubbleY) - 5 -1, 
      LEFT_BUBBLE_Y + 10, BUBBLE_H + leftBubbleY -1, 
      ST7735_BLUE);
    tft.setTextColor(ST7735_WHITE);
    textX = LEFT_TEXT_X;
    tft.setCursor(textX, textY);
    tft.println("5:00:02");
    textY += 8;
    tft.setCursor(textX, textY);
    tft.println("MSG FROM BASE: HELLO!");
    leftBubble = false;
    delay(500);
  }
  else { // Draw right side bubble
    // Set Y cursor for bubbles
    if (bubbleIndex == 1) {
      rightBubbleY = RIGHT_BUBBLE_Y;
    } else if (bubbleIndex == 2) {
      rightBubbleY = RIGHT_BUBBLE_Y; // TODO: this needs a reset since we never reach index 1
      rightBubbleY += 21;
    } else {
      rightBubbleY += 42;
    }
    tft.fillRect(RIGHT_BUBBLE_X, rightBubbleY, RIGHT_BUBBLE_W, BUBBLE_H, ST7735_CYAN);
    tft.fillTriangle(149, rightBubbleY,
      153, rightBubbleY + 4,
      157, rightBubbleY,
      ST7735_CYAN);
    tft.setTextColor(ST7735_BLACK);
    textX = RIGHT_TEXT_X;
    tft.setCursor(textX, textY);
    tft.println("5:00:00");
    textY += 8;
    tft.setCursor(textX, textY);
    tft.println("SENT REPLY...OK");
    leftBubble = true;
    delay(3000);
  }
  
//  Serial.println(F("Sending to server"));
//    
//  // Send a message to manager_server
//  if (manager.sendtoWait(data, sizeof(data), SERVER_ADDRESS))
//  {
//    // Now wait for a reply from the server
//    uint8_t len = sizeof(buf);
//    uint8_t from;   
//    if (manager.recvfromAckTimeout(buf, &len, 2000, &from))
//    {
//      Serial.print(F("got reply from: "));
//      Serial.print(from, DEC);
//      Serial.print(F(": "));
//      Serial.println((char*)buf);
//    }
//    else
//    {
//      Serial.println(F("No reply, is server running?"));
//    }
//  }
//  else
//    Serial.println(F("sendtoWait failed"));
//  delay(500);
}

