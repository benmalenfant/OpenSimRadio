#include <Arduino.h>

HardwareTimer digit_timer(TIM3);
HardwareTimer digit_timer2(TIM2);

#define pin PA6
#define pin2 PA7_ALT1

void callback_test(void){
  digitalToggle(PC13);
}

void setup() {
  Serial.begin();
  delay(1000);
  Serial.println("Begin");

  digit_timer.getHandle()->Init.CounterMode = TIM_COUNTERMODE_CENTERALIGNED1;
  HAL_TIM_Base_Init(digit_timer.getHandle());

  digit_timer.setOverflow(300, MICROSEC_FORMAT);

  digit_timer.setMode(1,TIMER_OUTPUT_COMPARE_PWM2,pin);
  digit_timer.setCaptureCompare(1, 2, PERCENT_COMPARE_FORMAT); 

  digit_timer.setMode(2,TIMER_OUTPUT_COMPARE_PWM1,pin2);
  digit_timer.setCaptureCompare(2, 1, PERCENT_COMPARE_FORMAT);

  digit_timer.attachInterrupt(callback_test);

  digit_timer.resume();

  pinMode(PC13,OUTPUT);
  digitalWrite(PC13,0);
}

void loop() {
  delay(100);
}