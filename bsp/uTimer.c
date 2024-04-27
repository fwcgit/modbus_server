#include "uTimer.h"
#include <gd32f10x.h>
#include "Log.h"
#include "systick.h"
/**
sys_mhz 主频
定时器5 产生1us计数一次做延时
定时器6 产生1ms计数一次做延时
**/
void utimer_init(unsigned char sys_mhz)
{
	rcu_periph_clock_enable(RCU_TIMER5);
	rcu_periph_clock_enable(RCU_TIMER6);
	
	timer_parameter_struct t_parameter_base;
	t_parameter_base.alignedmode 		= TIMER_COUNTER_EDGE;
	t_parameter_base.clockdivision 		= TIMER_CKDIV_DIV1;
	t_parameter_base.counterdirection 	= TIMER_COUNTER_CENTER_UP;
	t_parameter_base.period 			= 65535;
	t_parameter_base.prescaler 			= sys_mhz -1;
	
	timer_init(TIMER5,&t_parameter_base);
	timer_interrupt_disable(TIMER5,TIMER_INT_UP);
	timer_interrupt_flag_clear(TIMER5, TIMER_INT_FLAG_UP);
	
	t_parameter_base.prescaler = sys_mhz * 1000 -1;
	timer_init(TIMER6,&t_parameter_base);
	timer_interrupt_flag_clear(TIMER6, TIMER_INT_FLAG_UP);
	timer_interrupt_disable(TIMER6,TIMER_INT_UP);
	
}

/***
ms 0~65535 
**/
void udelayMs(unsigned short ms)
{
	if(timer_counter_read(TIMER6)!= 0)
		return;
	timer_interrupt_flag_clear(TIMER6, TIMER_INT_FLAG_UP);
	timer_counter_value_config(TIMER6,0);
	timer_enable(TIMER6);
	while(timer_counter_read(TIMER6) < ms);
	timer_counter_value_config(TIMER6,0);
	timer_disable(TIMER6);
	timer_interrupt_flag_clear(TIMER6, TIMER_INT_FLAG_UP);
}

/***
us 0~65535 
**/
void udelayUs(unsigned short us)
{
	if(timer_counter_read(TIMER5)!= 0)
		return;
	timer_interrupt_flag_clear(TIMER5, TIMER_INT_FLAG_UP);
	timer_counter_value_config(TIMER5,0);
	timer_enable(TIMER5);
	while(timer_counter_read(TIMER5) < us);
	timer_counter_value_config(TIMER5,0);
	timer_disable(TIMER5);
	timer_interrupt_flag_clear(TIMER5, TIMER_INT_FLAG_UP);
}