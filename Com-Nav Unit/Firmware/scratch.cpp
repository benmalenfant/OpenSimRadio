#include <Arduino.h>
//
// Slow and precise BLDC motor driver using SPWM and SVPWM modulation
// Part of code used from http://elabz.com/
// (c) 2015 Ignas Gramba www.berryjam.eu
//
 
#define TX PB3
#define RX PB4
HardwareSerial Serial1(RX, TX);


const int EN1 = PC6;

const int IN1_H = PHASE_UH;
const int IN1_L = PHASE_UL;
const int IN2_H = PHASE_VH;
const int IN2_L = PHASE_VL;
const int IN3_H = PHASE_WH;
const int IN3_L = PHASE_WL;


// SPWM (Sine Wave)
const int pwmSin[] = {127, 138, 149, 160, 170, 181, 191, 200, 209, 217, 224, 231, 237, 242, 246, 250, 252, 254, 254, 254, 252, 250, 246, 242, 237, 231, 224, 217, 209, 200, 191, 181, 170, 160, 149, 138, 127, 116, 105, 94, 84, 73, 64, 54, 45, 37, 30, 23, 17, 12, 8, 4, 2, 0, 0, 0, 2, 4, 8, 12, 17, 23, 30, 37, 45, 54, 64, 73, 84, 94, 105, 116 };

int currentStepA;
int currentStepB;
int currentStepC;
int sineArraySize;
int increment = 0;
boolean direct = 1; // direction true=forward, false=backward

//////////////////////////////////////////////////////////////////////////////


HardwareTimer *MyTim1, *MyTim2, *MyTim3,*MyTim4;
uint32_t channel1,channel2,channel3; 

HardwareTimer* pwm_init_high_and_low(int uhPin, int ulPin, int vhPin, int vlPin, int whPin, int wlPin,uint32_t PWM_freq)
{
  PinName uhPinName = digitalPinToPinName(uhPin);
  PinName ulPinName = digitalPinToPinName(ulPin);
  PinName vhPinName = digitalPinToPinName(vhPin);
  PinName vlPinName = digitalPinToPinName(vlPin);
  PinName whPinName = digitalPinToPinName(whPin);
  PinName wlPinName = digitalPinToPinName(wlPin);

  TIM_TypeDef *Instance = (TIM_TypeDef *)pinmap_peripheral(uhPinName, PinMap_PWM);
 
  uint32_t index = get_timer_index(Instance);
  Serial1.println("timer: "); Serial1.println(index);

  if (HardwareTimer_Handle[index] == NULL) {
    HardwareTimer_Handle[index]->__this = new HardwareTimer((TIM_TypeDef *)pinmap_peripheral(uhPinName, PinMap_PWM));
    HardwareTimer_Handle[index]->handle.Init.CounterMode = TIM_COUNTERMODE_CENTERALIGNED3;
    HAL_TIM_Base_Init(&(HardwareTimer_Handle[index]->handle));
    ((HardwareTimer *)HardwareTimer_Handle[index]->__this)->setOverflow(PWM_freq, HERTZ_FORMAT);
  }
  HardwareTimer *HT = (HardwareTimer *)(HardwareTimer_Handle[index]->__this);
  uint32_t channelU = STM_PIN_CHANNEL(pinmap_function(uhPinName, PinMap_PWM));
  uint32_t channelV = STM_PIN_CHANNEL(pinmap_function(vhPinName, PinMap_PWM));
  uint32_t channelW = STM_PIN_CHANNEL(pinmap_function(whPinName, PinMap_PWM));

  
  HT->setMode(channelU, TIMER_OUTPUT_COMPARE_PWM1, uhPinName);
  HT->setMode(channelU, TIMER_OUTPUT_COMPARE_PWM1, ulPinName);
  HT->setMode(channelV, TIMER_OUTPUT_COMPARE_PWM1, vhPinName);
  HT->setMode(channelV, TIMER_OUTPUT_COMPARE_PWM1, vlPinName);
  HT->setMode(channelW, TIMER_OUTPUT_COMPARE_PWM1, whPinName);
  HT->setMode(channelW, TIMER_OUTPUT_COMPARE_PWM1, wlPinName);
  
  LL_TIM_OC_SetDeadTime(HT->getHandle()->Instance, 100); // deadtime is non linear!
  LL_TIM_CC_EnableChannel(HT->getHandle()->Instance, LL_TIM_CHANNEL_CH1 | LL_TIM_CHANNEL_CH1N | LL_TIM_CHANNEL_CH2 | LL_TIM_CHANNEL_CH2N | LL_TIM_CHANNEL_CH3 | LL_TIM_CHANNEL_CH3N);
  HT->refresh();
  return HT;
}

// setting pwm to hardware pin - instead analogWrite()
void pwm_set(int ulPin, uint32_t value, int resolution)
{
  PinName pin = digitalPinToPinName(ulPin);
  TIM_TypeDef *Instance = (TIM_TypeDef *)pinmap_peripheral(pin, PinMap_PWM);
  uint32_t index = get_timer_index(Instance);
  HardwareTimer *HT = (HardwareTimer *)(HardwareTimer_Handle[index]->__this);

  uint32_t channel = STM_PIN_CHANNEL(pinmap_function(pin, PinMap_PWM));
  Serial1.print(value); Serial1.print(" ");
  HT->setCaptureCompare(channel, value/6, (TimerCompareFormat_t)resolution);
}

#define constrain2(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(0):(amt)))

void setup() {
  Serial1.begin(115200);
        
  pinMode(EN1, OUTPUT); 
  digitalWrite(EN1, HIGH);

  MyTim1 = pwm_init_high_and_low(IN1_H, IN1_L, IN2_H, IN2_L, IN3_H, IN3_L, 50000);

  delay(1000);

  // sine table init
  sineArraySize = sizeof(pwmSin)/sizeof(int); // Find lookup table size
  int phaseShift = sineArraySize / 3;         // Find phase shift and initial A, B C phase values
  currentStepA = 0;
  currentStepB = currentStepA + phaseShift;
  currentStepC = currentStepB + phaseShift;

  Serial1.print(currentStepA); Serial1.print(" ");
  Serial1.print(currentStepB); Serial1.print(" ");
  Serial1.print(currentStepC); Serial1.print(" ");

  sineArraySize--; // Convert from array Size to last PWM array number
}

//////////////////////////////////////////////////////////////////////////////

void loop() {
    Serial1.print(currentStepA); Serial1.print(" ");
    Serial1.print(currentStepB); Serial1.print(" ");
    Serial1.print(currentStepC); Serial1.print(" ");

  pwm_set(IN1_H, pwmSin[currentStepA], 8);
  // pwm_set(IN1_L, constrain(pwmSin[currentStepA]+10,0,255), 8);
  pwm_set(IN2_H, pwmSin[currentStepB], 8);
  // pwm_set(IN2_L, constrain(pwmSin[currentStepB]+10,0,255), 8);
  pwm_set(IN3_H, pwmSin[currentStepC], 8);
  // pwm_set(IN3_L, constrain(pwmSin[currentStepC]+10,0,255), 8);
    Serial1.println();

  // sine table for next loop
  currentStepA = constrain2(currentStepA + 1,0,sineArraySize);
  currentStepB = constrain2(currentStepB + 1,0,sineArraySize);
  currentStepC = constrain2(currentStepC + 1,0,sineArraySize);
  
  /// Control speed by this delay
  delay(1);
}