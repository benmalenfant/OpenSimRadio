#include <Arduino.h>
#include "display.h"

void setup() {
  Serial.begin();
  delay(1000);
  Serial.println("Begin");
  display_init();
  pinMode(PC13,OUTPUT);
}


void loop() {

}

