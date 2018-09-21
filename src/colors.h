/*
  bubble.cpp - Outset texting demo
  author: Tina Zhao
  github: @manacake
*/
#ifndef Colors_h
#define Colors_h

// Use 5:6:5 color depth (16 bit)
// light to dark (as number decreases)
// But Adafruit GFX lib uses BGR (reversed)!
#define BLUE_MODE     1
#define GREEN_MODE    2
#define PINK_MODE     3

#define BLUE_LIGHTEST 0xF7F4
#define BLUE_LIGHT    0xE68C
#define BLUE_MID      0xC488
#define BLUE_DARK     0x4143

#define GREEN_LIGHTEST 0xCF90
#define GREEN_LIGHT    0x9F40
#define GREEN_MID      0x5500
#define GREEN_DARK     0x3B40

#define PINK_LIGHTEST  0xFD9C
#define PINK_LIGHT     0xFB1E
#define PINK_MID       0xC21B
#define PINK_DARK      0x7913

#define GREY           0x4A69

#define TFT_BLACK ST7735_BLACK
#define TFT_WHITE ST7735_WHITE
#endif
