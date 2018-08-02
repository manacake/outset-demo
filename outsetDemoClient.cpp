// Outset Demo for client side
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <RH_RF95.h>
#include <SPI.h>
#include <Wire.h>
#include "RTClib.h"

// General
#define LVL_SHIFT_EN 2
// Radio
#define RFM_CS     10
#define RFM_INT    3
#define RFM_RESET  49
#define RFM_PWR_EN 5
// Screen
#define TFT_CS     27
#define TFT_RESET  29
#define TFT_DC     28
#define TFT_PWR_EN 4
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

Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_RESET);
RTC_DS1307 RTC;
RH_RF95 driver(RFM_CS, RFM_INT);
RH_RF95::ModemConfig modemConfig = {
  0x78, // Reg 0x1D: BW=125kHz, Coding=4/8, Header=explicit
  0xc4, // Reg 0x1E: Spread=4096chips/symbol, CRC=enable
  0x0c  // Reg 0x26: LowDataRate=On, Agc=ON (Automatic gain control)
};
// Other option to try
// 0x08  // Reg 0x26: LowDataRate=On, Agc=OFF
// 0x0c  // Reg 0x26: LowDataRate=On, Agc=ON (Automatic gain control)

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
bool radioInit = false;
bool freqSet = false;

uint8_t calendarIconX = 94;
uint8_t calendarIconY = 2;
uint8_t batteryIconX = 71;
uint8_t batteryIconY = 2;
uint8_t userIconX = 3;
uint8_t userIconY = 2;

uint8_t data[] = "YO";
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

void formatTime(char* buff, uint8_t hour, uint8_t min, uint8_t sec) {
  char temp[3];

  buff[0] = '\0';
  itoa(hour, temp, 10);
  if (hour < 10) {
    strcpy(buff, "0");
  }
  strcat(buff, temp);
  strcat(buff, ":");

  if (min < 10) {
    strcat(buff, "0");
  }
  itoa(min, temp, 10);
  strcat(buff, temp);
  strcat(buff, ":");

  if (sec < 10) {
    strcat(buff, "0");
  }
  itoa(sec, temp, 10);
  strcat(buff, temp);
}

void formatDate(char* buff, uint8_t month, uint8_t day, uint8_t year) {
  char temp[3];

  buff[0] = '\0';
  itoa(month, temp, 10);
  if (month < 10) {
    strcpy(buff, "0");
  }
  strcat(buff, temp);
  strcat(buff, "-");

  if (day < 10) {
    strcat(buff, "0");
  }
  itoa(day, temp, 10);
  strcat(buff, temp);
  strcat(buff, "-");

  if (year < 10) {
    strcat(buff, "0");
  }
  itoa(year, temp, 10);
  strcat(buff, temp);
}

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
  tft.setCursor(111, 5);
  tft.print(date);
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

  tft.setCursor(20, 5);
  tft.print("MITCH");
}

void drawHeader() {
  DateTime now = RTC.now();
  char date[8];
  formatDate(date, now.month(), now.day(), 18);

  tft.fillRect(0, 0, 160, 17, ST7735_YELLOW);
  drawCalendarIcon(date);
  drawBatteryIcon();
  drawUserIcon();
}

void setup() {
  Serial.begin(9600);

  // pinMode(RFM_RESET, OUTPUT);
  pinMode(RFM_PWR_EN, OUTPUT);
  delay(250);
  digitalWrite(RFM_PWR_EN, LOW);
  // Init radio
  if (!driver.init()) {
    Serial.println(F("init failed"));
  }
  else if (!driver.setFrequency(915.0)) {
    Serial.println(F("set freq failed"));
  }
  else {
    driver.setTxPower(20, false);
    driver.setModemRegisters(&modemConfig);
    // driver.setTimeout(500); // only available in reliable datagram
    radioInit = true;
    freqSet = true;
    Serial.println(F("radio init complete"));
  }

  pinMode(LVL_SHIFT_EN, OUTPUT);
  digitalWrite(LVL_SHIFT_EN, LOW);
  pinMode(TFT_PWR_EN, OUTPUT);
  digitalWrite(TFT_PWR_EN, HIGH);
  delay(250);
  digitalWrite(LVL_SHIFT_EN, HIGH);
  digitalWrite(TFT_PWR_EN, LOW);

  tft.initR(INITR_GREENTAB);
  tft.fillScreen(ST7735_BLACK);
  tft.setTextSize(1);
  tft.setRotation(3);
  tft.setCursor(0,0);

  // Init clock
  RTC.begin();
  // RTC not running!
  if (!RTC.isrunning()) {
    // Sets the RTC to the date & time this file was compiled
    RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  if (!radioInit) {
    tft.println("RADIO INIT FAILED!");
    while (1);
  } else if (!freqSet) {
    tft.println("SET 915.0 FREQ FAILED");
    while(1);
  } else {
    tft.println("RADIO SETUP COMPLETE!");
  }
  delay(2000);
  tft.fillScreen(ST7735_BLACK);
  drawHeader();
}

void drawNextBubble(char* timestamp, bool reply = false) {
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
    if (reply) {
      tft.println("REPLY FROM MITCH: HI!");
    } else {
      tft.println("NO REPLY FROM MITCH..");
    }
    leftBubble = false;
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
    tft.println("SENT MSG: HELLO");
    leftBubble = true;
  }
}

// Some oddness with the server only being able to receive every other message
void loop() {
  Serial.println(F("Sending to server"));
  driver.send(data, sizeof(data));
  driver.waitPacketSent();

  uint8_t len = sizeof(buf);
  DateTime now = RTC.now();
  char timestamp[8];
  formatTime(timestamp, now.hour(), now.minute(), now.second());
  drawNextBubble(timestamp);

  if (driver.waitAvailableTimeout(3000)) {
    // Should be a reply message for us now
    if (driver.recv(buf, &len)) {
      Serial.print(F("got reply: "));
      Serial.println((char*)buf);
      drawNextBubble(timestamp, true);
    } else {
      Serial.println(F("recv failed"));
      drawNextBubble(timestamp, false);
    }
  } else {
    Serial.println(F("No reply, is server running?"));
    drawNextBubble(timestamp, false);
  }
  delay(1000);
}
