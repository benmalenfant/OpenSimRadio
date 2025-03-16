#include <Arduino.h>

HardwareTimer clock_timer(TIM3);
HardwareTimer active_digit_timer(TIM2);

#define PIN_OE PA6        // Timer3 chanel 1
#define PIN_CLK PA7_ALT1 // Timer3 chanel 2
#define PIN_DATA PA5      // Timer2 chanel 1

#define CHANNEL_OE 1
#define CHANNEL_CLK 2
#define CHANNEL_DATA 1

#define N_DIGITS 15

void display_update(void)
{
    //use TIM2->CNT for active digit
    GPIOB->ODR = ~GPIOB->ODR;
}

void display_init(void)
{
    // clock_timer configuration
    // init the timer in center align mode
    clock_timer.getHandle()->Init.CounterMode = TIM_COUNTERMODE_CENTERALIGNED1;
    HAL_TIM_Base_Init(clock_timer.getHandle());

    // set the trgo output for the second timer
    LL_TIM_SetTriggerOutput(TIM3, TIM_TRGO_UPDATE);

    // Set timer period to 60us to get around 1kHz refresh rate
    clock_timer.setOverflow(200, MICROSEC_FORMAT);

    // Set channel 1 (OE) to be off 2% of duty cycle
    // This will be used to dim the display, the greater the value, the dimmer
    clock_timer.setMode(CHANNEL_OE, TIMER_OUTPUT_COMPARE_PWM2, PIN_OE);
    clock_timer.setCaptureCompare(CHANNEL_OE, 1, PERCENT_COMPARE_FORMAT);

    // Set channel 2 (CLK) to be on 1% of duty cycle
    clock_timer.setMode(CHANNEL_CLK, TIMER_OUTPUT_COMPARE_PWM1, PIN_CLK);
    //Clock set to a short pulse, period around 600ns
    clock_timer.setCaptureCompare(CHANNEL_CLK, 60, TICK_COMPARE_FORMAT);

    // Attach the interrupt that will handle the digit update from array
    clock_timer.attachInterrupt(CHANNEL_CLK,display_update);


    // active_digit_timer configuration
    // init the counter to it's preload value
    __HAL_TIM_SetCounter(active_digit_timer.getHandle(), N_DIGITS - 1);

    // Initial counter config, count down from 14 every timer3 cycle
    active_digit_timer.getHandle()->Init.CounterMode = TIM_COUNTERMODE_DOWN;
    active_digit_timer.getHandle()->Init.Period = N_DIGITS - 1;
    //Prescaler of 1 since center aligned timers generate 2 event per period
    active_digit_timer.getHandle()->Init.Prescaler = 1;
    HAL_TIM_Base_Init(active_digit_timer.getHandle());

    // Set the timer2 to use ITR2(Timer3 output) as a source
    LL_TIM_SetClockSource(TIM2, TIM_CLOCKSOURCE_ITR2);
    LL_TIM_SetSlaveMode(TIM2, TIM_SLAVEMODE_EXTERNAL1);

    // Configure the Data pin (channel 1) to output once every 15 clock cycle to shift in a bit
    active_digit_timer.setMode(CHANNEL_DATA, TIMER_OUTPUT_COMPARE_PWM2, PIN_DATA);
    //set the output to on if the counter is equal or smaller to 0
    active_digit_timer.setCaptureCompare(CHANNEL_DATA, 0, TICK_COMPARE_FORMAT);

    LL_TIM_SetUpdateSource(TIM3,LL_TIM_UPDATESOURCE_COUNTER);

    // Start the timers and enable the pwm outputs
    clock_timer.resume();
    active_digit_timer.resume();

    delay(1);//let time for all bits to shift

    //Get arduino to init GPIOB
    pinMode(PB0,OUTPUT);

    //Set all digits pins to off (3.3V)
    GPIOB->ODR = 0xffffFFFF;
    //Set port B as output
    GPIOB->MODER = 0x55555555;

    // Attach the interrupt that will handle the digit update from array
    clock_timer.attachInterrupt(CHANNEL_CLK,display_update);
}

void display_set_brightness(uint8_t brightness){
    //A compare value of 1 = full brightness, compare value of 100 will shut down the display
    clock_timer.setCaptureCompare(CHANNEL_OE, constrain(100-brightness,1,100), PERCENT_COMPARE_FORMAT);
}