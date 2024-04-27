#ifndef BITBANDGPIO_H
#define BITBANDGPIO_H
#include <gd32f10x.h>

#define BITBAND_ADDR(addr,n) (*((uint32_t volatile*)(0x42000000u + (((uint32_t)&(addr) - (uint32_t)0x40000000u)<<5) + (((uint32_t)(n))<<2))))

#define WORK_LED BITBAND_ADDR(GPIO_OCTL(GPIOC),13)

#define MCU_4G_STATUS BITBAND_ADDR(GPIO_ISTAT(GPIOA),0)

#define TEL_POWER_EN 		BITBAND_ADDR(GPIO_OCTL(GPIOB),9)
#define RGB_POWER_EN 		BITBAND_ADDR(GPIO_OCTL(GPIOB),0)
#define MCU_4G_RESET 		BITBAND_ADDR(GPIO_OCTL(GPIOA),1)
#define IO485_UART3 		BITBAND_ADDR(GPIO_OCTL(GPIOB),5)
#define IO485_UART2  		BITBAND_ADDR(GPIO_OCTL(GPIOA),6)

#endif
