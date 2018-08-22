#include "outset.h"

Outset outset;

void setup() {
  Serial.begin(9600);
  outset.init();
}

void loop() {
  outset.runFSM();
}
