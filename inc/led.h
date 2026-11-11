#ifndef _LED_H
#define _LED_H


// LED_x_ACTIVE is the pin level while the LED signals activity
#ifdef MKS_PRO

// The MKS CANable Pro V1.0 has its RX LED on PA1 and its TX LED on PA0. Both light up on a low pin.
#define LED_BLUE_Pin GPIO_PIN_1
#define LED_BLUE_Port GPIOA
#define LED_BLUE_ACTIVE 0

#define LED_GREEN_Pin GPIO_PIN_0
#define LED_GREEN_Port GPIOA
#define LED_GREEN_ACTIVE 0

#else

#define LED_BLUE_Pin GPIO_PIN_1
#define LED_BLUE_Port GPIOB
#define LED_BLUE_ACTIVE 1

#define LED_GREEN_Pin GPIO_PIN_0
#define LED_GREEN_Port GPIOB
#define LED_GREEN_ACTIVE 0

#endif

#define LED_BLUE LED_BLUE_Port , LED_BLUE_Pin
#define LED_GREEN LED_GREEN_Port , LED_GREEN_Pin



#define LED_DURATION 25 

void led_init();
void led_blue_blink(uint8_t numblinks);
void led_green_on(void);
void led_green_off(void);
void led_blue_on(void);
void led_process(void);

#endif
