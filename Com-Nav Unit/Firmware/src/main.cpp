#include <Arduino.h>
#include "display.h"

void setup()
{
    Serial.begin();
    delay(1000);
    Serial.println("Begin");
    display_init();
    display_set_brightness(100);
}


void loop()
{
    delay(100);
}
