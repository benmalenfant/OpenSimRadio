#include <HardwareTimer.h>

extern HardwareTimer clock_timer;
extern HardwareTimer active_digit_timer;

void display_init(void);

void display_set_brightness(uint8_t brightness);