#include <Arduino.h>

HardwareTimer digit_timer(TIM3);

TIM_HandleTypeDef* timer2;

#define pin PA6
#define pin2 PA7_ALT1

void callback_test(void){

}

void setup() {
  Serial.begin();
  delay(1000);
  Serial.println("Begin");

  digit_timer.getHandle()->Init.CounterMode = TIM_COUNTERMODE_CENTERALIGNED1;
  HAL_TIM_Base_Init(digit_timer.getHandle());
  LL_TIM_SetTriggerOutput(TIM3,TIM_TRGO_UPDATE);

  digit_timer.setOverflow(200000, MICROSEC_FORMAT);

  digit_timer.setMode(1,TIMER_OUTPUT_COMPARE_PWM2,pin);
  digit_timer.setCaptureCompare(1, 2, PERCENT_COMPARE_FORMAT); 
  digit_timer.setMode(2,TIMER_OUTPUT_COMPARE_PWM1,pin2);
  digit_timer.setCaptureCompare(2, 1, PERCENT_COMPARE_FORMAT);
  digit_timer.attachInterrupt(callback_test);


  timer2 = new TIM_HandleTypeDef();
  
  timer2->Instance = TIM2;
  timer2->Channel = HAL_TIM_ACTIVE_CHANNEL_CLEARED;
  timer2->hdma[0] = NULL;
  timer2->hdma[1] = NULL;
  timer2->hdma[2] = NULL;
  timer2->hdma[3] = NULL;
  timer2->hdma[4] = NULL;
  timer2->hdma[5] = NULL;
  timer2->hdma[6] = NULL;
  timer2->Lock = HAL_UNLOCKED;
  timer2->State = HAL_TIM_STATE_RESET;

 __HAL_TIM_SetCounter(timer2,14);
  timer2->Init.CounterMode = TIM_COUNTERMODE_DOWN;
  timer2->Init.Period = 14;
  timer2->Init.Prescaler = 1;
  HAL_TIM_Base_Init(timer2);

  LL_TIM_SetClockSource(TIM2,TIM_CLOCKSOURCE_ITR2);
  LL_TIM_SetSlaveMode(TIM2,TIM_SLAVEMODE_EXTERNAL1);
  HAL_TIM_Base_Start(timer2);


  digit_timer.resume();

  pinMode(PC13,OUTPUT);
}


void loop() {
  Serial.println(__HAL_TIM_GetCounter(timer2));
  delay(100);
}