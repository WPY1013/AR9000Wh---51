#include <STC8H.H>

/* Change these macros to match the PCB. */
#define LED_PORT_MODE0 P3M0
#define LED_PORT_MODE1 P3M1
#define LED_PIN        P32
#define LED_ACTIVE_LOW 1

void MX_GPIO_Init(void)
{
    /* Push-pull output. */
    LED_PORT_MODE0 |= 0x04;
    LED_PORT_MODE1 &= (unsigned char)~0x04;
    LED_PIN = LED_ACTIVE_LOW ? 1 : 0;
}


