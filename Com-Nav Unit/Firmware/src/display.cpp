#include <Arduino.h>

HardwareTimer clock_timer(TIM3);
HardwareTimer active_digit_timer(TIM2);

#define PIN_OE PA6        // Timer3 chanel 1
#define PIN_SCLK PA7_ALT1 // Timer3 chanel 2
#define PIN_DATA PA5      // Timer2 chanel 1

#define N_DIGITS 15

void display_update(void)
{
    Serial.println(TIM2->CNT);
}

void display_init(void)
{
    // init the timer in center align mode
    clock_timer.getHandle()->Init.CounterMode = TIM_COUNTERMODE_CENTERALIGNED1;
    HAL_TIM_Base_Init(clock_timer.getHandle());

    // set the trgo output for the second timer
    LL_TIM_SetTriggerOutput(TIM3, TIM_TRGO_UPDATE);

    // Set timer period to 300us to get around 200hz refresh rate
    clock_timer.setOverflow(300, MICROSEC_FORMAT);

    // Set channel 1 (OE) to be off 2% of duty cycle
    // This will be used to dim the display, the greater the value, the dimmer
    clock_timer.setMode(1, TIMER_OUTPUT_COMPARE_PWM2, PIN_OE);
    clock_timer.setCaptureCompare(1, 1, PERCENT_COMPARE_FORMAT);

    // Set channel 2 (CLK) to be on 1% of duty cycle
    clock_timer.setMode(2, TIMER_OUTPUT_COMPARE_PWM1, PIN_SCLK);
    clock_timer.setCaptureCompare(2, 1, MICROSEC_COMPARE_FORMAT);

    // Attach the interrupt that will handle the digit update from array
    clock_timer.attachInterrupt(display_update);

    // active_digit_timer configuration

    // init the counter to it's preload value
    __HAL_TIM_SetCounter(active_digit_timer.getHandle(), N_DIGITS - 1);

    // Initial counter config, count down from 14 every timer3 cycle
    active_digit_timer.getHandle()->Init.CounterMode = TIM_COUNTERMODE_DOWN;
    active_digit_timer.getHandle()->Init.Period = N_DIGITS - 1;
    active_digit_timer.getHandle()->Init.Prescaler = 0;
    HAL_TIM_Base_Init(active_digit_timer.getHandle());

    // Set the timer2 to use ITR2(Timer3 output) as a source
    LL_TIM_SetClockSource(TIM2, TIM_CLOCKSOURCE_ITR2);
    LL_TIM_SetSlaveMode(TIM2, TIM_SLAVEMODE_EXTERNAL1);

    // Configure the Data pin (channel 1) to output once every 15 clock cycle to shift in a bit
    active_digit_timer.setMode(1, TIMER_OUTPUT_COMPARE_PWM1, PIN_DATA);
    active_digit_timer.setCaptureCompare(1, 0, TICK_COMPARE_FORMAT);

    // Start the timers and enable the pwm outputs
    clock_timer.resume();
    active_digit_timer.resume();
}