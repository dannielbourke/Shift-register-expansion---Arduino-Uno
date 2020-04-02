// Led chaser program that uses shift register expansion
#include "Arduino.h"
#include "Expansion.h"

// pin 9 data out / pin 11 data in / pin 10 shift clock / pin 8 strobe

// All pins on a single port, don't mix ports with this class
Expansion myOutputs(9,11,10,8);

uint8_t chaser = B00000001;

void setup() {
  DDRB = DDRB | B00111111; // PortB 0 to 5 are outputs
}

void loop() {
  myOutputs.setOutOneVal(chaser);
  myOutputs.updateExpansion();
  if(chaser & B10000000) chaser = B00000001; // Reset
    else chaser = chaser << 1; // Shift to the left by 1 place
}
