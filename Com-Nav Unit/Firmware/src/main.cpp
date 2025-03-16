#include <Arduino.h>
#include "display.h"

void setup()
{
    Serial.begin();
    delay(1000);
    Serial.println("Begin");
    display_init();
}

void loop()
{
    display_set_brightness(100);
}
